#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
class FileProtocolSocket;
class FileClient : public QObject
{
    Q_OBJECT
public:
    explicit FileClient(QHostAddress serverAddr, qint64 port, QObject *parent = 0);
    FileProtocolSocket* getSocket() {
        return smartSocket;
    }

signals:
    void fileSent();
public slots:
    void receiveData();

    //void sendFile();
    void sendFile(QString filename);
protected:
    QUdpSocket* socket;
    FileProtocolSocket* smartSocket;
};

#endif // FILECLIENT_H
