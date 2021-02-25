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

#include "gsttcamdeviceprovider.h"

#include "gsttcamdevice.h"

#include "tcamgstbase.h"

#include <algorithm>

GST_DEBUG_CATEGORY_STATIC(tcam_deviceprovider_debug);
#define GST_CAT_DEFAULT tcam_deviceprovider_debug


G_DEFINE_TYPE(TcamDeviceProvider, tcam_device_provider, GST_TYPE_DEVICE_PROVIDER)


struct device
{
    tcam::DeviceInfo device;
    GstDevice* gstdev;
};


struct provider_state
{
    tcam::DeviceIndex index;
    std::vector<device> known_devices;

    std::condition_variable cv;
    std::mutex _mtx;
    std::atomic<bool> run_updates;
    std::thread _update_thread;
};


static GstDevice* tcam_device_new(GstElementFactory* factory,
                                  const tcam::DeviceInfo& device)
{
    GST_INFO("Creating new device");

    auto dev = tcam::open_device(device.get_serial(), device.get_device_type());

    std::vector<tcam::VideoFormatDescription> format = dev->get_available_video_formats();

    GstCaps* caps = convert_videoformatsdescription_to_caps(format);

    std::string serial = device.get_serial() + "-" + device.get_device_type_as_string();
    std::string display_string = "tcam-" + device.get_name_safe() + "-" + serial;

    GstDevice* ret = GST_DEVICE(g_object_new(TCAM_TYPE_DEVICE,
                                             "serial",
                                             serial.c_str(),
                                             "display_name",
                                             display_string.c_str(),
                                             "device-class",
                                             "Source/Video/Device/tcam",
                                             "caps",
                                             caps,
                                             NULL));

    gst_caps_unref(caps);

    TCAM_DEVICE(ret)->serial = g_strdup(serial.c_str());
    TCAM_DEVICE(ret)->factory = GST_ELEMENT_FACTORY(gst_object_ref(factory));

    return ret;
}


static void update_device_list(TcamDeviceProvider* self)
{

    auto check_lost_devices = [self](const std::vector<tcam::DeviceInfo>& new_list) {
        std::vector<device> to_remove;
        for (const auto& d : self->state->known_devices)
        {
            auto iter = std::find_if(new_list.begin(), new_list.end(), [&d](const auto& new_d)
            {
                if (new_d.get_serial() == d.device.get_serial()
                    && new_d.get_device_type() == d.device.get_device_type())
                {
                    return true;
                }
                return false;
            });

            if (iter == new_list.end())
            {
                GST_DEBUG("Notifying about lost device...");
                gst_device_provider_device_remove(GST_DEVICE_PROVIDER(self), d.gstdev);
                g_object_unref(d.gstdev);
                to_remove.push_back(d);
            }
        }

        for (const auto& d : to_remove)
        {
            self->state->known_devices.erase(std::remove_if(self->state->known_devices.begin(),
                                                            self->state->known_devices.end(),
                                                            [d](const struct device& dev) {
                                                                if (dev.device == d.device)
                                                                {
                                                                    return true;
                                                                }
                                                                return false;
                                                            }),
                                             self->state->known_devices.end());
        }
    };

    auto check_new_devices = [self](const std::vector<tcam::DeviceInfo>& new_list)
    {
        std::vector<device> new_devices;

        for (const auto& d : new_list)
        {
            auto iter =
                std::find_if(self->state->known_devices.begin(),
                             self->state->known_devices.end(),
                             [&d](const auto& new_d) {
                                 if (new_d.device.get_serial() == d.get_serial()
                                     && new_d.device.get_device_type() == d.get_device_type())
                                 {
                                     return true;
                                 }
                                 return false;
                             });

            if (iter == self->state->known_devices.end())
            {
                std::string name = d.get_serial() + "-" + d.get_device_type_as_string();

                auto new_gstdev = tcam_device_new(self->factory, d);
                GST_DEBUG("Notifying about new device...");
                gst_device_provider_device_add(GST_DEVICE_PROVIDER(self), new_gstdev);
                struct device new_dev = { d, new_gstdev };
                new_devices.push_back(new_dev);
            }
        }

        self->state->known_devices.insert(self->state->known_devices.end(),
                                          new_devices.begin(),
                                          new_devices.end());
    };

    std::unique_lock lck(self->state->_mtx);
    auto sec = std::chrono::seconds(1);
    while (self->state->run_updates)
    {
        GST_DEBUG("Checking for new devices");

        auto new_list = self->state->index.get_device_list();

        check_lost_devices(new_list);
        check_new_devices(new_list);

        self->state->cv.wait_for(lck, 2 * sec);
    }
    GST_DEBUG("update thread stopping....");

}


