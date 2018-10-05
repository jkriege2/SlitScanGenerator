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

QMAKE_TARGET_PRODUCT = "SlitScanGenerator"
QMAKE_TARGET_COPYRIGHT = "(c) 2016-2018 by Jan W. Krieger"
win32:RC_ICONS += ./icons/logo_64x64.ico
QMAKE_TARGET_COMPANY = "https://github.com/jkriege2/SlitScanGenerator"

FFMPEG_PATH = ../ffmpeg/win64

SOURCES += main.cpp\
        mainwindow.cpp \
    cimg_tools.cpp \
    ffmpeg_tools.cpp \
    imageviewer.cpp \
    processingparametertable.cpp \
    aboutbox.cpp \
    importdialog.cpp \
    processingwidget.cpp \
    processingthread.cpp \
    processingtask.cpp \
    taskswidget.cpp \
    geo_tools.cpp

HEADERS  += mainwindow.h \
            ../CImg/CImg.h \
    cimg_tools.h \
    ffmpeg_tools.h \
    imageviewer.h \
    processingparametertable.h \
    aboutbox.h \
    importdialog.h \
    processingwidget.h \
    processingthread.h \
    processingtask.h \
    taskswidget.h \
    geo_tools.h

FORMS    += mainwindow.ui \
    aboutbox.ui \
    importdialog.ui \
    processingwidget.ui

INCLUDEPATH += ../CImg \
               $$FFMPEG_PATH/include

DEFINES +=
CONFIG(debug, debug|release):DEFINES += DEBUG_FLAG

LIBS += -lm -L$$FFMPEG_PATH/lib -lavutil -lavcodec -lavdevice -lswscale -lavformat -lswresample -lpostproc -lavfilter

MAKE_CXXFLAGS_RELEASE += -O3 -fopenmp -msse2 -msse -fpmath=both

RESOURCES += \
    slitscangenerator.qrc
