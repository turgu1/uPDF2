#-------------------------------------------------
#
# Project created by QtCreator 2016-12-16T23:40:29
#
#-------------------------------------------------

QT       += core gui sql svg xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uPDF2
TEMPLATE = app

win32: RC_ICONS = updf.ico
macOs: ICON = updf.ico

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
    src/bookmarksbrowser.cpp \
    src/bookmarksdb.cpp \
    src/bookmarkselector.cpp \
    src/documenteditdialog.cpp \
    src/config.cpp \
    src/documentmapperdelegate.cpp \
    src/documentmodel.cpp \
    src/entrymapperdelegate.cpp \
    src/loadpdffile.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/newbookmarkdialog.cpp \
    src/pagenbrdelegate.cpp \
    src/pdffile.cpp \
    src/pdfloader.cpp \
    src/pdfpageworker.cpp \
    src/pdfviewer.cpp \
    src/selectrecentdialog.cpp \
    src/utils.cpp \
    src/preferencesdialog.cpp

HEADERS  += \
    src/bookmarksbrowser.h \
    src/bookmarksdb.h \
    src/bookmarkselector.h \
    src/documenteditdialog.h \
    src/config.h \
    src/documentmapperdelegate.h \
    src/documentmodel.h \
    src/entrymapperdelegate.h \
    src/loadpdffile.h \
    src/mainwindow.h \
    src/newbookmarkdialog.h \
    src/pagenbrdelegate.h \
    src/pdffile.h \
    src/pdfloader.h \
    src/pdfpageworker.h \
    src/pdfviewer.h \
    src/selectrecentdialog.h \
    src/updf.h \
    src/utils.h \
    src/preferencesdialog.h

FORMS    += \
    ui/bookmarksbrowser.ui \
    ui/bookmarkselector.ui \
    ui/documenteditdialog.ui \
    ui/mainwindow.ui \
    ui/newbookmarkdialog.ui \
    ui/selectrecentdialog.ui \
    ui/preferencesdialog.ui

RESOURCES += \
    res/updf.qrc

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/poppler
INCLUDEPATH += /usr/include/poppler

LIBS += -L/usr/local/lib

QMAKE_CXXFLAGS   += -isystem -Wall -Wextra -Wno-unused-parameter
unix|win32: LIBS += -lpoppler -llzo2

QT          += widgets
CONFIG      += plugin
#TEMPLATE    = lib
