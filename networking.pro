QT += core
QT -= gui
android {
    message("* Compiling for android, GUI is encessary:")
    QT += gui
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
}
QT += network
CONFIG += c++11
#dddd
TARGET = networking
CONFIG += console
CONFIG -= app_bundle

#QT += private
#IP KRISTYNA: 192.168.3.11
#IP HOME PC: 192.168.1.163

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
    Checksum.cpp \
    PacketGuard.cpp \
    SendProgress.cpp \
    QApplicationProfiler.cpp \
    OffsetRangeMap.cpp \
    crc32.cpp


HEADERS += \
    FileServer.h \
    FileProtocolSocket.h \
    ClientID.h \
    DataConvertor.h \
    DataPacket.h \
    Exception.h \
    FileClient.h \
    Checksum.h \
    PacketGuard.h \
    SendProgress.h \
    QApplicationProfiler.h \
    OffsetRangeMap.h \
    crc32.h
#
android {
    QT += gui
    FORMS += \
        AndroidMainWindow.ui
    HEADERS += \
        AndroidMainWindow.h
    SOURCES += \
        AndroidMainWindow.cpp \
        android.cpp
    DEFINES+=ANDROID

    DISTFILES += \
        android/AndroidManifest.xml \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/gradlew \
        android/res/values/libs.xml \
        android/build.gradle \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew.bat

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}



