#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>

#include "FileProtocolSocket.h"
//QT_BEGIN_NAMESPACE
//class QUdpSocket;
//QT_END_NAMESPACE
class FileServer : public QObject
{
    Q_OBJECT
public:
    explicit FileServer(qint64 port, QObject *parent = 0, QHostAddress listenAddr = QHostAddress::Any);

signals:
    void fileSent();
    // Informs about other server nearby
    void broadcastReceived(ClientID address);
public slots:
    void readPendingDatagrams();
    void processDatagram(QByteArray array, QHostAddress ip, qint64 port);
    // Will start broadcasting itself on this port
    void startBroadcast();
    // sends broadcast datagram
    void sendBroadcast();

protected:
    ClientID serverAddress;
    QUdpSocket* listener;
    QList<FileProtocolSocket*> connections;
    QTimer broadcastTimer;
};

#endif // FILESERVER_H
