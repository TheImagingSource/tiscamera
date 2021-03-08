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

#include "gsttcamdevice.h"

G_DEFINE_TYPE(TcamDevice, tcam_device, GST_TYPE_DEVICE)


enum
{
    PROP_SERIAL = 1,
    PROP_MODEL,
    PROP_TYPE,
};


static void tcam_device_init(TcamDevice* self) {}

static void tcam_device_finalize(GObject* object)
{
    TcamDevice* self = TCAM_DEVICE(object);

    g_free(self->serial);
    g_free(self->model);
    g_free(self->type);

    G_OBJECT_CLASS(tcam_device_parent_class)->finalize(object);
}

static void tcam_device_dispose(GObject* object)
{
    TcamDevice* self = TCAM_DEVICE(object);

    gst_object_replace((GstObject**)&self->factory, NULL);

    G_OBJECT_CLASS(tcam_device_parent_class)->dispose(object);
}

static GstElement* tcam_device_create_element(GstDevice* device, const gchar* name)
{
    TcamDevice* self = TCAM_DEVICE(device);
    GstElement* ret;

    ret = gst_element_factory_create(self->factory, name);

    gst_util_set_object_arg(G_OBJECT(ret), "serial", self->serial);

    return ret;
}


static void gst_tcam_device_get_property(GObject* object,
                                         guint prop_id,
                                         GValue* value,
                                         GParamSpec* pspec)
{
    TcamDevice* self = TCAM_DEVICE(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            g_value_set_string(value, self->serial);
            break;
        }
        case PROP_MODEL:
        {
            g_value_set_string(value, self->model);
            break;
        }
        case PROP_TYPE:
        {
            g_value_set_string(value, self->type);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_device_set_property(GObject* object,
                                         guint prop_id,
                                         const GValue* value,
                                         GParamSpec* pspec)
{
    TcamDevice* self = TCAM_DEVICE(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            self->serial = g_strdup(g_value_get_string(value));
            break;
        }
        case PROP_MODEL:
        {
            self->model = g_strdup(g_value_get_string(value));
            break;
        }
        case PROP_TYPE:
        {
            self->type = g_strdup(g_value_get_string(value));
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void tcam_device_class_init(TcamDeviceClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstDeviceClass* gst_device_class = GST_DEVICE_CLASS(klass);

    gobject_class->finalize = tcam_device_finalize;
    gobject_class->dispose = tcam_device_dispose;

    gobject_class->get_property = gst_tcam_device_get_property;
    gobject_class->set_property = gst_tcam_device_set_property;

    gst_device_class->create_element = tcam_device_create_element;

    g_object_class_install_property(
        gobject_class,
        PROP_SERIAL,
        g_param_spec_string("serial",
                            "serial-type",
                            "tiscamera unique identifier",
                            "",
                            static_cast<GParamFlags>(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT_ONLY)));
    g_object_class_install_property(
        gobject_class,
        PROP_MODEL,
        g_param_spec_string("model",
                            "Device Model",
                            "tiscamera device description",
                            "",
                            static_cast<GParamFlags>(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT_ONLY)));
    g_object_class_install_property(
        gobject_class,
        PROP_TYPE,
        g_param_spec_string("type",
                            "device backend type",
                            "tiscamera device type",
                            "",
                            static_cast<GParamFlags>(G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT_ONLY)));
}
