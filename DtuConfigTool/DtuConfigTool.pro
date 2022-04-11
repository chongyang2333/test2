#-------------------------------------------------
#
# Project created by QtCreator 2022-01-12T17:58:25
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DtuConfigTool
TEMPLATE = app

RC_FILE += icon.rc

SOURCES += main.cpp\
    serial_iii.cpp \
    httpclient.cpp \
    pdlog.cpp \
    pdhttpapi.cpp \
    dtucfgthread.cpp   \
    mainwindow.cpp \
    pdserial.cpp \
    atcommandsendthread.cpp \
    iqctestthread.cpp \
    signalstrengthtestthread.cpp \
    settingini.cpp \
    dishwashersignalquerythread.cpp

HEADERS  += \
    serial_iii.h\
    httpclient.h \
    pdlog.h \
    pdhttpapi.h \
    deviceinfo.h \
    dtucfgthread.h\    
    mainwindow.h \
    pdserial.h \
    atcommandsendthread.h \
    iqctestthread.h \
    signalstrengthtestthread.h \
    settingini.h \
    dishwashersignalquerythread.h

FORMS    += dtuconfigtool.ui \
    mainwindow.ui
DISTFILES += \
    lib/libeay32.dll \
    lib/ssleay32.dll \
    icon.rc \
    image/logo.ico

CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug

DLLFiles.path = $${DESTDIR}/
DLLFiles.files = $$files(lib/*)

INSTALLS += DLLFiles
