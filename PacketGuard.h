#ifndef PACKETGUARD_H
#define PACKETGUARD_H

#include <QObject>
/**
 * This class is responsible for ensuring that a packet correctly arrives to the destination.
 * The class waits until packet confirmation packet is received from the server. It resends
 * the original packet given number of times untill confirmation arrives.
 *
 * If number of re-send attempts is exhausted and the receipt wasn't confirmed, the class
 * will notify that the packed failed to be sent.
 *
 * Do note that instead of using the `delivered` signal, one might simply delete the guard
 * instance as soon as confirmation arrives.
 */
QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE
class FileProtocolSocket;

class PacketGuard : public QObject
{
    Q_OBJECT
public:
    explicit PacketGuard(FileProtocolSocket *parent, const QByteArray& payload, const quint16 maxAttempts=3, const quint32 timeout=200);
    // Returns true if the underlying data packet was correctly delivered
    // to destination
    bool isDelivered() const {return delivered_;}
    // Returns true if all efforts to deliver failed
    bool isFailed() const {return failed_;}
    quint32 identifier;
    // Timeout can increase with attempts, reducing bandwidth and increasing latency
    int timeoutMultiplier;
signals:
    void failed(PacketGuard* self);
    void delivered(PacketGuard* self);
    // Emitted whenever data should be sent
    void sendingData(QByteArray data);

    void failedSimple();
    void deliveredSimple();
public slots:
    // Called when the payload should be sent the first time
    void start();
    // call this slot to tell guard that the confirmation
    // packet was received and valid
    void confirmationReceived();
    // This slot is used to indicate that confirmation timed out
    // Payload will be re-sent at this point
    void timedOut();
protected:
    FileProtocolSocket* parent;
    const QByteArray payload;
    QTimer* timer;
    const int maxAttempts;
    const int timeoutDuration;
    int attempts;
    bool delivered_;
    bool failed_;




    // sends data, increments number of attempts and starts timeout
    void sendData();
};

#endif // PACKETGUARD_H
