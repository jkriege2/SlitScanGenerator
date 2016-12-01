#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T19:43:01
#
#-------------------------------------------------

QT       += core gui multimedia
CONFIG += c++11 stl exceptions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SlitScanGenerator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cimg_tools.cpp \
    ffmpeg_tools.cpp \
    imageviewer.cpp \
    processingparametertable.cpp

HEADERS  += mainwindow.h \
            ../CImg/CImg.h \
    cimg_tools.h \
    ffmpeg_tools.h \
    imageviewer.h \
    processingparametertable.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../CImg \
               ../ffmpeg/include

DEFINES +=

LIBS += -lm -L../ffmpeg/lib -lavutil -lavcodec -lavdevice -lswscale -lavformat -lswresample -lpostproc -lavfilter

MAKE_CXXFLAGS_RELEASE += -O3 -fopenmp -msse2 -msse -fpmath=both
