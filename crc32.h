#ifndef CRC32_H
#define CRC32_H

#include <QString>
#include <QMap>
#include <QReadWriteLock>

class Crc32
{
protected:
    static quint32 crc_table[256];
    bool crc_table_initialized = false;
    static QReadWriteLock crc_table_lock;

    QMap<int, quint32> instances;

public:
    Crc32();

    quint32 calculateFromFile(QString filename);
    static quint32 calculate(const QByteArray&);
    static quint32 calculate(const QByteArray&, quint32 start, quint32 end);
    void initInstance(int i);
    void pushData(int i, char *data, int len);
    quint32 releaseInstance(int i);

    static quint32 readFromByteArray(const QByteArray& buffer, quint32 position);
};

#endif // CRC32_H
