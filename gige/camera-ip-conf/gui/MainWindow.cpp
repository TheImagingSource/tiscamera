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

#include "MainWindow.h"
#include "ui_mainwindow.h"

#include "../Camera.h"
#include "../utils.h"

#include <memory>
#include <thread>
#include <future>

#include <QThread>
#include <QFileDialog>
#include <QTreeView>
#include <QApplication>


using namespace tis;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0); // assure we always start with overview tab

    // assure camera list is sorted after the serial numbers in alphabetical order
    ui->cameraList->sortByColumn(0,Qt::AscendingOrder);
    ui->cameraList->setSortingEnabled(true);

    // disable dropping of leaves as top level items
    ui->cameraList->invisibleRootItem()->setFlags( Qt::ItemIsSelectable |
                                                   Qt::ItemIsUserCheckable |
                                                   Qt::ItemIsEnabled );

    // allow passing of camera_list between threads
    qRegisterMetaType<camera_list> ("camera_list");
    qRegisterMetaType<std::shared_ptr<Camera>> ("std::shared_ptr<Camera>");

    // connect thread signal to out update method
    connect(&uh, SIGNAL(update(camera_list)), this, SLOT(updateCycle(camera_list)));
    connect(&uh, SIGNAL(deprecated(std::shared_ptr<Camera>)), this, SLOT(deprecatedCamera(std::shared_ptr<Camera>)));

    uh.start();

    // hide progressbar until it is needed
    ui->progressFirmwareUpload->setHidden(true);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::displayCamera()
{
    QList<QTreeWidgetItem*> selection = ui->cameraList->selectedItems();

    QTreeWidgetItem* item;
    if (!selection.empty())
    {
        item = selection.at(0);
    }
    else
    {
        return;
    }

    auto cam = getCameraFromList(cameras, item->text(coloumn.serial).toStdString());
    if (cam == NULL)
    {
        return;
    }

    selectedCamera = cam;
    blockUpload = false;

    // overview tab
    ui->displayModelName->setText(QString(cam->getModelName().c_str()));
    ui->displaySerialNumber->setText(QString(cam->getSerialNumber().c_str()));
    ui->displayVendor->setText(QString(cam->getVendorName().c_str()));

    ui->displayCurrentIP->setText(QString(cam->getCurrentIP().c_str()));
    ui->displayCurrentNetmask->setText(QString(cam->getCurrentSubnet().c_str()));
    ui->displayCurrentGateway->setText(QString(cam->getCurrentGateway().c_str()));
    ui->displayMAC->setText(QString(cam->getMAC().c_str()));

    // set name tab
    ui->editName->setText(QString(cam->getUserDefinedName().c_str()));

    // settings tab
    ui->editSettingsIP->setText(QString(cam->getPersistentIP().c_str()));
    ui->editSettingsNetmask->setText(QString(cam->getPersistentSubnet().c_str()));
    ui->editSettingsGateway->setText(QString(cam->getPersistentGateway().c_str()));


    if (cam->isDHCPactive())
    {
        ui->checkBoxDHCP->setChecked(true);
    }
    else
    {
        ui->checkBoxDHCP->setChecked(false);
    }

    if (cam->isStaticIPactive())
    {
        ui->checkBoxStatic->setChecked(true);
        // enable persistent config fields
        ui->editSettingsIP->setEnabled(true);
        ui->editSettingsNetmask->setEnabled(true);
        ui->editSettingsGateway->setEnabled(true);
    }
    else
    {
        ui->checkBoxStatic->setChecked(false);
        // disable persistent config fields
        ui->editSettingsIP->setEnabled(false);
        ui->editSettingsNetmask->setEnabled(false);
        ui->editSettingsGateway->setEnabled(false);
    }
     // firmware tab
    ui->labelVersion->setText(cam->getFirmwareVersion().c_str());
}


void MainWindow::addNetworkInterfacesToCameraList ()
{
    auto interfaces = detectNetworkInterfaces();

    for (const auto& interface : interfaces)
    {
        if ((ui->cameraList->findItems(QString(interface->getInterfaceName().c_str()), Qt::MatchContains).empty()))
        {
            QTreeWidgetItem* ifa = new QTreeWidgetItem();
            ifa->setText(0, QString(interface->getInterfaceName().c_str()));
            ifa->setFlags(Qt::ItemIsEnabled);
            ifa->setDisabled(false);
            ifa->sortChildren(0,Qt::AscendingOrder);

            ifa->setExpanded(true);

            ui->cameraList->addTopLevelItem(ifa);
        }
    }
}


