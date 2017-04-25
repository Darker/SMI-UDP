#include "FileProtocolSocket.h"
#include "DataPacket.h"
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include "Checksum.h"
#include "PacketGuard.h"
#include <functional>
#include "crc32.h"
FileProtocolSocket::FileProtocolSocket(QUdpSocket* parent, ClientID target) :
    QObject(parent)
  , socket(parent)
  , target(target)
  , handlesAllDatagrams(false)
  , currentFile(nullptr)
  , writeFile(nullptr)
  , currentByte(0)
  , sendFailures(0)
  , CRCFailures(0)
  , pingIsPending(false)
  , pingSum(0)
  , maxPacketConfirmLatency(1)
{
    QObject::connect(this, &FileProtocolSocket::fileChunkReady, this, &FileProtocolSocket::sendNextFileChunk, Qt::QueuedConnection);
    QObject::connect(this, &FileProtocolSocket::receiptsConfirmed, this, &FileProtocolSocket::notifyReceipt, Qt::QueuedConnection);
    QObject::connect(&receivedUnconfirmedTimeout, &QTimer::timeout, this, &FileProtocolSocket::confirmUnconfirmedPackets);
    receivedUnconfirmedTimeout.setSingleShot(true);
    receivedUnconfirmedTimeout.setInterval(maxPacketConfirmLatency);

    lastPingCheck.start();

}

QDateTime FileProtocolSocket::getLastActivity() const
{
    return lastActivity;
}

void FileProtocolSocket::handleAllDatagrams(bool doHandle)
{
    if(doHandle && !handlesAllDatagrams) {
        QObject::connect(socket, &QUdpSocket::readyRead,
                this, &FileProtocolSocket::filterDatagrams, Qt::QueuedConnection);
    }
    else if(handlesAllDatagrams) {
        QObject::disconnect(socket, &QUdpSocket::readyRead,
                            this, &FileProtocolSocket::filterDatagrams);
    }
}

void FileProtocolSocket::sendFile(QFileInfo filePath)
{
    if(currentFile == nullptr && filePath.isFile() && filePath.isReadable()) {
        currentFile = new QFile(filePath.absoluteFilePath(), this);
        currentFile->open(QIODevice::ReadOnly);
        transferStart.restart();
        transferTimeSpentWaiting = 0;
        FileHeader header(filePath.completeBaseName()+"."+filePath.suffix(), filePath.size(), Checksum::file(currentFile, QCryptographicHash::Md5));
        PacketGuard* guard = sendDatagramGuarded(header, 400, 250);

        emit fileSendStarted();

        currentFile->seek(0);
        currentByte = 0;
        qInfo()<<"Waiting for the file header to be delivered. After that, file transfer will start.";
        QObject::connect(guard, &PacketGuard::deliveredSimple, this, &FileProtocolSocket::fileChunkReady, Qt::QueuedConnection);
    }
}

