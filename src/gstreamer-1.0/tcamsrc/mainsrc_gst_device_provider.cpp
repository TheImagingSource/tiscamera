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

#include "mainsrc_gst_device_provider.h"

#include "../../DeviceIndex.h"
#include "../../logging.h"
#include "../../utils.h"
#include "mainsrc_gst_device.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <gst-helper/gst_ptr.h>
#include <thread>

GST_DEBUG_CATEGORY_STATIC(tcam_deviceprovider_debug);
#define GST_CAT_DEFAULT tcam_deviceprovider_debug


G_DEFINE_TYPE(TcamMainSrcDeviceProvider, tcam_mainsrc_device_provider, GST_TYPE_DEVICE_PROVIDER)

namespace
{
static const char* provider_info = "Source/Video/Device/tcam";

struct device
{
    tcam::DeviceInfo device_info;
    gst_helper::gst_ptr<GstDevice> gstdev;

    bool operator==(const tcam::DeviceInfo& dev) const noexcept
    {
        return dev == device_info;
    }
};

} // namespace

namespace tcammainsrc
{
struct provider_state
{
    gst_helper::gst_ptr<GstElementFactory> factory_;

    tcam::DeviceIndex index_;
    std::vector<device> known_devices_;

    std::condition_variable cv_;
    std::mutex mtx_;
    std::atomic<bool> run_updates_;
    std::thread update_thread_;
};
} // namespace tcammainsrc

static GstDevice* tcam_mainsrc_device_new(GstElementFactory* factory,
                                          const tcam::DeviceInfo& device)
{
    GstCaps* caps = gst_caps_new_any();

    std::string serial = device.get_serial();
    std::string model = device.get_name();
    std::string type = device.get_device_type_as_string();

    std::string display_string = model + " (" + serial + "-" + type + ")";

    GstStructure* struc = gst_structure_new("tcam-device-properties",
                                            "serial",
                                            G_TYPE_STRING,
                                            serial.c_str(),
                                            "model",
                                            G_TYPE_STRING,
                                            model.c_str(),
                                            "type",
                                            G_TYPE_STRING,
                                            type.c_str(),
                                            nullptr);

    GstDevice* ret = GST_DEVICE(g_object_new(TCAM_TYPE_DEVICE,
                                             "display_name",
                                             display_string.c_str(),
                                             "device-class",
                                             provider_info,
                                             "caps",
                                             caps,
                                             "properties",
                                             struc,
                                             NULL));

    gst_caps_unref(caps);
    gst_structure_free(struc);

    TCAM_DEVICE(ret)->factory = GST_ELEMENT_FACTORY(gst_object_ref(factory));

    return ret;
}

static void run_update_logic(std::unique_lock<std::mutex>& /*lck*/,
                             TcamMainSrcDeviceProvider* self,
                             std::vector<tcam::DeviceInfo>&& new_list)
{
    auto& known_devices = self->state->known_devices_;

    // sort the new known_devices list to [still-present,not-present] with removed_devices_begin as the pivot
    // Note: We use stable-partition here to prevent elements from moving
    auto removed_devices_begin = std::stable_partition(
        known_devices.begin(),
        known_devices.end(),
        [&new_list](const auto& known_dev)
        {
            return std::any_of(new_list.begin(),
                               new_list.end(),
                               [&known_dev](const auto& new_dev) { return known_dev == new_dev; });
        });

    for (auto iter = removed_devices_begin; iter != known_devices.end();
         ++iter) // iterate over the 'removed' devices and remove them from
    {
        gst_device_provider_device_remove(GST_DEVICE_PROVIDER(self), iter->gstdev.get());
    }

    known_devices.erase(removed_devices_begin, known_devices.end());


    // sort the new devices list to [already-known,actually-new] with new_devices_begin as the pivot
    auto new_devices_begin = std::partition(
        new_list.begin(),
        new_list.end(),
        [&known_devices](const auto& new_dev)
        {
            return std::any_of(known_devices.begin(),
                               known_devices.end(),
                               [&new_dev](const auto& known_dev) { return known_dev == new_dev; });
        });

    for (auto iter = new_devices_begin; iter != new_list.end(); ++iter)
    {
        auto new_gstdev =
            gst_helper::make_ptr(tcam_mainsrc_device_new(self->state->factory_.get(), *iter));
        if (new_gstdev == nullptr)
        {
            SPDLOG_WARN("Failed to create a TcamDevice for serial={}", iter->get_name());
            continue;
        }
        self->state->known_devices_.push_back(device { *iter, new_gstdev });

        gst_device_provider_device_add(GST_DEVICE_PROVIDER(self), new_gstdev.get());
    }
}