void MainWindow::addCameraToCameraList (const std::shared_ptr<Camera> cam)
{
    QList<QTreeWidgetItem*> interface = ui->cameraList->findItems(QString(cam->getInterfaceName().c_str()), Qt::MatchContains);

    if (!interface.empty())
    {
        QTreeWidgetItem* new_cam = new QTreeWidgetItem();

        QString name = cam->getModelName().c_str();

        new_cam->setText(coloumn.model, name);
        new_cam->setText(coloumn.serial, QString(cam->getSerialNumber().c_str()));
        new_cam->setText(coloumn.name, QString(cam->getUserDefinedName().c_str()));
        new_cam->setText(coloumn.ip, QString(cam->getCurrentIP().c_str()));
        new_cam->setText(coloumn.netmask, QString(cam->getCurrentSubnet().c_str()));
        new_cam->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable
                          | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
        new_cam->setDisabled(false);
        interface.at(0)->addChild(new_cam);
    }
}


void MainWindow::updateCamera (QTreeWidgetItem& item, const std::shared_ptr<Camera> camera)
{
    item.setText(coloumn.name,   QString(camera->getUserDefinedName().c_str()));
    item.setText(coloumn.ip,     QString(camera->getCurrentIP().c_str()));
    item.setText(coloumn.netmask,QString(camera->getCurrentSubnet().c_str()));

    // if nothing is selected we are finished
    if (selectedCamera == NULL)
    {
        return;
    }

    if (camera->getSerialNumber().compare(selectedCamera->getSerialNumber()) == 0)
    {

        ui->displayModelName->setText(QString(camera->getModelName().c_str()));
        ui->displaySerialNumber->setText(QString(camera->getSerialNumber().c_str()));
        ui->displayVendor->setText(QString(camera->getVendorName().c_str()));

        ui->displayCurrentIP->setText(QString(camera->getCurrentIP().c_str()));
        ui->displayCurrentNetmask->setText(QString(camera->getCurrentSubnet().c_str()));
        ui->displayCurrentGateway->setText(QString(camera->getCurrentGateway().c_str()));
        ui->displayMAC->setText(QString(camera->getMAC().c_str()));

        // firmware tab
        ui->labelVersion->setText(camera->getFirmwareVersion().c_str());
    }
}


void MainWindow::updateCameraList ()
{
    QList<QTreeWidgetItem*> selection = ui->cameraList->selectedItems();

    QString s;
    if (!selection.empty())
    {
        s = selection.at(0)->text(0);
    }

    addNetworkInterfacesToCameraList();
    for (const auto& cam : cameras)
    {
        QList<QTreeWidgetItem*> list = ui->cameraList->findItems(
                    QString(cam->getSerialNumber().c_str()),Qt::MatchRecursive | Qt::MatchContains, coloumn.serial);
        if (list.size() != 1)
        {
            addCameraToCameraList(cam);
        }
        else
        {
            updateCamera(*list.at(0), cam);
        }
    }
}


void MainWindow::changeToSettings()
{
    int index = 1;

    for (int x = 0; x < ui->tabWidget->count(); ++x)
    {
        const QWidget* q = ui->tabWidget->widget(x);
        if (q->accessibleName().compare(QString("Settings")) == 0)
        {
            index = x;
            break;
        }
    }

    ui->tabWidget->setCurrentIndex(index);
}


void MainWindow::sendForceIP()
{
    std::string forceIP = ui->editForceIP->text().toStdString();
    std::string forceGateway = ui->editForceGateway->text().toStdString();
    std::string forceNetmask = ui->editForceNetmask->text().toStdString();

    std::string errorMessage;

    // check if input is valid
    if (!isValidIpAddress(forceIP))
    {
        errorMessage.append("Not a valid IP\n");
    }

    if (!isValidIpAddress(forceGateway))
    {
        errorMessage.append("Not a valid netmask\n");
    }
    if (!isValidIpAddress(forceNetmask))
    {
        errorMessage.append("Not a valid gateway");
    }

    if (!errorMessage.empty())
    {
        QString msg = errorMessage.c_str();
        box.showMessage(msg);
        return;
    }

    selectedCamera->forceIP(forceIP, forceNetmask, forceGateway);
}


void MainWindow::saveName()
{
    std::string name = ui->editName->text().toStdString();

    if (selectedCamera != NULL)
    {
        selectedCamera->setUserDefinedName(name);
    }

    ui->editName->setText("");
}


