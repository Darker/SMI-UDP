#include <QCoreApplication>
#include <FileServer.h>
#include "FileClient.h"
int mainServer(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FileServer server(6660, &a);
    //FileClient client(QHostAddress::LocalHost, 6660, &a);
    return a.exec();
}
