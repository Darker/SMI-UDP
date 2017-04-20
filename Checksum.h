#ifndef CHECKSUM_H
#define CHECKSUM_H
#include <QCryptographicHash>
#include <QString>
#include <QByteArray>
QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE
namespace Checksum {
QByteArray file(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm);

QByteArray file(QFile* file,
                        QCryptographicHash::Algorithm hashAlgorithm);
}
#endif // CHECKSUM_H
