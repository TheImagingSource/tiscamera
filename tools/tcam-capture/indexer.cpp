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

#include "indexer.h"

Device to_device(GstDevice* device)
{
    GstStructure* struc = gst_device_get_properties(device);
    gchar* display_name = gst_device_get_display_name(device);
    g_free(display_name);
    if (!struc)
    {
        qWarning("No properties to handle.\n");
        gst_object_unref(device);
        return Device();
    }

    GstCaps* caps = gst_device_get_caps(device);
    std::string name = gst_structure_get_string(struc, "model");
    std::string serial = gst_structure_get_string(struc, "serial");
    std::string type = gst_structure_get_string(struc, "type");
    Device ret(name, serial, type, caps);

    gst_structure_free(struc);
    gst_caps_unref(caps);
    return ret;
}


gboolean Indexer::bus_function(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    GstDevice* device;
    Indexer* self = static_cast<Indexer*>(user_data);

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
        {
            gst_message_parse_device_added(message, &device);
            auto dev = to_device(device);
            emit self->new_device(dev);
            self->m_mutex.lock();
            self->m_device_list.push_back(dev);
            self->m_mutex.unlock();
            gst_object_unref(device);
            break;
        }
        case GST_MESSAGE_DEVICE_REMOVED:
        {
            gst_message_parse_device_removed(message, &device);
            Device dev = to_device(device);

            self->m_mutex.lock();

            self->m_device_list.erase(std::remove_if(self->m_device_list.begin(),
                                                    self->m_device_list.end(),
                                                    [&dev](const Device& d) {
                                                        if (dev == d)
                                                        {
                                                            return true;
                                                        }
                                                        return false;
                                                    }),
                                     self->m_device_list.end());

            self->m_mutex.unlock();
            qInfo("Device lost: %s", dev.serial_long().c_str());
            emit self->device_lost(dev);
            gst_object_unref(device);
            break;
        }
        default:
        {
            break;
        }
    }

    return G_SOURCE_CONTINUE;
}


Indexer::Indexer()
{

    p_monitor = gst_device_monitor_new();

    GstBus* bus = gst_device_monitor_get_bus(p_monitor);
    gst_bus_add_watch(bus, &Indexer::bus_function, this);
    gst_object_unref(bus);

    gst_device_monitor_add_filter(p_monitor, "Video/Source/tcam", NULL);
    gst_device_monitor_start(p_monitor);
}

Indexer::~Indexer()
{
    gst_device_monitor_stop(p_monitor);

    gst_object_unref(p_monitor);
}

std::vector<Device> Indexer::get_device_list()
{
    QMutexLocker lock(&m_mutex);
    return m_device_list;
}

void Indexer::update()
{}
