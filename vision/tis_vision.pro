#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T13:58:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tis_vision
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
           mainwindow.cpp \
           capturedeviceselectiondialog.cpp \
           propertywidget.cpp \
           videowidget.cpp

HEADERS  += mainwindow.h \
            capturedeviceselectiondialog.h \
            propertywidget.h \
            videowidget.h

FORMS    += mainwindow.ui \
            capturedeviceselectiondialog.ui \
            propertywidget.ui \
    videowidget.ui

unix:!macx: LIBS += -L/home/edt/lib -ltis_imaging

INCLUDEPATH += $$PWD/../../
INCLUDEPATH += /home/edt/include
INCLUDEPATH += /home/edt/work/tiscamera/src

DEPENDPATH += $$PWD/../../

RESOURCES +=
