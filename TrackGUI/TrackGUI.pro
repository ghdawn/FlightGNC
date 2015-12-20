#-------------------------------------------------
#
# Project created by QtCreator 2015-12-20T17:23:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrackGUI
TEMPLATE = app

QT += network
SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    IOControl.h

FORMS    += mainwindow.ui

unix:!macx: LIBS += -L$$PWD/../../iTRLib/itrbase/bin/debug/ -litrbase -lavcodec -lswscale -lavutil

INCLUDEPATH += $$PWD/../../iTRLib/itrbase
DEPENDPATH += $$PWD/../../iTRLib/itrbase/bin/debug

unix:!macx: PRE_TARGETDEPS += $$PWD/../../iTRLib/itrbase/bin/debug/libitrbase.a

unix:!macx: LIBS += -L$$PWD/../../iTRLib/itrsystem/bin/debug/ -litrsystem

INCLUDEPATH += $$PWD/../../iTRLib/itrsystem
DEPENDPATH += $$PWD/../../iTRLib/itrsystem

unix:!macx: PRE_TARGETDEPS += $$PWD/../../iTRLib/itrsystem/bin/debug/libitrsystem.a
