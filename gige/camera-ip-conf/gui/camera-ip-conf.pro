#-------------------------------------------------
#
# Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = camera-ip-conf
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x -O2

CONFIG(release, debug|release) {
    DESTDIR = ".."
}

SOURCES +=\
    gui_main.cpp \
    ../CameraDiscovery.cpp \
    ../NetworkInterface.cpp \
    ../Socket.cpp \
    ../Camera.cpp \
    ../FirmwareUpgrade.cpp \
    ../utils.cpp \
    UpdateHandler.cpp \
    InfoBox.cpp \
    MainWindow.cpp

HEADERS  += \
    ../CameraDiscovery.h \
    ../NetworkInterface.h \
    ../Socket.h \
    ../gigevision.h \
    ../Camera.h \
    ../FirmwareUpgrade.h \
    ../Firmware.h \
    ../utils.h \
    UpdateHandler.h \
    InfoBox.h \
    MainWindow.h

FORMS    += mainwindow.ui \
    infobox.ui

unix|win32: LIBS += -ltinyxml -lzip
