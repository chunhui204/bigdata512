#-------------------------------------------------
#
# Project created by QtCreator 2018-07-18T18:19:31
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia printsupport

TARGET = ALL
TEMPLATE = app


SOURCES += main.cpp\
    mainwidget.cpp \
    qcustomplot.cpp \
    audiosetting.cpp \
    audiowidget.cpp \
    audiodatathread.cpp \
    audioplotthread.cpp \
    wavfile.cpp

HEADERS += mainwidget.h \
    qcustomplot.h \
    audiosetting.h \
    audiowidget.h \
    audiodatathread.h \
    common.h \
    audioplotthread.h \
    wavfile.h

FORMS += mainwidget.ui \
    audiosetting.ui \
    audiowidget.ui

CONFIG += C++11


