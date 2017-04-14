#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <FileServer.h>
#include "FileClient.h"
#include <iostream>
int mainServer(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCommandLineParser args;
    QStringList rawArguments(a.arguments());
    args.addOption(QCommandLineOption("p", "Port number", "port", "6660"));
    args.process(rawArguments);
    qint64 port = 6660;
    if(args.isSet("p")) {
        qint64 portNum = args.value("p").toInt();
        if(portNum>1000) {
            port = portNum;
        }
        else {
            std::cout<<"Error: port number must be greater than 1000.\n";
            port = 0;
        }
    }
    if(port!=0) {
        FileServer server(port, &a);
        std::cout<<"Listenning on 0.0.0.0:"<<port<<'\n';
        //FileClient client(QHostAddress::LocalHost, 6660, &a);
        return a.exec();
    }
    else
        return 1;
}
