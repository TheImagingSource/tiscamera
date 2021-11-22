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

#include <mutex>
#include <QThread>
#include <QEvent>
#include <QCoreApplication>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
template <typename F>
static void postToObject(QObject* obj,F && fun) {
   QMetaObject::invokeMethod( obj, std::forward<F>(fun), Qt::QueuedConnection );
}

#else
namespace
{
template<typename F> struct FEvent : public QEvent
{
    using Fun = typename std::decay<F>::type;
    Fun fun_;
    FEvent(Fun&& fun) : QEvent(QEvent::None), fun_(std::move(fun)) {}
    FEvent(const Fun& fun) : QEvent(QEvent::None), fun_(fun) {}
    ~FEvent()
    {
        fun_();
    }
};
} // namespace

template<typename F> static void postToObject(QObject* obj,F&& fun)
{
    if (qobject_cast<QThread*>(obj))
        qWarning( "posting a call to a thread object - consider using postToThread" );
    QCoreApplication::postEvent(obj, new ::FEvent<F>(std::forward<F>(fun)));
}
#endif


static Device to_device(GstDevice* device)
{
    GstStructure* struc = gst_device_get_properties(device);
    gchar* display_name = gst_device_get_display_name(device);
    g_free(display_name);
    if (!struc)
    {
        qWarning("No properties to handle.\n");
        return Device();
    }

    GstCaps* caps = gst_device_get_caps(device);

    std::string name;
    const char* name_str = gst_structure_get_string(struc, "model");

    if (name_str)
    {
        name = name_str;
    }

    std::string serial;
    const char* serial_str = gst_structure_get_string(struc, "serial");

    if (serial_str)
    {
        serial = serial_str;
    }

    std::string type;
    const char* type_str = gst_structure_get_string(struc, "type");

    if (type_str)
    {
        type = type_str;
    }
    else
    {
        type = "unknown";
    }

    Device ret(name, serial, type, caps);

    gst_structure_free(struc);
    gst_caps_unref(caps);
    return ret;
}


gboolean Indexer::bus_function(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    GstDevice* device = nullptr;
    Indexer* self = static_cast<Indexer*>(user_data);

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
        {
            gst_message_parse_device_added(message, &device);

            self->add_device(device);

            gst_object_unref(device);
            break;
        }
        case GST_MESSAGE_DEVICE_REMOVED:
        {
            gst_message_parse_device_removed(message, &device);

            self->remove_device(device);

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

    GList* devices = gst_device_monitor_get_devices(p_monitor);
    for (GList* entry = devices; entry != nullptr; entry = entry->next)
    {
        GstDevice* dev = (GstDevice*)entry->data;

        add_device(dev);
    }
    g_list_free_full(devices, gst_object_unref);
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

void Indexer::add_device(GstDevice* new_device)
{
    auto dev = to_device(new_device);

    bool is_new_device = false;
    {
        std::lock_guard lck { m_mutex };
        is_new_device = std::none_of(m_device_list.begin(),
                                     m_device_list.end(),
                                     [&dev](const Device& vec_dev) { return vec_dev == dev; });
        if (is_new_device)
        {
            m_device_list.push_back(dev);
        }
    }
    if (is_new_device) // moved out of scope of lock
    {
        qInfo("New device: %s", dev.serial_long().c_str());

        postToObject( this, [this,dev]{ emit this->new_device( dev ); } );
    }
}

void Indexer::remove_device(GstDevice* device)
{
    Device dev = to_device(device);

    m_mutex.lock();

    m_device_list.erase(std::remove_if(m_device_list.begin(),
                                       m_device_list.end(),
                                       [&dev](const Device& d) { return dev == d; }),
                        m_device_list.end());

    m_mutex.unlock();

    qInfo("Device lost: %s", dev.serial_long().c_str());

    postToObject( this, [this,dev]{ emit this->device_lost( dev ); } );
}