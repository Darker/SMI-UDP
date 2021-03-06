#ifndef FILEPROTOCOLSOCKET_H
#define FILEPROTOCOLSOCKET_H
#include <QUdpSocket>
#include <QFile>
#include <QDateTime>
#include "Exception.h"
#include "ClientID.h"
#include <QFileInfo>
#include <QQueue>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>

class PacketGuard;
class FileChunk;
class BasicDataClass;
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
    void fileReceived(QString filename);
    // used to make file sending async
    void fileChunkReady();
    void fileSent();
    // When outgoing file transfer starts
    void fileSendStarted();
    // When packet delivery is confirmed
    void receiptsConfirmed(QList<quint32> packetIndex);
    // Used to init cleanup
    void fatalError();
    // Emitted when all packets are confirmed to be delivered
    // this is a good time to send more data or exit
    void allPacketsDelivered();
    // When limit of pending packets is not exceeded
    // emitted every time a packet is confirmed
    // and limit condition is met
    // Therefore is a good way of deferring sending after limit is no longer exceeded
    void packetLimitNotExceeded();
    // Emitted if broadcast is received
    void broadcastReceived(ClientID addr);
public slots:
    void sendFile(QString filePath) {sendFile(QFileInfo(filePath));}
    void sendFile(QFileInfo filePath);
    // does nothing if file is not open or is already sent
    void sendNextFileChunk();
    void datagramReceived(QByteArray data, ClientID addr);
    void fileHeaderReceived(QString name, quint64 size, QByteArray checksum);
    void fileChunkReceived(QByteArray data, quint64 offset);

    void sendDatagram(QByteArray data);

    PacketGuard* sendDatagramGuarded(QByteArray data, quint32 packetIndex);
    PacketGuard* sendDatagramGuarded(const BasicDataClass& data, const quint16 maxAttempts = 10, const quint32 timeout=200);

    void filterDatagram(QByteArray datagram, ClientID source);
    // Reads all datagrams from the socket and picks those that match client ID
    void filterDatagrams();

    // Call this to notify packet of given index that it was received
    void notifyReceipt(QList<quint32> packetIndexes);
    // Fire by timeout when it's time to confirm all buffered acket ids
    void confirmUnconfirmedPackets();
    // If the queue of pending packets is overcrowded, resend tem forcefully
    void forceResendPackets();

    void pendingPacketFailed(PacketGuard* packet);
    // Gives the ammount of bytes transferred so far
    // No behavior defined when file is not being sent
    quint32 sendBytes() {return currentByte;}
    // Gives the size of file being sent
    quint32 sendFileSize() {
        return currentFile!=nullptr?currentFile->size():0;
    }
    QTime sendSinceStart() {return transferStart;}
    quint32 timeSpentWaitingForConfirmation() const {return transferTimeSpentWaiting;}

protected:
    QUdpSocket* socket;
    QDateTime lastActivity;
    bool handlesAllDatagrams;
    // Pending unconfirmed packets
    QList<PacketGuard*> pendingPackets;
    // Tries to clear pending packet queue by sending ping packet to the server which forces
    // the server to instantly send all packet confirmations
    // It also manages ping related information
    void checkQueueStatus();
    bool pingIsPending;
    static const quint16 maxPingHistory = 20;
    QQueue<quint16> pingHistory;
    // average ping = pingSum/pingHistory.size()
    quint16 pingSum;
    QElapsedTimer lastPingCheck;

    // list of packets that were received correctly and are to be ignored if received again
    // (although the receipt must be reconfirmed, otherwise the disconnect error will be trigerred)
    QQueue<quint32> receivedPackets;
    // This buffers received packets that were not confirmed yet
    // every once in a while, all peding sockets are confirmed
    QList<quint32> receivedUnconfirmed;
    QTimer receivedUnconfirmedTimeout;

    static const quint32 MAX_RECEIVED_QUEUE_LENGTH = 1000;
    // this is a recommended value
    // the program should avoid even generating packets when the ammount of pending packets is above this
    static const quint32 PENDING_PACKET_LIMIT = 100;
    // maximum number of ms to wait before sending multi confirm packet
    // this is auto adjusted based on latency, default is 1
    quint16 maxPacketConfirmLatency;

    static const quint32 chunkSize = 450;
    // send file stuff
    QFile* currentFile;
    QTime transferStart;
    // [ms] time spent waiting for packet confirmation
    quint32 transferTimeSpentWaiting;
    // indicates the byte offset at hich the next chunk must begin
    // If currentFile is nullptr, this also indicates that fileSent was not emmited yet
    quint64 currentByte;
    // checksum of file being received
    QByteArray currentChecksum;
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
public:
    // temporary profiling variables
    quint32 timeSpentReading;
    quint32 timeSpentClearingPending;

    // Raw data transfer
    quint64 receivedBytes;
    quint64 sentBytes;

    // Error counts
    quint32 sendFailures;
    quint32 CRCFailures;
};



class NoTargetException: public Exception {

};
class ConnectionError: public Exception {
public:
    ConnectionError(const QString& err) : Exception(err) {}
};
class BindError: public Exception {
public:
    BindError(const QString& err) : Exception(err) {}
};
#endif // FILEPROTOCOLSOCKET_H
