#-------------------------------------------------
#
# Project created by QtCreator 2018-07-18T18:19:31
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia printsupport

TARGET = ALL
TEMPLATE = app


SOURCES += main.cpp\
    mainwidget.cpp \
    qcustomplot.cpp \
    audiosetting.cpp \
    networkthread.cpp \
    plotthread.cpp \
    audiowidget.cpp

HEADERS  += mainwidget.h \
    qcustomplot.h \
    audiosetting.h \
    networkthread.h \
    plotthread.h \
    audiowidget.h

FORMS += mainwidget.ui \
    audiosetting.ui \
    audiowidget.ui

CONFIG += C++11
