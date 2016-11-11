#-------------------------------------------------
#
# Project created by QtCreator 2016-08-31T16:44:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GData
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    nnmodel.cpp

HEADERS  += mainwindow.h \
    nnmodel.h

FORMS    += mainwindow.ui


# For CNTK
INCLUDEPATH += /home/plotnikov/cntk/Source/Common/
LIBS += -L /home/plotnikov/cntk/build/release/lib/ -leval -lcntkmath
