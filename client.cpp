#include <QCoreApplication>
#include <QCommandLineParser>
#include "FileClient.h"
#include <iostream>
#include "QApplicationProfiler.h"
void printUsage(const QString& additionalMessage="") {
    std::cout<< "Parameters: filename [-p port] [-a ip address]\n";
    if(!additionalMessage.isEmpty()) {
        std::cout<<additionalMessage.toStdString()<<'\n';
    }
}

int mainClient(int argc, char *argv[])
{
    QApplicationProfiler a(argc, argv);
    QStringList rawArguments(a.arguments());
    QCommandLineParser args;
    // "test.py" -p 6666 -a 192.168.3.13
    args.addPositionalArgument("filename", QString("file that will be sent"));
    //QCommandLineOption(const QString &name, const QString &description, const QString &valueName = QString(), const QString &defaultValue = QString())
    args.addOption(QCommandLineOption("p", "Port number", "port"));
    args.addOption(QCommandLineOption("a", "Server ip", "IP"));

    args.process(rawArguments);
    QHostAddress serverAddr(QHostAddress::LocalHost);
    quint32 port = 6660;
    QString targetFile("");

    if(rawArguments.length()>1) {
        QFileInfo path(rawArguments[1]);
        if(path.isFile() && path.isReadable()) {
            targetFile = path.absoluteFilePath();
        }
        else {
            printUsage(QString("Error: file %1 does not exist or isn't readable.").arg(path.absoluteFilePath()));
        }
    }
    else {
        printUsage("Error: At least the filename must be specified.");
    }
    if(args.isSet("p")) {
        qint64 portNum = args.value("p").toInt();
        if(portNum>1000) {
            port = portNum;
        }
        else {
            printUsage("Error: port number must be greater than 1000.");
            port = 0;
        }
    }

    if(args.isSet("a")) {
        QHostAddress address(args.value("a"));
        serverAddr = address;
    }
    if(serverAddr.isNull()) {
        printUsage("Error: invalid IP address.");
        port = 0;
    }
    if(port>1000 && !targetFile.isEmpty()) {
        FileClient client(serverAddr, port, &a);
        client.sendFile(targetFile);
        QObject::connect(&client, &FileClient::fileSent, &a, &QCoreApplication::quit, Qt::QueuedConnection);
        return a.exec();
    }
    else {
        return 1;
    }
}