static void update_device_list(TcamMainSrcDeviceProvider* self)
{
    tcam::set_thread_name( "tcam_gstdevlst" );
    std::unique_lock<std::mutex> lck( self->state->mtx_ );
    while (self->state->run_updates_)
    {
        {
            lck.unlock();
            auto new_list = self->state->index_.get_device_list();
            lck.lock();

            if (!self->state->run_updates_)
            { // recheck state to provide early shutdown
                return;
            }

            run_update_logic(lck, self, std::move(new_list));
        }

        self->state->cv_.wait_for(lck, std::chrono::seconds(2));
    }
}

static void tcam_mainsrc_device_provider_init(TcamMainSrcDeviceProvider* self)
{
    self->state = new tcammainsrc::provider_state();

    self->state->factory_ = gst_helper::make_ptr(gst_element_factory_find("tcammainsrc"));

    /* Ensure we can introspect the factory */
    gst_object_unref(gst_plugin_feature_load(GST_PLUGIN_FEATURE(self->state->factory_.get())));
}

static GList* tcam_mainsrc_device_provider_probe(GstDeviceProvider* provider)
{
    TcamMainSrcDeviceProvider* self = TCAM_MAINSRC_DEVICE_PROVIDER(provider);

    std::unique_lock lck(self->state->mtx_);

    GList* ret = NULL;
    if (self->state->run_updates_)
    {
        for (const auto& device_entry : self->state->known_devices_)
        {
            ret = g_list_append(ret, gst_object_ref(device_entry.gstdev.get()));
        }
    }
    else
    {
        for (const auto& device_entry : self->state->index_.get_device_list())
        {
            auto dev = tcam_mainsrc_device_new(self->state->factory_.get(), device_entry);
            if (dev == nullptr)
            {
                continue;
            }
            ret = g_list_append(ret, dev);
        }
    }
    return ret;
}

static gboolean tcam_mainsrc_device_provider_start(GstDeviceProvider* provider)
{
    TcamMainSrcDeviceProvider* self = TCAM_MAINSRC_DEVICE_PROVIDER(provider);

    std::unique_lock<std::mutex> lck(self->state->mtx_);
    run_update_logic(lck, self, self->state->index_.get_device_list());
    self->state->run_updates_ = true;
    self->state->update_thread_ = std::thread(update_device_list, self);

    return TRUE;
}

static void tcam_mainsrc_device_provider_stop(GstDeviceProvider* provider)
{
    TcamMainSrcDeviceProvider* self = TCAM_MAINSRC_DEVICE_PROVIDER(provider);

    self->state->run_updates_ = false;
    self->state->cv_.notify_all();

    self->state->update_thread_.join();

    self->state->known_devices_.clear();
}

static void tcam_mainsrc_device_provider_dispose(GObject* object)
{
    TcamMainSrcDeviceProvider* self = TCAM_MAINSRC_DEVICE_PROVIDER(object);

    if (self->state->update_thread_.joinable())
    {
        self->state->run_updates_ = false;
        self->state->cv_.notify_all();

        self->state->update_thread_.join();
    }

    self->state->factory_.reset();
    self->state->known_devices_.clear();

    G_OBJECT_CLASS(tcam_mainsrc_device_provider_parent_class)->dispose(object);
}

static void tcam_mainsrc_device_provider_finalize(GObject* object)
{
    TcamMainSrcDeviceProvider* self = TCAM_MAINSRC_DEVICE_PROVIDER(object);
    delete self->state;
    self->state = nullptr;
    G_OBJECT_CLASS(tcam_mainsrc_device_provider_parent_class)->finalize(object);
}

static void tcam_mainsrc_device_provider_class_init(TcamMainSrcDeviceProviderClass* klass)
{
    GstDeviceProviderClass* dm_class = GST_DEVICE_PROVIDER_CLASS(klass);
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = tcam_mainsrc_device_provider_dispose;
    gobject_class->finalize = tcam_mainsrc_device_provider_finalize;

    dm_class->probe = tcam_mainsrc_device_provider_probe;
    dm_class->start = tcam_mainsrc_device_provider_start;
    dm_class->stop = tcam_mainsrc_device_provider_stop;

    gst_device_provider_class_set_static_metadata(
        dm_class,
        "TCam Device Provider",
        provider_info,
        "Lists and provides tcam source devices",
        "The Imaging Source <support@theimagingsource.com>");
    GST_DEBUG_CATEGORY_INIT(
        tcam_deviceprovider_debug, "tcamdeviceprovider", 0, "tcam device provider");
}
