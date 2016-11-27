#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T19:43:01
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SlitScanGenerator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
            ../CImg/CImg.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../CImg \
               ../ffmpeg/include

DEFINES +=

LIBS += -lm -L../ffmpeg/lib -lavutil -lavcodec -lavdevice -lswscale -lavformat -lswresample -lpostproc -lavfilter
