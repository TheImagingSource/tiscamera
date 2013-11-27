/*
 * Copyright 2013 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QTreeView>
#include <QStandardItemModel>
#include <QApplication>
#include <QPushButton>
#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>


#include "../Camera.h"
#include "../CameraDiscovery.h"
#include "../NetworkInterface.h"
#include "UpdateHandler.h"
#include "InfoBox.h"


namespace Ui {
class MainWindow;
}

using namespace tis;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow (QWidget *parent = 0);
    ~MainWindow ();

public slots:

    /// @name displayCamera
    /// @brief updates tabView with information about selected Camera
    void displayCamera ();

    /// @name changeToSettings
    /// @brief changes selected tab from "Overview" to "Settings"
    void changeToSettings ();

    /// @name sendForceIP
    /// @brief reads input fields an sends forceip
    void sendForceIP ();

    /// @name saveName
    /// @brief stores given name in camera
    void saveName ();

    /// @name saveSettings
    /// @brief stores given settings in camera
    void saveSettings ();

    /// @name allowPersistentConfig
    /// @param allow - wether or not to allow editing of lines
    /// @brief enables ot disables the possibility to edit the persistent ip settings
    void allowPersistentConfig (bool allow);

    /// @name
    /// @brief opens file dialog to choose firmware
    void buttonFirmware_clicked ();

    /// @name uploadFirmware
    /// @brief transmits selected firmware to camera
    void uploadFirmware ();

    /// @name updateCycle
    /// @param c - updated camera_list displaying the current state
    /// @brief notification slot about completed update cycle
    void updateCycle (camera_list c);

    /// @name deprecatedCamera
    /// @param camera - Camera object hat shall be removed
    /// @brief removes given camera from all gui elements
    void deprecatedCamera (std::shared_ptr<Camera> camera);

private:
    /// @name adddCameraToCameraList
    /// @param camera - shared_ptr to the new camera
    /// @brief adds camera to the camera list under the according network interface
    void addCameraToCameraList (const std::shared_ptr<Camera> camera);

    /// @name addNetworkInterfacesToCameraList
    /// @brief adds all available network interfaces to the camera listing
    void addNetworkInterfacesToCameraList ();

    /// @name updateCamera
    /// @param item - TreeItem tat shall be updated
    /// @param camera - Camera from which information are taken
    /// @brief updates displayed information for given camera
    void updateCamera (QTreeWidgetItem& item, const std::shared_ptr<Camera> camera);
    /// @name  updateCameraList
    /// @brief updates the treewidget of displayed cameras
    void updateCameraList ();


    struct treeColoumn
    {
        static const int model = 0;
        static const int serial = 1;
        static const int name = 2;
        static const int ip = 3;
        static const int netmask = 4;
    } coloumn;

    // gui container class
    Ui::MainWindow *ui;

    // known cameras
    tis::camera_list cameras;
    std::shared_ptr<Camera> selectedCamera;

    UpdateHandler uh;

    InfoBox box;

    bool blockUpload;

};

#endif // MAINWINDOW_H
