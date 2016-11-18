#-------------------------------------------------
#
# Project created by QtCreator 2014-11-25T14:38:46
#
#-------------------------------------------------

QT       += core gui multimedia serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = michMuseumInterface
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    museumscene.cpp

HEADERS  += mainwindow.h \
    museumscene.h

FORMS    += mainwindow.ui
