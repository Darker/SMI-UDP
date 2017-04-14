#include "FileProtocolSocket.h"
#include "DataPacket.h"
#include <QDebug>
FileProtocolSocket::FileProtocolSocket(QUdpSocket* parent, ClientID target) :
    QObject(parent)
  , socket(parent)
  , target(target)
  , handlesAllDatagrams(false)
  , currentFile(nullptr)
  , writeFile(nullptr)
{
    QObject::connect(this, &FileProtocolSocket::fileChunkReady, this, &FileProtocolSocket::sendNextFileChunk, Qt::QueuedConnection);
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
        FileHeader header(filePath.completeBaseName()+"."+filePath.suffix(), filePath.size());
        sendDatagram(header.toMessage());
        currentFile = new QFile(filePath.absoluteFilePath(), this);
        currentFile->open(QIODevice::ReadOnly);
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
    currentByte+=data.length();
    sendDatagram(chunk.toMessage());

    if(!currentFile->atEnd()) {
        emit fileChunkReady();
    }
    else {
        currentFile->close();
        delete currentFile;
        emit fileSent();
    }
}
void FileProtocolSocket::fileHeaderReceived(QString name, quint64 size)
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
    // If there are any old chunks to write
    Q_FOREACH(const FileChunkStruct& chunk, chunkBuffer) {
        fileChunkReceived(chunk.data, chunk.offset);
    }
    chunkBuffer.clear();
    // confirm receipt of the header
    // for now, just use ping message
    sendDatagram(Ping("File header received!").toMessage());
}

void FileProtocolSocket::fileChunkReceived(QByteArray data, quint64 offset)
{
    if(writeFile) {
        writeFile->seek(offset);
        writeFile->write(data);
        receivedData+=data.length();
        if(receivedData>=newSize) {
            writeFile->close();
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
    QDataStream mainStream(&data, QIODevice::ReadOnly);
    quint32 ID;
    mainStream>>ID;
    //quint32 size = get<quint32>(str);
    QByteArray bytes;
    mainStream>>bytes;
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    BasicDataClass* parsedData = nullptr;

    switch(ID) {
    case FileHeader::ID: {
        FileHeader* header = nullptr;
        stream>>header;
        fileHeaderReceived(header->filename, header->size);
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
    case 666: {}
    default:{
            //m.data = std::shared_ptr<ErrorData>(new ErrorData(bytes));
        qWarning()<<"Invalid data chunk received!";
        }
    }
    if(parsedData!=nullptr) {
        qInfo()<<"Received: "<<parsedData->toString();
        delete parsedData;
    }
}



void FileProtocolSocket::sendDatagram(QByteArray data)
{
    if(target) {
        qInfo()<<"Sending "+QString::number(data.length())+" bytes of data.";
        if(socket->writeDatagram(data, target.address, target.port)<0) {
            QString error(socket->errorString());
            throw ConnectionError(QString("Cannot send packet: ")+error);
        }
    }
    else {
        throw NoTargetException();
    }
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
