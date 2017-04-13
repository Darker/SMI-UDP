QT += core
QT -= gui
QT += network
CONFIG += c++11

TARGET = networking
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    FileServer.cpp \
    FileProtocolSocket.cpp \
    ClientID.cpp \
    DataConvertor.cpp \
    DataPacket.cpp \
    Exception.cpp \
    FileClient.cpp \
    client.cpp \
    server.cpp

HEADERS += \
    FileServer.h \
    FileProtocolSocket.h \
    ClientID.h \
    DataConvertor.h \
    DataPacket.h \
    Exception.h \
    FileClient.h
#
