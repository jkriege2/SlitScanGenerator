#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T19:43:01
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11 stl exceptions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SlitScanGenerator
TEMPLATE = app

QMAKE_TARGET_PRODUCT="SlitScanGenerator"
QMAKE_TARGET_COPYRIGHT="(c) 2016 by Jan W. Krieger"
win32:RC_ICONS += ./icons/logo_64x64.ico
QMAKE_TARGET_COMPANY="https://github.com/jkriege2/SlitScanGenerator"

SOURCES += main.cpp\
        mainwindow.cpp \
    cimg_tools.cpp \
    ffmpeg_tools.cpp \
    imageviewer.cpp \
    processingparametertable.cpp \
    aboutbox.cpp \
    importdialog.cpp

HEADERS  += mainwindow.h \
            ../CImg/CImg.h \
    cimg_tools.h \
    ffmpeg_tools.h \
    imageviewer.h \
    processingparametertable.h \
    aboutbox.h \
    importdialog.h

FORMS    += mainwindow.ui \
    aboutbox.ui \
    importdialog.ui

INCLUDEPATH += ../CImg \
               ../ffmpeg/include

DEFINES +=

LIBS += -lm -L../ffmpeg/lib -lavutil -lavcodec -lavdevice -lswscale -lavformat -lswresample -lpostproc -lavfilter

MAKE_CXXFLAGS_RELEASE += -O3 -fopenmp -msse2 -msse -fpmath=both

RESOURCES += \
    slitscangenerator.qrc
