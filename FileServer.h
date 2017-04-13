#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
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
public slots:
    void readPendingDatagrams();
    void processDatagram(QByteArray array, QHostAddress ip, qint64 port);
protected:
    ClientID serverAddress;
    QUdpSocket* listener;
    QList<FileProtocolSocket*> connections;
};

#endif // FILESERVER_H
