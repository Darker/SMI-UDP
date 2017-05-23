#include <QCoreApplication>
#include <iostream>
#include <FileServer.h>
#include "OffsetRangeMap.h"
#include <QDebug>
#include "FileClient.h"
int mainClient(int argc, char *argv[]);
int mainServer(int argc, char *argv[]);
int mainAndroid(int argc, char *argv[]);
int main(int argc, char *argv[])
{

#ifdef COMPILE_CLIENT
    std::cout<<"Starting the File client!\n";
    return mainClient(argc, argv);
#elif defined(ANDROID)
    return mainAndroid(argc, argv);
#else
    std::cout<<"Starting the File server!\n";
    return mainServer(argc, argv);
#endif
}
