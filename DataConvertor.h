#ifndef DATACONVERTOR_H
#define DATACONVERTOR_H

#include <QObject>
#include <QFile>
/*
 * This class converts received data to objects and converts ontgoing objects to bytes and sends them.
 * the class represents pseudoconnection between client and server.
*/
class DataConvertor : public QObject
{
    Q_OBJECT
public:
    explicit DataConvertor(QObject *parent = 0);

signals:

public slots:
    //void sendFile(QFile file);
    void startReceiveFile();
};

#endif // DATACONVERTOR_H
