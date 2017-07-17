#-------------------------------------------------
#
# Project created by QtCreator 2017-07-17T15:00:50
#
#-------------------------------------------------

QT       += core gui charts serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEmission
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    comchatter.cpp \
    logger.cpp

HEADERS += mainwindow.h \
    comchatter.h \
    logger.h

FORMS   += mainwindow.ui