static void tcam_device_provider_init(TcamDeviceProvider* self)
{
    self->factory = gst_element_factory_find("tcamsrc");

    /* Ensure we can introspect the factory */
    gst_object_unref(gst_plugin_feature_load(GST_PLUGIN_FEATURE(self->factory)));
    self->state = new provider_state();
}

static GList* tcam_device_provider_probe(GstDeviceProvider* provider)
{
    GST_INFO("received probe");

    TcamDeviceProvider* self = TCAM_DEVICE_PROVIDER(provider);

    std::lock_guard(self->state->_mtx);

    GList* ret = NULL;

    for (const auto& d : self->state->known_devices)
    {
        std::string long_serial =
            d.device.get_serial() + "-" + d.device.get_device_type_as_string();
        GST_DEBUG("Appending: %s", long_serial.c_str());

        ret = g_list_append(ret,
                            tcam_device_new(self->factory,
                                            d.device));
    }

    return ret;
}

static gboolean tcam_device_provider_start(GstDeviceProvider* provider)
{
    GST_DEBUG("start");

    TcamDeviceProvider* self = TCAM_DEVICE_PROVIDER(provider);

    self->state->run_updates = true;
    self->state->_update_thread = std::thread(update_device_list, self);

    return TRUE;
}

static void tcam_device_provider_stop(GstDeviceProvider* provider)
{
    GST_DEBUG("stop");

    TcamDeviceProvider* self = TCAM_DEVICE_PROVIDER(provider);

    self->state->run_updates = false;
    self->state->cv.notify_all();

    self->state->_update_thread.join();
}

static void tcam_device_provider_dispose(GObject* object)
{
    TcamDeviceProvider* self = TCAM_DEVICE_PROVIDER(object);

    gst_object_replace((GstObject**)&self->factory, NULL);

    G_OBJECT_CLASS(tcam_device_provider_parent_class)->dispose(object);
}

static void tcam_device_provider_finalize(GObject* object)
{
    TcamDeviceProvider* self = TCAM_DEVICE_PROVIDER(object);

    if (self->state)
    {
        delete self->state;
        self->state = nullptr;
    }
    G_OBJECT_CLASS(tcam_device_provider_parent_class)->finalize(object);
}


static void tcam_device_provider_class_init(TcamDeviceProviderClass* klass)
{
    GstDeviceProviderClass* dm_class = GST_DEVICE_PROVIDER_CLASS(klass);
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = tcam_device_provider_dispose;
    gobject_class->finalize = tcam_device_provider_finalize;

    dm_class->probe = tcam_device_provider_probe;
    dm_class->start = tcam_device_provider_start;
    dm_class->stop = tcam_device_provider_stop;

    gst_device_provider_class_set_static_metadata(
        dm_class,
        "TCam Device Provider",
        "Source/Video/tcam",
        "Lists and provides tcam source devices",
        "The Imaging Source <support@theimagingsource.com>");
    GST_DEBUG_CATEGORY_INIT(
        tcam_deviceprovider_debug, "tcamdeviceprovider", 0, "tcam device provider");
}
