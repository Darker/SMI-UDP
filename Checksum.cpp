#include "Checksum.h"
#include <QFile>
namespace Checksum {
QByteArray file(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        return file(&f, hashAlgorithm);
    }
    return QByteArray();
}

QByteArray file(QFile* fileObj,
                        QCryptographicHash::Algorithm hashAlgorithm)
{

    if (fileObj!=nullptr && fileObj->isOpen() && fileObj->isReadable()) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(fileObj)) {
            return hash.result();
        }
    }
    return QByteArray();
}

}
