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

#ifndef INDEXER_H
#define INDEXER_H

#include "device.h"

#include <QMutex>
#include <QTimer>
#include <gst/gst.h>
#include <vector>

class Indexer : public QObject
{
    Q_OBJECT

public:
    Indexer();
    ~Indexer();

    std::vector<Device> get_device_list();

signals:

    void new_device(const Device&);
    void device_lost(const Device&);
    void new_list(const std::vector<Device>&);

private:
    static gboolean bus_function(GstBus* bus, GstMessage* message, gpointer user_data);

private:
    void add_device(GstDevice* new_device);
    void remove_device(GstDevice* device);

    std::vector<Device> m_device_list;

    QMutex m_mutex;

    GstDeviceMonitor* p_monitor = nullptr;
};

#endif // INDEXER_H
