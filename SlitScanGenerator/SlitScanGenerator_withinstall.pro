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
RC_ICONS += ./icons/logo_64x64.ico
QMAKE_TARGET_COMPANY = "https://github.com/jkriege2/SlitScanGenerator"

TEMPNAME = $${QMAKE_QMAKE}
QTPATH = $$dirname(TEMPNAME)

INSTALLDIR=$$OUT_PWD

contains(QTPATH, .*msys.* ) {
  QTBINDIR=$$QTPATH
  QTPLUGINDIR=$$QTPATH/../share/qt5/plugins
} else {
  QTBINDIR=$$QTPATH
  QTPLUGINDIR=$$QTPATH/../plugins
}

win32-g++:contains(QMAKE_HOST.arch, x86_64):{
      message("Host is 64bit OUT_PWD='$$OUT_PWD'")
      FFMPEG_PATH = ../ffmpeg/win64
      INSTALLDIR=$$OUT_PWD/$${QMAKE_TARGET_PRODUCT}_win64
}else:{
      message("Host is 32bit OUT_PWD='$$OUT_PWD' QTDIR='$$QTDIR'")
      FFMPEG_PATH = ../ffmpeg/win32
      INSTALLDIR=$$OUT_PWD/$${QMAKE_TARGET_PRODUCT}_win64
}
message("-- QTPATH='$$QTPATH'")
message("-- QTDIR='$$QTDIR'")
message("-- QT_INSTALL_BINS='$$QT_INSTALL_BINS'")
message("-- QT_INSTALL_LIBS='$$QT_INSTALL_LIBS'")
message("-- QT_INSTALL_PLUGINS='$$QT_INSTALL_PLUGINS'")
message("-- QMAKE_QMAKE='$$QMAKE_QMAKE'")
message("-- QMAKE_LIBDIR='$$QMAKE_LIBDIR'")
message("-- QMAKE_PATH='$$QMAKE_PATH'")
message("-- QTBINDIR='$$QTBINDIR'")
message("-- QTPLUGINDIR='$$QTPLUGINDIR'")
message("-- INSTALLDIR='$$INSTALLDIR'")



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


FFMPEG_COPY_LIBS.files = $$FFMPEG_PATH/bin/*.*
FFMPEG_COPY_LIBS.path = $$INSTALLDIR

FFMPEG_COPY_DOC.files = $$FFMPEG_PATH/doc/*.*
FFMPEG_COPY_DOC.path = $$INSTALLDIR/ffmpeg/doc/

FFMPEG_COPY_LICENSE.files = $$FFMPEG_PATH/licenses/*.*
FFMPEG_COPY_LICENSE.path = $$INSTALLDIR/ffmpeg/licenses/

IMAGEPLUGINS_COPY.files = $$QTPLUGINDIR/imageformats/*.dll
IMAGEPLUGINS_COPY.path = $$INSTALLDIR/plugins/imageformats/
PLATFORMPLUGINS_COPY.files = $$QTPLUGINDIR/platforms/*.dll
PLATFORMPLUGINS_COPY.path = $$INSTALLDIR/plugins/platforms/

INSTALLS += FFMPEG_COPY_LIBS FFMPEG_COPY_DOC FFMPEG_COPY_LICENSE IMAGEPLUGINS_COPY PLATFORMPLUGINS_COPY
