#include "AndroidMainWindow.h"
#include <QApplication>
#include "FileServer.h"
int mainAndroid(int argc, char *argv[]) {
    QApplication a(argc, argv);

    FileServer server(6660, &a);
    AndroidMainWindow main;
    main.show();
    server.startBroadcast();
    //std::cout<<"Listenning on 0.0.0.0:"<<port<<'\n';
    return a.exec();
}
