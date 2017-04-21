#ifndef QAPPLICATIONPROFILER_H
#define QAPPLICATIONPROFILER_H

#include <QCoreApplication>
#include <QMap>
class QApplicationProfiler : public QCoreApplication
{
public:
    QApplicationProfiler(int & argc, char** argv);
public slots:
    virtual bool notify(QObject *, QEvent *);

protected:
    // simple log of time durations
    QMap<QString, quint32> timePerSlot;
};

#endif // QAPPLICATIONPROFILER_H
