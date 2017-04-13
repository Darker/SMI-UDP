#ifndef DATAPACKET_H
#define DATAPACKET_H
#include <QtNumeric>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <memory>
/*class Sendable {
    QByteArray toByteArray() const = 0;
    operator QByteArray() const {
        return toByteArray();
    }

    quint32 packetID() const = 0;
};*/

template <typename T>
inline T get(QDataStream& str) {
    T value;
    str>>value;
    return value;
}

/** Specific data classes **/
class BasicDataClass {
public:
    static const quint32 ID = 0;
    virtual quint32 getID() const = 0;
    BasicDataClass() {

    }

    virtual QByteArray toBytes() const {
        QByteArray buf;
        QDataStream stream(&buf, QIODevice::WriteOnly);
        toBytes(stream);
        return buf;
    }
    // creates the wrapping message structure around the main data
    virtual QByteArray toMessage() const {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream<<(quint32)getID()<<toBytes();
        return buffer;
    }
    virtual QString toString() const = 0;
    virtual ~BasicDataClass() {}

protected:
    // put the individual parts to the stream in this method
    virtual void toBytes(QDataStream& stream) const = 0;
};
typedef std::shared_ptr<BasicDataClass> BDataPtr;
class ErrorData: public BasicDataClass {
public:
    static const quint32 ID = 666;
    virtual quint32 getID() const {return ID;}
    ErrorData(const QByteArray& data) :
        BasicDataClass()
      , originalData(data)
    {}
    virtual QByteArray toBytes() const override {
        return originalData;
    }
    virtual QString toString() const {return "ErrorData";}
protected:
    virtual void toBytes(QDataStream& stream) const override {
        stream<<originalData;
    }
    QByteArray originalData;
};
class FileHeader: public BasicDataClass {
public:
    static const quint32 ID = 1;
    virtual quint32 getID() const {return ID;}

    FileHeader(const QString& filename, const quint64 size) :
        BasicDataClass()
      , filename(filename)
      , size(size)
    {}
    const QString filename;
    const quint64 size;

    virtual QString toString() const {return "FileHeader - "+filename;}

protected:
    virtual void toBytes(QDataStream& stream) const override {
        stream<<(QString)filename<<(quint64)size;
    }

};


class FileChunk: public BasicDataClass {
public:
    static const quint32 ID = 2;
    virtual quint32 getID() const {return ID;}

    FileChunk(const quint64 startByte, const QByteArray& data) :
        BasicDataClass()
      , startByte(startByte)
      , data(data)
    {}
    virtual QString toString() const {return "FileChunk - "+QString::number(data.length(), 10)+QString("bytes");}
    const quint64 startByte;
    const QByteArray data;
protected:
    virtual void toBytes(QDataStream& stream) const override {
        stream<<startByte<<data;
    }

};

class Ping: public BasicDataClass {
public:
    static const quint32 ID = 133;
    virtual quint32 getID() const override {return ID;}
    Ping(const QString& message) :
        BasicDataClass()
      , message(message)
    {}
    const QString message;

    virtual QByteArray toBytes() const override {
        return BasicDataClass::toBytes();
    }
    virtual QString toString() const {return "Ping - "+message;}
protected:
    virtual void toBytes(QDataStream& stream) const {
        stream<<message;
    }
};


QDataStream& operator>>(QDataStream& str, FileHeader*& ptr);

QDataStream& operator>>(QDataStream& str, FileChunk*& ptr);

QDataStream& operator>>(QDataStream& str, Ping*& ptr);

class DataPacket
{
public:
    // If parsing fails, value will be ErrorData instance
    BDataPtr data;
};

QDataStream& operator<<(QDataStream& str, const DataPacket& m);
/*
QDataStream& operator>>(QDataStream& str, DataPacket& m)
{
    quint32 ID = get<quint32>(str);
    //quint32 size = get<quint32>(str);
    QByteArray bytes = get<QByteArray>(str);
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    switch(ID) {
    case FileHeader::ID: {
        m.data = std::shared_ptr<FileHeader>(get<FileHeader*>(stream));
        break;
    }
    case Ping::ID: {
        m.data = std::shared_ptr<Ping>(get<Ping*>(stream));
        break;
    }
    case 666: {}
    default:{
            m.data = std::shared_ptr<ErrorData>(new ErrorData(bytes));
        }
    }

    return str;
}
*/

#endif // DATAPACKET_H
