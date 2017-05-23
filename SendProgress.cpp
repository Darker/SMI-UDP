#include "SendProgress.h"
#include "FileProtocolSocket.h"
#include <QDebug>
#include <QTextStream>
#include <QTime>
inline QTextStream& qStdout()
{
    static QTextStream r{stdout};
    return r;
}

SendProgress::SendProgress(FileProtocolSocket *parent)
    : QObject(parent)
    , parent(parent)
    , lastSent(0)
    , lastCRCFailures(0)
    , lastSendFailures(0)
    , lastSentBytes(0)
    , lastReceivedBytes(0)
{
    QObject::connect(&timer, &QTimer::timeout, this, &SendProgress::updateInfo);
    QObject::connect(parent, &FileProtocolSocket::fileSent, this, &SendProgress::done, Qt::QueuedConnection);
    QObject::connect(parent, &FileProtocolSocket::fileSendStarted, this, &SendProgress::started, Qt::QueuedConnection);

}
QString SendProgress::size_human(double size)
{
    QStringList list;
    list << "kB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("B");

    while(size >= 1024.0 && i.hasNext())
     {
        unit = i.next();
        size /= 1024.0;
    }
    return QString().setNum(size,'f',2)+" "+unit;
}

QString SendProgress::getTimeString(int time)
{
    QString ret("%1:%2:%3");

    int millis = time % 1000;
    time/=1000;
    int seconds = time % 60;
    time/=60;
    int minutes = time % 60;
    time/=60;
    int hours = time % 60;
    time/=60;

    return ret.arg(hours).arg(minutes).arg(seconds);
}
QString SendProgress::getTimeStringMinSecMs(int time)
{
    QString ret("%1:%2.%3");

    int millis = time % 1000;
    time/=1000;
    int seconds = time % 60;
    time/=60;
    int minutes = time % 60;
    time/=60;
    //int hours = time % 60;
    //time/=60;

    return ret.arg(minutes).arg(seconds).arg(millis);
}
void SendProgress::updateInfo()
{
    quint32 size = parent->sendFileSize();
    quint32 sent = parent->sendBytes();
    if(sent>size)
        sent = size;
    double percent = (sent*100.0)/(double)size;
    if(percent>100)
        percent = 100;

    //QTextStream(stdout)<<qRound(percent)<<"% ("<<size_human(sent)<<" of "<<size_human(size)<<") "<<getTimeString(parent->sendSinceStart().elapsed());//<<"\n";
    // calculate current speed
    quint32 sinceLastCheck = qRound(lastCheck.elapsed()/1000.0);
    quint32 sentSinceLast = sent-lastSent;
    double speed = lastSent==0?0:(sentSinceLast / (double)sinceLastCheck);
    //QTextStream(stdout)<<"  SPEED: "<<size_human(speed)<<"/s";

    //QTextStream(stdout)<<"  Wait: "<<getTimeStringMinSecMs(parent->timeSpentWaitingForConfirmation())<<"";
    // Print real IO speed
    //QTextStream(stdout)<<"  s: "<<size_human((parent->sentBytes-lastSentBytes)/(double)sinceLastCheck)<<"/s r: ";


    //QTextStream(stdout)<<size_human((parent->receivedBytes-lastReceivedBytes)/(double)sinceLastCheck)<<"/s";

    //QTextStream(stdout)<<"\n";
    // If there were errors:
    quint16 CRCFailures = 0;
    quint16 socketFailures = 0;
    if(lastSendFailures<parent->sendFailures || lastCRCFailures<parent->CRCFailures) {
        //QTextStream(stdout)<<"  ERRORS: send="<<(parent->sendFailures-lastSendFailures)<<" CRC="<<(parent->CRCFailures-lastCRCFailures)<<"\n";
        socketFailures = parent->sendFailures-lastSendFailures;
        CRCFailures = parent->CRCFailures-lastCRCFailures;

        lastSendFailures = parent->sendFailures;
        lastCRCFailures = parent->CRCFailures;
    }
    emit progressMessage(ProgressInfo(sent,
                                      size,
                                      percent,
                                      parent->sendSinceStart().elapsed(),
                                      parent->timeSpentWaitingForConfirmation(),
                                      speed,
                                      (parent->sentBytes-lastSentBytes)/(double)sinceLastCheck,
                                      (parent->receivedBytes-lastReceivedBytes)/(double)sinceLastCheck,
                                      socketFailures,
                                      CRCFailures)
                         );

    lastCheck.restart();
    lastSent = sent;
    lastReceivedBytes = parent->receivedBytes;
    lastSentBytes = parent->sentBytes;
}

void SendProgress::done()
{
    timer.stop();
}

void SendProgress::started()
{
    lastSent = 0;
    lastCheck.start();
    timer.setInterval(1000);
    timer.start();
}

QString SendProgress::ProgressInfo::toString() const
{
    return QString("%1% (%2 of %3) %4 SPEED: %5 WAIT: %6 s: %7/s r: %8/s")
            .arg(percentage)
            .arg(size_human(fileBytesSent))
            .arg(size_human(fileBytesTotal))
            .arg(getTimeString(timeElapsed))
            .arg(size_human(fileSpeed))
            .arg(getTimeStringMinSecMs(timeWait))
            .arg(size_human(sendSpeed))
            .arg(size_human(receiveSpeed));
}
