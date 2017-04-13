#ifndef FILEPROTOCOLSOCKET_H
#define FILEPROTOCOLSOCKET_H
#include <QUdpSocket>
#include <QFile>
#include <QDateTime>
#include "Exception.h"
#include "ClientID.h"
#include <QFileInfo>
class FileChunk;
class FileProtocolSocket : public QObject
{
    Q_OBJECT
public:
    FileProtocolSocket(QUdpSocket* parent, ClientID target);
    const ClientID target;
    QDateTime getLastActivity() const;
    // Use this to have this class listen on datagrams on the socket
    // by default, socket does not handle datagrams
    // If you set doHandle to true, this socket will handle all datagrams
    // and filter those that match the client ID
    void handleAllDatagrams(bool doHandle);
    bool operator==(const ClientID& addr) {
        return target==addr;
    }

signals:
    void fileReceived(QByteArray data);
    // used to make file sending async
    void fileChunkReady();
    void fileSent();
public slots:
    void sendFile(QString filePath) {sendFile(QFileInfo(filePath));}
    void sendFile(QFileInfo filePath);
    // does nothing if file is not open or is already sent
    void sendNextFileChunk();
    void datagramReceived(QByteArray data);
    void fileHeaderReceived(QString name, quint64 size);
    void fileChunkReceived(QByteArray data, quint64 offset);

    void sendDatagram(QByteArray data);
    void filterDatagram(QByteArray datagram, ClientID source);
    // Reads all datagrams from the socket and picks those that match client ID
    void filterDatagrams();
protected:
    QUdpSocket* socket;
    QDateTime lastActivity;
    bool handlesAllDatagrams;

    // send file stuff
    QFile* currentFile;
    quint64 currentByte;
    static const quint32 chunkSize = 50;
    // received file
    QFile* writeFile;
    // this is used to check how much more data do we have to receive
    quint64 newSize;
    quint64 receivedData;
    // list of binary chunks received before header
    struct FileChunkStruct {
        quint64 offset;
        QByteArray data;
    };

    QList<FileChunkStruct> chunkBuffer;
};



class NoTargetException: public Exception {

};
class ConnectionError: public Exception {
public:
    ConnectionError(const QString& err) : Exception(err) {}
};

#endif // FILEPROTOCOLSOCKET_H
