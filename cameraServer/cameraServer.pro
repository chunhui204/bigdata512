#-------------------------------------------------
#
# Project created by QtCreator 2018-07-29T08:18:30
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cameraServer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    videodatathread.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    videodatathread.h

FORMS    += mainwindow.ui
CONFIG += C++11
