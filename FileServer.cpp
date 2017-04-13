#include "FileServer.h"
#include <QUdpSocket>
//#include <QHostAddress>
//#include <QNetworkDatagram>
FileServer::FileServer(qint64 port, QObject *parent, QHostAddress listenAddr)
    : QObject(parent)
    , listener(new QUdpSocket(this))
    , serverAddress(listenAddr, port)
{
    listener->bind(listenAddr.isNull()?QHostAddress::Any:listenAddr, 6660);

    connect(listener, &QUdpSocket::readyRead,
            this, &FileServer::readPendingDatagrams, Qt::QueuedConnection);

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
    client->datagramReceived(array);
}
