#include "PacketGuard.h"
#include "FileProtocolSocket.h"
#include <QTimer>
PacketGuard::PacketGuard(FileProtocolSocket *parent, const QByteArray& payload, const quint16 maxAttempts, const quint32 timeout)
    : QObject(parent)
    , maxAttempts(maxAttempts)
    , payload(payload)
    , timeoutDuration(timeout)
    , attempts(0)
    , timer(new QTimer(this))
    , delivered_(false)
    , failed_(false)
    , identifier(0)
    , timeoutMultiplier(0)
{
    timer->setSingleShot(true);
    QObject::connect(this, &PacketGuard::sendingData, parent, &FileProtocolSocket::sendDatagram, Qt::QueuedConnection);
    QObject::connect(timer, &QTimer::timeout, this, &PacketGuard::timedOut, Qt::QueuedConnection);
}

void PacketGuard::start()
{
    if(attempts>0)
        throw "You attempted to send a packet twice!";
    sendData();
}

void PacketGuard::confirmationReceived()
{
    timer->stop();
    delivered_ = true;
    emit delivered(this);
    emit deliveredSimple();
}

void PacketGuard::timedOut()
{
    timer->stop();
    if(attempts>=maxAttempts)
        qWarning()<<"Packet #"<<identifier<<"receipt not confirmed!"<<(attempts<maxAttempts?" ... retrying":" ... giving up");
    if(attempts<maxAttempts) {
        sendData();
    }
    else {
        failed_ = true;
        emit failed(this);
        emit failedSimple();
    }
}

void PacketGuard::sendData()
{
    if(attempts >= maxAttempts) {
        // dunno, probably ignore
        return;
    }

    emit sendingData(payload);
    attempts++;
    timer->start(timeoutDuration + timeoutMultiplier*attempts);
    //QTimer::singleShot(timeoutDuration, this, &PacketGuard::timedOut, Qt::QueuedConnection);
}
