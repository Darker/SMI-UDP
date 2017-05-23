#include "QApplicationProfiler.h"
#include <QElapsedTimer>
#ifdef ANDROID
#include <QtCore/5.5.1/QtCore/private/qobject_p.h>
#else
#include <QtCore/5.6.0/QtCore/private/qobject_p.h>
#endif

#include <QMetaMethod>
#include <QDebug>
#include <QString>
//#include <QMetaCallEvent>
QApplicationProfiler::QApplicationProfiler(int& argc, char** argv)
    : QCoreApplication(argc, argv)
{

}
static const QString methodTypes[] = {QString("Method"), QString("Signal"), QString("Slot"), QString("Constructor")};
bool QApplicationProfiler::notify(QObject* target, QEvent* event)
{
    if(QMetaCallEvent* metaCall = static_cast<QMetaCallEvent*>(event)) {

        //qDebug() << "Metacall:" << target << slot.methodSignature();

        QElapsedTimer timer;
        timer.start();
        const QMetaMethod slot = target->metaObject()->method(metaCall->id());
        bool result = QCoreApplication::notify(target, event);
        quint32 elapsed = timer.elapsed();
        if(elapsed>10) {

            QString slotName = slot.isValid()?slot.methodSignature():QString("!INVALID[")+QString::number(metaCall->id())+"]!";
            slotName.prepend("] ");
            if(slot.methodType()>=0 && slot.methodType()<4) {
                slotName.prepend(methodTypes[slot.methodType()]);

            }else {
                slotName.prepend("Unknown");
            }
            slotName.prepend("[");
            qDebug() << "The slow operation" << slotName<< "took" << timer.elapsed() << "milliseconds";
        }
        return result;
    }
    else {
        return QCoreApplication::notify(target, event);
    }
}
