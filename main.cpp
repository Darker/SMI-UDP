#include <QCoreApplication>
#include <FileServer.h>
#include "FileClient.h"
int mainClient(int argc, char *argv[]);
int mainServer(int argc, char *argv[]);
int main(int argc, char *argv[])
{
#ifdef COMPILE_CLIENT
    return mainClient(argc, argv);
#else
    return mainServer(argc, argv);
#endif
}
