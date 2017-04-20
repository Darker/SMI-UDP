#include "FileProtocolSocket.h"
#include "DataPacket.h"
#include <QDebug>
#include <QFile>
#include "Checksum.h"
#include "PacketGuard.h"
#include <functional>
FileProtocolSocket::FileProtocolSocket(QUdpSocket* parent, ClientID target) :
    QObject(parent)
  , socket(parent)
  , target(target)
  , handlesAllDatagrams(false)
  , currentFile(nullptr)
  , writeFile(nullptr)
  , currentByte(0)
{
    QObject::connect(this, &FileProtocolSocket::fileChunkReady, this, &FileProtocolSocket::sendNextFileChunk, Qt::QueuedConnection);
    QObject::connect(this, &FileProtocolSocket::receiptConfirmed, this, &FileProtocolSocket::notifyReceipt, Qt::QueuedConnection);
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
        FileHeader header(filePath.completeBaseName()+"."+filePath.suffix(), filePath.size(), Checksum::file(currentFile, QCryptographicHash::Md5));
        sendDatagramGuarded(header);

        emit fileSendStarted();

        currentFile->seek(0);
        currentByte = 0;
        emit fileChunkReady();
    }
}

void FileProtocolSocket::sendNextFileChunk()
{
    //QFile file(currentFile.absoluteFilePath());
    //file.open(QIODevice::ReadOnly);
    //file.seek(currentByte);
    QByteArray data(currentFile->read(chunkSize));
    if(data.length()==0)
        return;

    FileChunk chunk(currentByte, data);
    sendDatagramGuarded(chunk);

    currentByte+=data.length();

    if(!currentFile->atEnd()) {
        if(pendingPackets.length()<5) {
            emit fileChunkReady();
        }
        else {
            //qInfo()<<"Pausing file sending until pending sockets are confirmed.";
            // This holds the connection to tist object signal, so that we can disconnect it later
            std::shared_ptr<QMetaObject::Connection> connection(new QMetaObject::Connection);
            // if this is true, then our connection is disconnected and the signal should be ignored
            std::shared_ptr<bool> disconnectedAlready(new bool);
            *disconnectedAlready = false;

            *connection = connect(this, &FileProtocolSocket::packetLimitNotExceeded, this, [connection, this, disconnectedAlready]() {
                if(*disconnectedAlready) {
                    //qWarning()<<"Multiple calls to transfer resume lambda function!";
                    return;
                }
                if(this->pendingPackets.length() < 5) {
                    //qInfo()<<"Resuming transfer.";
                    *disconnectedAlready = true;
                    QObject::disconnect(*connection);
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
    // CRC validation should appear here
    // ...

    QDataStream mainStream(&data, QIODevice::ReadOnly);
    quint32 ID;
    quint32 packetIndex;
    mainStream>>ID>>packetIndex;

    // See if the value was received already
    bool received = false;
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
            emit receiptConfirmed(packetIndex);
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
                qInfo()<<"Received: "<<parsedData->toString();
            delete parsedData;
        }
    }
    // Confirm receipt of this packet
    // every valid packet must be confirmed
    // packets that are received for a second time are considered valid,
    // as they were parsed before
    // Also, we do not confirm receipt of confirmation packets!
    if(!invalid && ID!=ConfirmReceipt::ID)
        sendDatagram(ConfirmReceipt(packetIndex).toMessage());
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
            // let's see what happens, if I act as if no error happened
            throw ConnectionError(QString("Cannot send packet: ")+error);
        }
        else {
            // Wait for confirmation of receipt
        }
    }
    else {
        throw NoTargetException();
    }
}
void FileProtocolSocket::sendDatagramGuarded(QByteArray data, quint32 packetIndex)
{
    PacketGuard* guard = new PacketGuard(this, data);
    guard->identifier = packetIndex;
    pendingPackets<<guard;
    QObject::connect(guard, &PacketGuard::sendingData, this, &FileProtocolSocket::sendDatagram, Qt::QueuedConnection);

    guard->start();
}

void FileProtocolSocket::sendDatagramGuarded(const BasicDataClass& data)
{
    PacketGuard* guard = new PacketGuard(this, data.toMessage());
    guard->identifier = data.getPacketIndex();
    // Additional info
    guard->setProperty("STRING", QVariant::fromValue(data.toString()));
    guard->setProperty("IS_FILE_PACKET", QVariant::fromValue(data.getID()==FileHeader::ID || data.getID()==FileChunk::ID));
    if(data.getID()!=FileChunk::ID)
        qInfo()<<"Sending: "<<data.toString();

    pendingPackets<<guard;
    QObject::connect(guard, &PacketGuard::sendingData, this, &FileProtocolSocket::sendDatagram, Qt::QueuedConnection);

    guard->start();
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

void FileProtocolSocket::notifyReceipt(quint32 packetIndex)
{
    QMutableListIterator<PacketGuard*> i(pendingPackets);
    bool hasPendingFilePackets = false;
    while (i.hasNext()) {
        PacketGuard* const guard = i.next();
        if(guard->identifier == packetIndex) {
            //qInfo()<<"Receipt of packet #"<<packetIndex<<" ("<<guard->property("STRING")<<") confirmed!";
            delete guard;
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

void FileProtocolSocket::pendingPacketFailed(PacketGuard* packet)
{
    qWarning()<<"Packet #"<<packet->identifier<<" failed to reach destination!";
}
