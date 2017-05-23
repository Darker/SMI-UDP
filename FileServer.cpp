#include "FileServer.h"
#include <QUdpSocket>
#include "DataPacket.h"
//#include <QHostAddress>
//#include <QNetworkDatagram>

FileServer::FileServer(qint64 port, QObject *parent, QHostAddress listenAddr)
    : QObject(parent)
    , listener(new QUdpSocket(this))
    , serverAddress(listenAddr, port)
{

    if(!listener->bind(listenAddr.isNull()?QHostAddress::Any:listenAddr, port)) {
        throw BindError(QString("Cannot bind to target port %1!").arg(port));
    }

    connect(listener, &QUdpSocket::readyRead,
            this, &FileServer::readPendingDatagrams, Qt::QueuedConnection);
    QObject::connect(&broadcastTimer, &QTimer::timeout, this, &FileServer::sendBroadcast);
}

void FileServer::readPendingDatagrams()
{
    while (listener->hasPendingDatagrams()) {
        // This is how this is done in new QT
        //QNetworkDatagram datagram = listener->receiveDatagram();
        //processTheDatagram(datagram);
        // This is how it is done in old QT
        QByteArray datagram;
        datagram.resize(listener->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        listener->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);
        processDatagram(datagram, sender, senderPort);
    }
}

void FileServer::processDatagram(QByteArray array, QHostAddress ip, qint64 port)
{
    const ClientID source(ip, port);

    FileProtocolSocket* client = nullptr;
    Q_FOREACH(FileProtocolSocket* socket, connections) {
        if(socket->target == source) {
            client = socket;
            break;
        }
    }
    if(client==nullptr) {
        connections<<(client = new FileProtocolSocket(listener, source));
    }
    client->datagramReceived(array, source);
}

void FileServer::startBroadcast()
{
    if(!broadcastTimer.isActive()) {
        broadcastTimer.setInterval(800);
        broadcastTimer.start();
    }
}

void FileServer::sendBroadcast()
{
    static const Broadcast constBroadcastPacket = Broadcast();
    static const QByteArray broadcastData = constBroadcastPacket.toMessage();
    listener->writeDatagram(broadcastData, QHostAddress::Broadcast, serverAddress.port);
}
