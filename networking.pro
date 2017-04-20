QT += core
QT -= gui
QT += network
CONFIG += c++11
#ddd
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
    server.cpp \
    CRC.cpp \
    Checksum.cpp \
    PacketGuard.cpp \
    SendProgress.cpp

HEADERS += \
    FileServer.h \
    FileProtocolSocket.h \
    ClientID.h \
    DataConvertor.h \
    DataPacket.h \
    Exception.h \
    FileClient.h \
    CRC.h \
    Checksum.h \
    PacketGuard.h \
    SendProgress.h
#
