/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "devicedialog.h"

#include "devicewidget.h"

#include "ui_devicedialog.h"

DeviceDialog::DeviceDialog(std::shared_ptr<Indexer> index, QWidget* parent)
    : QDialog(parent), ui(new Ui::DeviceDialog), _index(index)
{
    ui->setupUi(this);

    // prevent dialog maximization 
    setWindowFlags(windowFlags() | Qt::Tool);

    QObject::connect(_index.get(), &Indexer::new_list, this, &DeviceDialog::update_device_listing);
    QObject::connect(_index.get(), &Indexer::new_device, this, &DeviceDialog::new_device);
    QObject::connect(_index.get(), &Indexer::device_lost, this, &DeviceDialog::lost_device);

    m_device_list = _index->get_device_list();

    fill_list();
}

DeviceDialog::~DeviceDialog()
{
    delete ui;
}

void DeviceDialog::fill_list()
{
    auto selection = this->ui->listWidget->currentItem();
    Device selected_dev;
    bool have_device = false;
    if (selection)
    {
        have_device = true;
        selected_dev = ((const DeviceWidget*)selection)->get_device();
    }

    this->ui->listWidget->blockSignals(true);
    this->ui->listWidget->clear();

    for (const auto& dev : m_device_list)
    {
        auto w = new DeviceWidget(dev);
        this->ui->listWidget->addItem(w);
        if (have_device && dev == selected_dev)
        {
            w->setSelected(true);
        }
    }

    this->ui->listWidget->update();
    this->ui->listWidget->blockSignals(false);
}

Device DeviceDialog::get_selected_device() const
{
    return m_selected_device;
}


void DeviceDialog::set_format_handling(FormatHandling handling)
{
    ui->buttonBox->blockSignals(true);
    if (handling == FormatHandling::Auto)
    {
        ui->button_auto->setChecked(true);
    }
    else
    {
        ui->button_dialog->setChecked(true);
    }
    ui->buttonBox->blockSignals(false);
}


FormatHandling DeviceDialog::get_format_handling() const
{
    if (ui->button_auto->isChecked())
    {
        return FormatHandling::Auto;
    }
    return FormatHandling::Dialog;
}

void DeviceDialog::on_buttonBox_accepted()
{
    hide();
}

void DeviceDialog::new_device(const Device& new_device)
{
    auto w = new DeviceWidget(new_device);
    this->ui->listWidget->blockSignals(true);
    this->ui->listWidget->addItem(w);
    this->ui->listWidget->blockSignals(false);
}

void DeviceDialog::lost_device(const Device& lost_device)
{
    for (int i = 0; i < this->ui->listWidget->count(); ++i)
    {
        auto itemf = static_cast<DeviceWidget*>(this->ui->listWidget->item(i));
        if (*itemf == lost_device)
        {
            QListWidgetItem* item = this->ui->listWidget->takeItem(i);

            delete item;
            break;
        }
    }
}

void DeviceDialog::update_device_listing(const std::vector<Device>& /*dev_list*/)
{}

void DeviceDialog::on_listWidget_currentItemChanged(QListWidgetItem* current,
                                                    QListWidgetItem* /*previous*/)
{
    if (current)
    {
        m_selected_device = ((DeviceWidget*)current)->get_device();
    }

}

void DeviceDialog::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    if (item)
    {
        m_selected_device = ((DeviceWidget*)item)->get_device();
    }
    this->accept();
}

void DeviceDialog::on_listWidget_itemClicked(QListWidgetItem* /*item*/)
{
    m_device_is_selected = false;

}