void FileProtocolSocket::sendNextFileChunk()
{
    //QFile file(currentFile.absoluteFilePath());
    //file.open(QIODevice::ReadOnly);
    //file.seek(currentByte);
    for(int i=0; i<10 && pendingPackets.length()<PENDING_PACKET_LIMIT; ++i) {
        const QByteArray data(currentFile->read(chunkSize));
        if(data.length()>0) {
            FileChunk chunk(currentByte, data);
            sendDatagramGuarded(chunk, 50, 900);

            currentByte+=data.length();
        }
    }

    if(!currentFile->atEnd()) {
        if(pendingPackets.length()<PENDING_PACKET_LIMIT) {
            emit fileChunkReady();
        }
        else {
            //qInfo()<<"Pausing file sending until pending sockets are confirmed.";
            // This holds the connection to object signal, so that we can disconnect it later
            std::shared_ptr<QMetaObject::Connection> connection(new QMetaObject::Connection);
            // if this is true, then our connection is disconnected and the signal should be ignored
            std::shared_ptr<bool> disconnectedAlready(new bool);
            *disconnectedAlready = false;
            QTime startedWaiting;
            // resend all packets again forcing other side to reconfirm them
            forceResendPackets();

            startedWaiting.start();

            *connection = connect(this, &FileProtocolSocket::packetLimitNotExceeded, this, [connection, this, disconnectedAlready, startedWaiting]() {
                if(*disconnectedAlready) {
                    //qWarning()<<"Multiple calls to transfer resume lambda function!";
                    return;
                }
                if(this->pendingPackets.length() <PENDING_PACKET_LIMIT-1) {
                    //qInfo()<<"Resuming transfer.";
                    *disconnectedAlready = true;
                    QObject::disconnect(*connection);
                    this->transferTimeSpentWaiting += startedWaiting.elapsed();
                    //qInfo() << "Time spent waiting:"<<this->transferTimeSpentWaiting/1000;
                    emit this->fileChunkReady();
                }
            }/*, Qt::QueuedConnection*/);
        }
    }
    else {
        if(pendingPackets.length() == 0) {
            emit fileSent();
            qDebug()<<"Sent "<<currentFile->size()<<" in "<<transferStart.elapsed()/1000<<" seconds.";
            currentByte = 0;
        }
        currentFile->close();
        delete currentFile;
        currentFile = nullptr;
    }
}
void FileProtocolSocket::fileHeaderReceived(QString name, quint64 size, QByteArray checksum)
{
    if(writeFile!=nullptr) {
        delete writeFile;
    }
    const QString newName(target.address.toString().replace(":", "").replace("/", "")+"_"+name);
    writeFile = new QFile(newName, this);
    writeFile->open(QIODevice::WriteOnly);
    writeFile->resize(size);
    newSize = size;
    receivedData = 0;
    currentChecksum = checksum;

    qInfo()<<"Received file header "<<newName<<" MD5:"<<currentChecksum;
    // If there are any old chunks to write
    Q_FOREACH(const FileChunkStruct& chunk, chunkBuffer) {
        fileChunkReceived(chunk.data, chunk.offset);
    }
    chunkBuffer.clear();
    // confirm receipt of the header
    // for now, just use ping message
    //sendDatagramGuarded(Ping("File header received!").toMessage());
}

void FileProtocolSocket::fileChunkReceived(QByteArray data, quint64 offset)
{
    if(writeFile) {
        writeFile->seek(offset);
        writeFile->write(data);
        receivedData+=data.length();
        if(receivedData>=newSize) {
            // File received
            writeFile->close();
            // Verify checksum
            writeFile->open(QIODevice::ReadOnly);
            QByteArray checksum(Checksum::file(writeFile, QCryptographicHash::Md5));
            if(currentChecksum != checksum) {
                qWarning()<<"Incoming file was damaged!";
            }
            else {
                qInfo()<<"File checksum OK.";
            }
            // Delete the file pointer
            delete writeFile;
            writeFile = nullptr;
        }
    }
    else {
        FileChunkStruct chunk;
        chunk.data = data;
        chunk.offset = offset;
        chunkBuffer<<chunk;
    }
}
void FileProtocolSocket::datagramReceived(QByteArray data)
{
    bool invalid = false;

    receivedBytes += data.length();
    // CRC validation should appear here
    // ...
    const quint32 size = data.size();
    const quint32 REMOTE_CRC = Crc32::readFromByteArray(data, size-4);
    const quint32 MY_CRC = Crc32::calculate(data, 0, data.size()-5);
    if(REMOTE_CRC != MY_CRC) {
        // Ignore this data
        CRCFailures++;
        return;
    }

    QDataStream mainStream(&data, QIODevice::ReadOnly);
    quint32 ID;
    quint32 packetIndex;
    mainStream>>ID>>packetIndex;

    // See if the value was received already
    bool received = false;
    quint16 confirmDelay = 0;
    if(receivedPackets.contains(packetIndex)) {
        received = true;
    }
    //quint32 size = get<quint32>(str);
    if(!received) {
        QByteArray bytes;
        mainStream>>bytes;
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        BasicDataClass* parsedData = nullptr;

        switch(ID) {
        case FileHeader::ID: {
            FileHeader* header = nullptr;
            stream>>header;
            fileHeaderReceived(header->filename, header->size, header->md5);
            parsedData = header;
            break;
        }
        case FileChunk::ID: {
            FileChunk* chunk = nullptr;
            stream>>chunk;
            fileChunkReceived(chunk->data, chunk->startByte);
            parsedData = chunk;
            //fileHeaderReceived(header->filename, header->size);
            break;
        }
        case Ping::ID: {
            //m.data = std::shared_ptr<Ping>(get<Ping*>(stream));
            Ping* ping = nullptr;
            stream>>ping;
            parsedData = ping;
            break;
        }
        case ConfirmReceipt::ID: {
            QList<quint32> idx;
            idx<<packetIndex;
            emit receiptsConfirmed(idx);
            break;
        }
        case ConfirmReceiptMulti::ID: {
            ConfirmReceiptMulti* multi = nullptr;
            stream>>multi;
            emit receiptsConfirmed(multi->indexes);
            // multi packet confirmation is not re-confirmed
            delete multi;
            break;
        }
        default:{
            invalid = true;
                //m.data = std::shared_ptr<ErrorData>(new ErrorData(bytes));
            qWarning()<<"Invalid data chunk received!";
            }
        }
        if(parsedData!=nullptr && !invalid) {
            parsedData->setPacketIndex(packetIndex);

            receivedPackets.enqueue(packetIndex);
            while(receivedPackets.size() > MAX_RECEIVED_QUEUE_LENGTH) {
                quint32 noop = receivedPackets.dequeue();
            }
            // debug and cleanup

            if(parsedData->getID()!=FileChunk::ID)
            {
                const QString msg(parsedData->toString());
                if(!msg.contains("CONFIRM_PACKETS"))
                    qInfo()<<"Received: "<<parsedData->toString();
            }

            // check if this packet must be confirmed immediatelly
            confirmDelay = parsedData->maxConfirmDelay();
            delete parsedData;
        }
    }
    // Confirm receipt of this packet
    // every valid packet must be confirmed
    // packets that are received for a second time are considered valid,
    // as they were parsed before
    // Also, we do not confirm receipt of confirmation packets!
    if(!invalid && ID!=ConfirmReceipt::ID && ID!=ConfirmReceiptMulti::ID) {
        confirmDelay = qMin(confirmDelay, maxPacketConfirmLatency);

        receivedUnconfirmed<<packetIndex;
        if(confirmDelay>0) {
            if(!receivedUnconfirmedTimeout.isActive() || receivedUnconfirmedTimeout.remainingTime()>confirmDelay)
                receivedUnconfirmedTimeout.start(confirmDelay);
        }
        else
            confirmUnconfirmedPackets();
        checkQueueStatus();

    }

}



