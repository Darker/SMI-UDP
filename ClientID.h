#ifndef CLIENTID_H
#define CLIENTID_H
#include <QHostAddress>

class ClientID
{
public:
    ClientID(const QHostAddress& address, const qint64 port);
    const QHostAddress address;
    const qint64 port;

    bool operator==(const ClientID& other) const {
        return other.port == port && other.address==address;
    }
    bool isValid() const {
        return port>0 && !address.isNull();
    }
    operator bool() const {
        return isValid();
    }
};

#endif // CLIENTID_H
