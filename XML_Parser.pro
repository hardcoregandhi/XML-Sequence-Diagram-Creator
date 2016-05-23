#-------------------------------------------------
#
# Project created by QtCreator 2016-04-25T11:03:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++0x

TARGET = XML_Parser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tinyxml2.cpp

HEADERS  += mainwindow.h \
    tinyxml2.h

FORMS    += mainwindow.ui

DISTFILES +=

#DEFINES += QT_NO_DEBUG_OUTPUT

