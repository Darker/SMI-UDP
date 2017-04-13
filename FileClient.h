#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include "FileProtocolSocket.h"
class FileClient : public QObject
{
    Q_OBJECT
public:
    explicit FileClient(QHostAddress serverAddr, qint64 port, QObject *parent = 0);

signals:
    void fileSent();
public slots:
    void receiveData();

    void sayHello();
    //void sendFile();
    void sendFile(QString filename);
protected:
    QUdpSocket* socket;
    FileProtocolSocket* smartSocket;
};

#endif // FILECLIENT_H
