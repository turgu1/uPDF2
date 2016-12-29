#-------------------------------------------------
#
# Project created by QtCreator 2016-12-16T23:40:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uPDF2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/config.cpp \
    src/loadpdffile.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/pdffile.cpp \
    src/pdfloader.cpp \
    src/pdfpageworker.cpp \
    src/pdfviewer.cpp \
    src/selectrecentdialog.cpp \
    src/utils.cpp

HEADERS  += \
    src/config.h \
    src/loadpdffile.h \
    src/mainwindow.h \
    src/pdffile.h \
    src/pdfloader.h \
    src/pdfpageworker.h \
    src/pdfviewer.h \
    src/selectrecentdialog.h \
    src/updf.h \
    src/utils.h

FORMS    += \
    ui/mainwindow.ui \
    ui/selectrecentdialog.ui

RESOURCES += \
    res/updf.qrc

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/poppler
INCLUDEPATH += /usr/include/poppler

LIBS += -L/usr/local/lib

QMAKE_CXXFLAGS += -isystem -Wall -Wextra -Wno-unused-parameter
unix|win32: LIBS += -lpoppler -llzo2

QT          += widgets uiplugin
CONFIG      += plugin
#TEMPLATE    = lib
