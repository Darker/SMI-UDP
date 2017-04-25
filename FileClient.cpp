#include "FileClient.h"
#include "ClientID.h"
#include "DataPacket.h"
#include "SendProgress.h"
#include <QTimer>

FileClient::FileClient(QHostAddress serverAddr, qint64 port, QObject *parent)
  : QObject(parent)
  , socket(new QUdpSocket(this))
  , smartSocket(new FileProtocolSocket(socket, ClientID(serverAddr, port)))
{
    QObject::connect(smartSocket, &FileProtocolSocket::fileSent, this, &FileClient::fileSent, Qt::QueuedConnection);
    //QObject::connect(smartSocket, FileProtocol)
    smartSocket->handleAllDatagrams(true);
    // Make user informer
    new SendProgress(smartSocket);
    //socket->bind(serverAddr, port);
    //QTimer::singleShot(500, this, &FileClient::sayHello);
    //QTimer::singleShot(1000, this, &FileClient::sendFile);
}

void FileClient::receiveData()
{

}


void FileClient::sendFile(QString filename)
{
    smartSocket->sendFile(filename);
}

//void FileClient::sendFile()
//{
//    smartSocket->sendFile("../networking/test.py");
//}
