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

signals:

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
};

#endif // SENDPROGRESS_H
