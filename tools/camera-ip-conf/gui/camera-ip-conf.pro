#
# Copyright 2013 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License") ;
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = camera-ip-conf
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x -O2

CONFIG += link_pkgconfig
PKGCONFIG += libzip tinyxml

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