void MainWindow::saveSettings ()
{
    if (selectedCamera == NULL)
    {
        return;
    }

    // retrieve Settings

    bool activateDHCP = ui->checkBoxDHCP->isChecked();
    bool activateStatic = ui->checkBoxStatic->isChecked();

    std::string ip = ui->editSettingsIP->text().toStdString();
    std::string netmask = ui->editSettingsNetmask->text().toStdString();
    std::string gateway = ui->editSettingsGateway->text().toStdString();

    std::string errorMessage;

    // check if input is valid
    if (!isValidIpAddress(ip))
    {
        errorMessage.append("Not a valid IP\n");
    }

    if (!isValidIpAddress(netmask))
    {
        errorMessage.append("Not a valid netmask\n");
    }
    if (!isValidIpAddress(gateway))
    {
        errorMessage.append("Not a valid gateway");
    }

    if (!errorMessage.empty())
    {
        QString msg = errorMessage.c_str();
        box.showMessage(msg);
        return;
    }

    if (!selectedCamera->setPersistentIP(ip))
    {
        QString s = "Error setting Persistent IP.";
        box.showMessage(s);
        return;
    }
    if (!selectedCamera->setPersistentSubnet(netmask))
    {
        QString s = "Error setting Persistent Netmask.";
        box.showMessage(s);
        return;
    }
    if (!selectedCamera->setPersistentGateway(gateway))
    {
        QString s = "Error setting Persistent Gateway.";
        box.showMessage(s);
        return;
    }

    if (activateStatic)
    {
        selectedCamera->setStaticIPstate(true);
    }
    else
    {
        selectedCamera->setStaticIPstate(false);
    }

    if (activateDHCP)
    {
        selectedCamera->setDHCPstate(true);
    }
    else
    {
        selectedCamera->setDHCPstate(false);
    }
    selectedCamera->resetIP();
}


void MainWindow::allowPersistentConfig (bool allow)
{
    if (allow)
    {
        ui->editSettingsIP->setEnabled(true);
        ui->editSettingsNetmask->setEnabled(true);
        ui->editSettingsGateway->setEnabled(true);
    }
    else
    {
        ui->editSettingsIP->setEnabled(false);
        ui->editSettingsNetmask->setEnabled(false);
        ui->editSettingsGateway->setEnabled(false);
    }
}


void MainWindow::buttonFirmware_clicked ()
{
    // show file dialog and fill edit box with selected filepath
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Firmware File"),
                                                    QDir::homePath(),
                                                    tr("Firmware Files (*.fwpack *.fw)"));
    ui->editFirmwarePath->setText(fileName);
}


void MainWindow::uploadFirmware ()
{

    if (blockUpload)
    {
        return;
    }

    std::string firmware_file = ui->editFirmwarePath->text().toStdString();
    if (firmware_file.empty())
    {
        return;
    }
    ui->progressFirmwareUpload->setVisible(true);

    auto func = [this] (int val)
    {

        ui->progressFirmwareUpload->setValue(val);
    };

    if (selectedCamera->uploadFirmware(firmware_file, func))
    {
        blockUpload = true;
        QString s = "Successfully uploaded Firmware. Please reconnect your camera for full funcionality.";
        box.showMessage(s);
    }
    else
    {
        QString s = "Error while uploading Firmware. Please try again.";
        box.showMessage(s);
    }
    ui->editFirmwarePath->setText("");
    ui->progressFirmwareUpload->setVisible(false);
}


void MainWindow::updateCycle (camera_list c)
{
    this->cameras = c;
    updateCameraList();
}


void MainWindow::deprecatedCamera (std::shared_ptr<Camera> camera)
{
    QTreeWidgetItem* item;

    QList<QTreeWidgetItem*> l = ui->cameraList->selectedItems();
    if (l.size() > 1)
    {
        // something is wrong; don't make it worse
        return;
    }

    // the lost camera is the selected one, therefore we have to clean up everything
    if (l.size() == 1 && camera->getSerialNumber().compare(l.at(0)->text(coloumn.serial).toStdString()) == 0)
    {
        item = l.at(0);
        QString defaultText = "<Select Device>";
        QString emptyString = "";
        ui->tabWidget->setCurrentIndex(0);

        ui->displayModelName->setText(defaultText);
        ui->displaySerialNumber->setText(defaultText);
        ui->displayVendor->setText(defaultText);

        ui->displayCurrentIP->setText(defaultText);
        ui->displayCurrentNetmask->setText(defaultText);
        ui->displayCurrentGateway->setText(defaultText);
        ui->displayMAC->setText(defaultText);

        // set name tab
        ui->editName->setText(emptyString);

        // settings tab
        ui->editSettingsIP->setText(emptyString);
        ui->editSettingsNetmask->setText(emptyString);
        ui->editSettingsGateway->setText(emptyString);

        ui->checkBoxDHCP->setChecked(false);
        ui->checkBoxStatic->setChecked(false);
        // enable persistent config fields
        ui->editSettingsIP->setEnabled(true);
        ui->editSettingsNetmask->setEnabled(true);
        ui->editSettingsGateway->setEnabled(true);

        // firmware tab
        ui->labelVersion->setText(defaultText);

        blockUpload = false;
    }
    else
    {
        // Not the selected camera
        QList<QTreeWidgetItem*> list = ui->cameraList->findItems(
                    QString(camera->getSerialNumber().c_str()), Qt::MatchRecursive | Qt::MatchContains, coloumn.serial);
        if (list.size() == 1)
        {
            item = list.at(0);
        }
        else
        {
            return;
        }
    }

    // remove from camera tree
    QTreeWidgetItem* par = item->parent();
    par->removeChild(item);
}
