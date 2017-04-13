#include "DataPacket.h"

/*
DataPacket::DataPacket(quint32 id)
{

}

DataPacket::DataPacket(QByteArray rawData)
{

}
*/



QDataStream&operator>>(QDataStream& str, FileHeader*& ptr)
{
    QString name;
    quint64 length;
    str>>name>>length;
    ptr = new FileHeader(name, length);
    return str;
}

QDataStream&operator>>(QDataStream& str, FileChunk*& ptr)
{
    quint64 startByte;
    QByteArray data;
    str>>startByte>>data;
    ptr = new FileChunk(startByte, data);
    return str;
}

QDataStream&operator>>(QDataStream& str, Ping*& ptr)
{
    QString msg;
    str>>msg;
    ptr = new Ping(msg);
    return str;
}

QDataStream&operator<<(QDataStream& str, const DataPacket& m)
{
    QByteArray bytes(m.data->toBytes());
    return str << (quint32)m.data->getID() << (quint32)bytes.length() << bytes;
}