void FileProtocolSocket::sendDatagram(QByteArray data)
{
    if(target) {
        int bytesWritten = -1;        
        // if socket is connected to something
        if(socket->state() == QAbstractSocket::BoundState && socket->isOpen()) {
            bytesWritten = socket->write(data);
        }
        else {
            bytesWritten = socket->writeDatagram(data, target.address, target.port);
        }
        //qInfo()<<"Sending "+QString::number(data.length())+" bytes of data.";
        if(bytesWritten<=0) {
            QString error(socket->errorString());
            sendFailures++;
            // let's see what happens, if I act as if no error happened
            //throw ConnectionError(QString("Cannot send packet: ")+error);
            //qCritical()<<QString("Cannot send packet: ")+error;
        }
        else {
            // Wait for confirmation of receipt
            sentBytes+=bytesWritten;
//            QDataStream mainStream(&data, QIODevice::ReadOnly);
//            quint32 ID;
//            quint32 packetIndex;
//            mainStream>>ID>>packetIndex;
//            qDebug()<<"Sending packet #"<<packetIndex;
//            if(bytesWritten!=data.size())
//                qDebug()<<"sent "<<data.size()<<" but written "<<bytesWritten;
        }
    }
    else {
        throw NoTargetException();
    }
}
PacketGuard* FileProtocolSocket::sendDatagramGuarded(QByteArray data, quint32 packetIndex)
{
    PacketGuard* guard = new PacketGuard(this, data, 25, 500);
    guard->identifier = packetIndex;
    pendingPackets<<guard;
    QObject::connect(guard, &PacketGuard::sendingData, this, &FileProtocolSocket::sendDatagram, Qt::QueuedConnection);

    guard->start();
    return guard;
}

