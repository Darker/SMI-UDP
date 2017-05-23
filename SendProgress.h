#ifndef SENDPROGRESS_H
#define SENDPROGRESS_H

#include <QObject>
#include <QTimer>
#include <QTime>
class FileProtocolSocket;
class SendProgress : public QObject
{
    Q_OBJECT
public:
    explicit SendProgress(FileProtocolSocket *parent = 0);

    class ProgressInfo {
    public:
        ProgressInfo(quint64 fileBytesSent, quint64 fileBytesTotal, double percentage, const quint32 timeElapsed, const quint32 timeWait, double fileSpeed, double sendSpeed, double receiveSpeed, quint16 socketErrors, quint16 crcErrors)
            : fileBytesSent(fileBytesSent)
            , fileBytesTotal(fileBytesTotal)
            , percentage(percentage)
            , timeElapsed(timeElapsed)
            , timeWait(timeWait)
            , fileSpeed(fileSpeed)
            , sendSpeed(sendSpeed)
            , receiveSpeed(receiveSpeed)
            , socketErrors(socketErrors)
            , crcErrors(crcErrors) {}

        const quint64 fileBytesSent;
        const quint64 fileBytesTotal;
        const double percentage;
        const quint32 timeElapsed;
        const quint32 timeWait;
        const double fileSpeed;
        const double sendSpeed;
        const double receiveSpeed;
        const quint16 socketErrors;
        const quint16 crcErrors;

        QString toString() const;
    };

    // returns size as human readable rounded string
    static QString size_human(double size);
    // Converts time to hours minutes and seconds
    static QString getTimeString(int time);
    // Converts time to minutes, seconds and milliseconds
    static QString getTimeStringMinSecMs(int time);
signals:
    void progressMessage(ProgressInfo);
public slots:
    void updateInfo();
    void done();
    void started();
protected:
    QTimer timer;
    FileProtocolSocket* parent;
    // used to compare speed
    quint32 lastSent;
    QTime lastCheck;
    quint32 lastSendFailures;
    quint32 lastCRCFailures;

    quint64 lastSentBytes;
    quint64 lastReceivedBytes;
};

#endif // SENDPROGRESS_H
