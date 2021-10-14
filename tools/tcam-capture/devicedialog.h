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

#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include "device.h"
#include "indexer.h"

#include <QDialog>
#include <QListWidgetItem>
#include <QMutex>

#include <memory>

#include "definitions.h"

namespace Ui
{
class DeviceDialog;
}

class DeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceDialog(std::shared_ptr<Indexer> index, QWidget* parent = nullptr);
    ~DeviceDialog();

    Device get_selected_device() const;
    // GstCaps* get_selected_caps() const;
    void set_format_handling(FormatHandling);
    FormatHandling get_format_handling() const;

public slots:
    void new_device(const Device& new_device);
    void lost_device(const Device& new_device);
    void update_device_listing(const std::vector<Device>& dev_list);

private slots:
    void on_buttonBox_accepted();

    void on_listWidget_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);

    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    void fill_list();

    Ui::DeviceDialog* ui;

    bool m_device_is_selected = false;
    Device m_selected_device;

    std::vector<Device> m_device_list;

    std::shared_ptr<Indexer> _index;
    QMutex m_mutex;
};

#endif // DEVICEDIALOG_H