PacketGuard* FileProtocolSocket::sendDatagramGuarded(const BasicDataClass& data, const quint16 maxAttempts, const quint32 timeout)
{
    PacketGuard* guard = new PacketGuard(this, data.toMessage(), maxAttempts, timeout);
    guard->identifier = data.getPacketIndex();
    // Additional info
    guard->setProperty("STRING", QVariant::fromValue(data.toString()));
    guard->setProperty("IS_FILE_PACKET", QVariant::fromValue(data.getID()==FileHeader::ID || data.getID()==FileChunk::ID));
    if(data.getID()!=FileChunk::ID) {
        const QString msg(data.toString());
        if(!msg.contains("CONFIRM_PACKETS"))
            qInfo()<<"Sending: "<<data.toString();
    } else {
        //guard->timeoutMultiplier = 500;
    }

    pendingPackets<<guard;
    QObject::connect(guard, &PacketGuard::sendingData, this, &FileProtocolSocket::sendDatagram, Qt::QueuedConnection);

    guard->start();
    checkQueueStatus();
    return guard;
}


void FileProtocolSocket::filterDatagram(QByteArray datagram, ClientID source)
{

}

void FileProtocolSocket::filterDatagrams()
{
    while (socket->hasPendingDatagrams()) {
        // This is how it is done in new Qt
        //QNetworkDatagram datagram = listener->receiveDatagram();
        //processTheDatagram(datagram);

        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);
        ClientID addr(sender, senderPort);
        // Apparently, the OS already filters datagrams
        if(datagram.length()>0)
            datagramReceived(datagram);
        else
            qWarning()<<"Empty datagram from"<<(sender.isNull()?"0.0.0.0":sender.toString())<<":"<<senderPort;
        //if(!target || addr==target) {
        //    datagramReceived(datagram);
        //}
    }
}

void FileProtocolSocket::notifyReceipt(QList<quint32> packetIndexes)
{

    QMutableListIterator<PacketGuard*> i(pendingPackets);
    bool hasPendingFilePackets = false;
    while (i.hasNext()) {
        PacketGuard* const guard = i.next();
        if(packetIndexes.contains(guard->identifier)) {
            //qInfo()<<"Receipt of packet #"<<packetIndex<<" ("<<guard->property("STRING")<<") confirmed!";
            guard->confirmationReceived();
            guard->deleteLater();
            i.remove();
        }
        else {
            if(guard->property("IS_FILE_PACKET").toBool()) {
                hasPendingFilePackets = true;
            }
        }
    }
    if(!hasPendingFilePackets && currentByte>0 && currentFile==nullptr) {
        currentByte = 0;
        emit fileSent();
    }
    if(pendingPackets.isEmpty()) {
        emit allPacketsDelivered();
    }
    if(pendingPackets.length() < PENDING_PACKET_LIMIT) {
        emit packetLimitNotExceeded();
    }
}

void FileProtocolSocket::confirmUnconfirmedPackets()
{
    if(receivedUnconfirmed.length()==0) {
        return;
    }
    ConfirmReceiptMulti packets(receivedUnconfirmed);
    sendDatagram(packets.toMessage());
    receivedUnconfirmed.clear();
}

void FileProtocolSocket::forceResendPackets()
{
    //qSort(pendingPackets.begin(), pendingPackets.end(), PacketGuard::CompareAge());
    quint16 resendMax = 10;
    Q_FOREACH(PacketGuard* g, pendingPackets) {
        if(--resendMax == 0)
            break;
        g->timedOut();
    }
}

void FileProtocolSocket::pendingPacketFailed(PacketGuard* packet)
{
    qWarning()<<"Packet #"<<packet->identifier<<" failed to reach destination!";
}

void FileProtocolSocket::checkQueueStatus()
{
    if(pingIsPending)
        return;
    const bool isConfirmPackets = pendingPackets.size()*2 > PENDING_PACKET_LIMIT;
    if(isConfirmPackets || lastPingCheck.elapsed()>1000) {
        pingIsPending = true;
        // remember last ping check time
        lastPingCheck.start();
        PacketGuard* guard = sendDatagramGuarded(Ping(isConfirmPackets?"CONFIRM_PACKETS":"PING"), 800, 80);
        QObject::connect(guard, &PacketGuard::deliveredSimple, [this]() {
            // calculate ping info
            if(pingHistory.size()>=maxPingHistory) {
                quint16 oldestPing = pingHistory.dequeue();
                pingSum-=oldestPing;
            }
            const quint16 ping = lastPingCheck.elapsed();
            pingSum += ping;
            // use average ping to set the max confirm latency
            const double avgPing = pingSum/(double)pingHistory.size();
            this->maxPacketConfirmLatency = (quint32)qRound(avgPing/2.0);

            this->pingIsPending = false;
            //qDebug()<<"Ping:"<<ping<<"ms";
        });
    }
}
