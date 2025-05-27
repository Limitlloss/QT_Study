#-------------------------------------------------
#
# Project created by QtCreator 2018-01-05T09:31:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
win32:msvc {
    QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
}
# QMAKE_CXXFLAGS += -input-charset=UTF-8
TARGET = 01_FirstProject
TEMPLATE = app


SOURCES += main.cpp\
        mywidget.cpp \
    mypushbutton.cpp

HEADERS  += mywidget.h \
    mypushbutton.h
