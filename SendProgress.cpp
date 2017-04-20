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
{
    QObject::connect(&timer, &QTimer::timeout, this, &SendProgress::updateInfo);
    QObject::connect(parent, &FileProtocolSocket::fileSent, this, &SendProgress::done, Qt::QueuedConnection);
    QObject::connect(parent, &FileProtocolSocket::fileSendStarted, this, &SendProgress::started, Qt::QueuedConnection);

}
QString size_human(double size)
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

QString getTimeString(int time)
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
void SendProgress::updateInfo()
{
    quint32 size = parent->sendFileSize();
    quint32 sent = parent->sendBytes();
    double percent = (sent*100.0)/(double)size;

    QTextStream(stdout)<<qRound(percent)<<"% ("<<size_human(sent)<<" of "<<size_human(size)<<") "<<getTimeString(parent->sendSinceStart().elapsed());//<<"\n";
    // calculate current speed
    quint32 sinceLastCheck = qRound(lastCheck.elapsed()/1000.0);
    quint32 sentSinceLast = sent-lastSent;
    double speed = (sentSinceLast / (double)sinceLastCheck);
    QTextStream(stdout)<<"  SPEED: "<<size_human(speed)<<"/s\n";

    lastCheck.restart();
    lastSent = sent;
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
