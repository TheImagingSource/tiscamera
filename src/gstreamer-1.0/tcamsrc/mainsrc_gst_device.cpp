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

#include "mainsrc_gst_device.h"

#include <gst-helper/gst_gvalue_helper.h>
#include <string>

G_DEFINE_TYPE(TcamDevice, tcam_device, GST_TYPE_DEVICE)

static void tcam_device_init(TcamDevice* /*self*/) {}

static void tcam_device_finalize(GObject* object)
{
    G_OBJECT_CLASS(tcam_device_parent_class)->finalize(object);
}

static void tcam_device_dispose(GObject* object)
{
    TcamDevice* self = TCAM_DEVICE(object);

    if (self->factory)
    {
        gst_object_unref(self->factory);
        self->factory = nullptr;
    }
    G_OBJECT_CLASS(tcam_device_parent_class)->dispose(object);
}

static GstElement* tcam_device_create_element(GstDevice* device, const gchar* name)
{
    TcamDevice* self = TCAM_DEVICE(device);

    GstElement* ret = gst_element_factory_create(self->factory, name);
    if (ret == nullptr)
    {
        return nullptr;
    }
    GstStructure* props = gst_device_get_properties(device);
    if (props == nullptr)
    {
        return nullptr;
    }

    std::string serial = gst_helper::get_string_entry(*props, "serial");
    std::string type = gst_helper::get_string_entry(*props, "type");

    gst_structure_free(props);

    if (!serial.empty())
    {
        gst_util_set_object_arg(G_OBJECT(ret), "serial", serial.c_str());
    }
    if (!type.empty())
    {
        gst_util_set_object_arg(G_OBJECT(ret), "type", type.c_str());
    }

    return ret;
}


static void tcam_device_class_init(TcamDeviceClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstDeviceClass* gst_device_class = GST_DEVICE_CLASS(klass);

    gobject_class->finalize = tcam_device_finalize;
    gobject_class->dispose = tcam_device_dispose;

    gst_device_class->create_element = tcam_device_create_element;
}
