#include "DataPacket.h"
#include "Checksum.h"
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
    QByteArray checksum;
    str>>name>>length>>checksum;
    ptr = new FileHeader(name, length, checksum);
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

//QDataStream&operator<<(QDataStream& str, const DataPacket& m)
//{
//    QByteArray bytes(m.data->toBytes());
//    return str << (quint32)m.data->getID() << (quint32)bytes.length() << bytes;
//}
QDataStream&operator>>(QDataStream& str, ConfirmReceipt*& ptr)
{
    ptr = new ConfirmReceipt(0);
    return str;
}
FileHeader::FileHeader(const QString& filename, const quint64 size) :
    BasicDataClass()
  , filename(filename)
  , size(size)
  , md5(Checksum::file(filename, QCryptographicHash::Md5))
{}
FileHeader::FileHeader(const QString& filename, const quint64 size, const QByteArray checksum) :
    BasicDataClass()
  , filename(filename)
  , size(size)
  , md5(checksum)
{}

quint32 BasicDataClass::getIndex()
{
    static quint32 index = 0;
    return(++index);
}


