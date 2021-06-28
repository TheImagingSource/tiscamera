/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

#include "gsttcamsrc.h"

#include "../../gobject/tcamprop.h"
#include "../../logging.h"
#include "../../tcam.h"
#include "../tcamgstbase.h"
#include "../tcamgstjson.h"
#include "../tcamgststrings.h"
#include "gsttcamdeviceprovider.h"
#include "gsttcammainsrc.h"

#include <algorithm>
#include <assert.h>
#include <queue>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>


GST_DEBUG_CATEGORY_STATIC(tcam_src_debug);
#define GST_CAT_DEFAULT tcam_src_debug


static GSList* gst_tcam_src_get_property_names(TcamProp* self);

static gchar* gst_tcam_src_get_property_type(TcamProp* self, const gchar* name);

static gboolean gst_tcam_src_get_tcam_property(TcamProp* self,
                                               const gchar* name,
                                               GValue* value,
                                               GValue* min,
                                               GValue* max,
                                               GValue* def,
                                               GValue* step,
                                               GValue* type,
                                               GValue* flags,
                                               GValue* category,
                                               GValue* group);

static GSList* gst_tcam_src_get_menu_entries(TcamProp* iface, const char* menu_name);

static gboolean gst_tcam_src_set_tcam_property(TcamProp* self,
                                               const gchar* name,
                                               const GValue* value);

static GSList* gst_tcam_src_get_device_serials(TcamProp* self);
static GSList* gst_tcam_src_get_device_serials_backend(TcamProp* self);

static gboolean gst_tcam_src_get_device_info(TcamProp* self,
                                             const char* serial,
                                             char** name,
                                             char** identifier,
                                             char** connection_type);

static gboolean open_source_element(GstTcamSrc* self);


static void gst_tcam_src_prop_init(TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcam_src_get_property_names;
    iface->get_tcam_property_type = gst_tcam_src_get_property_type;
    iface->get_tcam_property = gst_tcam_src_get_tcam_property;
    iface->get_tcam_menu_entries = gst_tcam_src_get_menu_entries;
    iface->set_tcam_property = gst_tcam_src_set_tcam_property;
    iface->get_tcam_device_serials = gst_tcam_src_get_device_serials;
    iface->get_tcam_device_serials_backend = gst_tcam_src_get_device_serials_backend;
    iface->get_tcam_device_info = gst_tcam_src_get_device_info;
}


#define gst_tcam_src_parent_class parent_class

G_DEFINE_TYPE_WITH_CODE(GstTcamSrc,
                        gst_tcam_src,
                        GST_TYPE_BIN,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, gst_tcam_src_prop_init))

/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamSrcProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
static gchar* gst_tcam_src_get_property_type(TcamProp* iface, const gchar* name)
{
    const gchar* ret = NULL;
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return nullptr;
    }

    ret = tcam_prop_get_tcam_property_type(TCAM_PROP(self->active_source), name);


    return (gchar*)ret;
}

/**
 * gst_tcam_src_get_property_names:
 * @self: a #GstTcamSrc
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
static GSList* gst_tcam_src_get_property_names(TcamProp* iface)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return nullptr;
    }

    return tcam_prop_get_tcam_property_names(TCAM_PROP(self->active_source));
}


static gboolean gst_tcam_src_get_tcam_property(TcamProp* iface,
                                               const gchar* name,
                                               GValue* value,
                                               GValue* min,
                                               GValue* max,
                                               GValue* def,
                                               GValue* step,
                                               GValue* type,
                                               GValue* flags,
                                               GValue* category,
                                               GValue* group)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return FALSE;
    }

    return tcam_prop_get_tcam_property(TCAM_PROP(self->active_source),
                                       name,
                                       value,
                                       min,
                                       max,
                                       def,
                                       step,
                                       type,
                                       flags,
                                       category,
                                       group);
}


static GSList* gst_tcam_src_get_menu_entries(TcamProp* iface, const char* menu_name)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return nullptr;
    }

    return tcam_prop_get_tcam_menu_entries(TCAM_PROP(self->active_source), menu_name);
}


static gboolean gst_tcam_src_set_tcam_property(TcamProp* iface,
                                               const gchar* name,
                                               const GValue* value)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return FALSE;
    }

    return tcam_prop_set_tcam_property(TCAM_PROP(self->active_source), name, value);
}


static GSList* gst_tcam_src_get_device_serials(TcamProp* iface)
{

    GstTcamSrc* self = GST_TCAM_SRC(iface);

    GSList* ret = nullptr;

    for (GSList* iter = self->source_list; iter != nullptr; iter = g_slist_next(iter))
    {
        GSList* tmp = nullptr;

        tmp = tcam_prop_get_device_serials(TCAM_PROP(iter->data));

        // takes ownership of tmp elements
        // no g_slist_free required
        ret = g_slist_concat(ret, tmp);
    }


    return ret;
}


static GSList* gst_tcam_src_get_device_serials_backend(TcamProp* iface)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    GSList* ret = nullptr;

    for (GSList* iter = self->source_list; iter != nullptr; iter = g_slist_next(iter))
    {
        GSList* tmp = nullptr;

        if (!iter->data)
        {
            GST_DEBUG("Source list entry is empty.");
            continue;
        }

        tmp = tcam_prop_get_device_serials_backend(TCAM_PROP(iter->data));

        // takes ownership of tmp elements
        // no g_slist_free required
        ret = g_slist_concat(ret, tmp);
    }

    return ret;
}


static gboolean gst_tcam_src_get_device_info(TcamProp* iface,
                                             const char* serial,
                                             char** name,
                                             char** identifier,
                                             char** connection_type)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    gboolean ret = FALSE;

    for (GSList* iter = self->source_list; iter != nullptr; iter = g_slist_next(iter))
    {
        ret = tcam_prop_get_device_info(
            TCAM_PROP(iter->data), serial, name, identifier, connection_type);

        if (ret)
        {
            break;
        }
    }

    return ret;
}


enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_TYPE,
    PROP_CAM_BUFFERS,
    PROP_NUM_BUFFERS,
    PROP_DO_TIMESTAMP,
    PROP_DROP_INCOMPLETE_FRAMES,
    PROP_STATE,
};


static GstStaticPadTemplate tcam_src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

static bool close_source_element(GstTcamSrc* self)
{
    GstState state;
    gst_element_get_state(GST_ELEMENT(self), &state, nullptr, 1000000);

    if (state > GST_STATE_NULL)
    {
        GST_ERROR("Active source is neither in GST_STATE_NULL nor GST_STATE_READY. Not closing.");
        return false;
    }

    if (self->active_source)
    {
        if (state != GST_STATE_NULL)
        {
            gst_element_set_state(self->active_source, GST_STATE_NULL);
        }
        // TODO causes critical error  g_object_ref: assertion 'old_val > 0' failed
        //gst_bin_remove(GST_BIN(self), self->active_source);

        self->active_source = nullptr;
    }
    return true;
}


static gboolean is_device_already_open(GstTcamSrc* self)
{
    char* serial_str = nullptr;

    g_object_get(G_OBJECT(self->active_source), "serial", &serial_str, NULL);

    bool is_serial_same = g_strcmp0(self->device_serial.c_str(), serial_str) == 0;

    g_free(serial_str);

    return is_serial_same ? TRUE : FALSE;
}


static void apply_element_property(GstTcamSrc* self,
                                   guint prop_id,
                                   const GValue* value,
                                   GParamSpec* pspec)
{

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            GstState state;
            gst_element_get_state(GST_ELEMENT(self), &state, NULL, 200);
            if (state == GST_STATE_NULL)
            {
                if (g_value_get_string(value) == NULL)
                {
                    self->device_serial.clear();
                }
                else
                {
                    std::string string_value = g_value_get_string(value);

                    std::string s;
                    std::string t;

                    bool sep_ret = separate_serial_and_type(string_value, s, t);
                    if (sep_ret)
                    {
                        GST_INFO("Serial-Type input detected. Using serial: '%s' type: '%s'",
                                 s.c_str(),
                                 t.c_str());

                        self->device_serial = s;
                        self->device_type = tcam::tcam_device_from_string(t);

                        GST_DEBUG("Type interpreted as '%s'",
                                  tcam::tcam_device_type_to_string(self->device_type).c_str());
                    }
                    else
                    {
                        self->device_serial = string_value;
                        //self->device_type = TCAM_DEVICE_TYPE_UNKNOWN;
                    }
                    // no else
                    // self->device_serial and s are already equal
                }

                GST_INFO("Set camera serial to %s", self->device_serial.c_str());
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            const char* type = g_value_get_string(value);

            // this check is simply for messaging the user
            // about invalid values
            auto vec = tcam::get_device_type_list_strings();
            if (std::find(vec.begin(), vec.end(), std::string(type)) == vec.end())
            {
                GST_ERROR("Unknown device type '%s'", type);
                self->device_type = TCAM_DEVICE_TYPE_UNKNOWN;
            }
            else
            {
                GST_DEBUG("Setting device type to %s", type);
                self->device_type = tcam::tcam_device_from_string(type);
            }
            break;
        }
        case PROP_CAM_BUFFERS:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "camera-buffers"))
            {
                g_object_set_property(G_OBJECT(self->active_source), "camera-buffers", value);
            }
            else
            {
                if (self->active_source)
                {
                    GST_INFO("Used source element does not support \"camera-buffers\".");
                }
                else
                {
                    self->cam_buffers = g_value_get_int(value);
                }
            }

            break;
        }
        case PROP_NUM_BUFFERS:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "num-buffers"))
            {
                g_object_set_property(G_OBJECT(self->active_source), "num-buffers", value);
            }
            else
            {
                if (self->active_source)
                {
                    GST_INFO("Used source element does not support \"num-buffers\".");
                }
                else
                {
                    self->num_buffers = g_value_get_int(value);
                }
            }

            break;
        }
        case PROP_DO_TIMESTAMP:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "do-timestamp"))
            {
                g_object_set_property(G_OBJECT(self->active_source), "do-timestamp", value);
            }
            else
            {
                self->do_timestamp = g_value_get_boolean(value);
            }
            break;
        }
        case PROP_DROP_INCOMPLETE_FRAMES:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "drop-incomplete-buffer"))
            {
                g_object_set_property(
                    G_OBJECT(self->active_source), "drop-incomplete-buffer", value);
            }
            else
            {
                if (self->active_source)
                {
                    GST_INFO("Used source element does not support \"drop-incomplete-buffer\"");
                }
                else
                {
                    self->drop_incomplete_frames = g_value_get_boolean(value);
                }
            }
            break;
        }
        case PROP_STATE:
        {

            if (self->active_source)
            {
                if (self->active_source == self->main_src)
                {
                    g_object_set_property(G_OBJECT(self->active_source), "state", value);
                }
                else
                {
                    bool state = load_device_settings(
                        TCAM_PROP(self), self->device_serial, g_value_get_string(value));
                    if (!state)
                    {
                        GST_WARNING("Device may be in an undefined state.");
                    }
                }
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(G_OBJECT(self), prop_id, pspec);
            break;
        }
    }
}


static gboolean open_source_element(GstTcamSrc* self)
{

    if (self->active_source)
    {
        // check if the wanted device is already open
        // the nothing is to do
        if (is_device_already_open(self))
        {
            return TRUE;
        }
        if (!close_source_element(self))
        {
            return FALSE;
        }
    }

    /*
      How source selection works

      if serial exists -> use first matching source
      if serial && type exist -> use matching source
      if no serial && type -> matching source
      if no source && no type -> first source with device available

      mainsrc has prevalence over other sources for unspecified devices
     */

    if (self->device_serial.empty() && self->device_type == TCAM_DEVICE_TYPE_UNKNOWN)
    {
        GSList* serials = tcam_prop_get_device_serials_backend(TCAM_PROP(self));
        if (!serials)
        {
            GST_ERROR("No devices to open");
            return FALSE;
        }

        auto vals = separate_serial_and_type((const char*)serials->data);

        TCAM_DEVICE_TYPE type = tcam::tcam_device_from_string(vals.second);

        g_slist_free_full(serials, g_free);

        if (type == TCAM_DEVICE_TYPE_ARAVIS || type == TCAM_DEVICE_TYPE_V4L2
            || type == TCAM_DEVICE_TYPE_LIBUSB)
        {
            self->active_source = self->main_src;
            self->device_type = type;
            self->device_serial = vals.first;
        }
        else if (type == TCAM_DEVICE_TYPE_PIMIPI)
        {
            self->active_source = self->pimipi_src;
            self->device_type = type;
            self->device_serial = vals.first;
        }
        else if (type == TCAM_DEVICE_TYPE_TEGRA)
        {
            self->active_source = self->tegra_src;
            self->device_type = type;
            self->device_serial = vals.first;
        }
        else
        {
            GST_ERROR("Unable to identify device. No Stream possible.");
            return FALSE;
        }
    }
    else
    {
        for (GSList* iter = self->source_list; iter != nullptr && !self->active_source;
             iter = g_slist_next(iter))
        {
            if (!iter->data)
            {
                GST_DEBUG("!!!");
                continue;
            }

            GstElement* cur_elem = (GstElement*)iter->data;

            GSList* tmp = nullptr;
            if (self->device_type == TCAM_DEVICE_TYPE_UNKNOWN && !self->device_serial.empty())
            {
                GST_INFO("Searching for '%s'", self->device_serial.c_str());
                tmp = tcam_prop_get_device_serials(TCAM_PROP(cur_elem));

                for (GSList* i = tmp; i != nullptr; i = g_slist_next(i))
                {
                    GST_DEBUG("Comparing '%s' to '%s'",
                              self->device_serial.c_str(),
                              (const char*)i->data);

                    if (g_strcmp0((const char*)i->data, self->device_serial.c_str()) == 0)
                    {
                        self->active_source = cur_elem;
                        break;
                    }
                }
            }
            else
            {
                tmp = tcam_prop_get_device_serials_backend(TCAM_PROP(cur_elem));

                for (GSList* i = tmp; i != nullptr; i = g_slist_next(i))
                {
                    std::string serial;
                    std::string type_str;
                    separate_serial_and_type((const char*)i->data, serial, type_str);

                    if (serial == self->device_serial
                        && tcam::tcam_device_from_string(type_str) == self->device_type)
                    {
                        self->active_source = cur_elem;
                        break;
                    }
                    // serial empty && type wished
                    else if (self->device_serial.empty()
                             && tcam::tcam_device_from_string(type_str) == self->device_type)
                    {
                        self->active_source = cur_elem;
                        break;
                    }
                }
            }
            g_slist_free_full(tmp, g_free);
        }
    }

    if (!self->active_source)
    {
        GST_ERROR("Unable to open a source element. Stream not possible.");
        return FALSE;
    }

    if (self->active_source == self->main_src)
    {
        g_object_set(self->active_source,
                     "type",
                     tcam::tcam_device_type_to_string(self->device_type).c_str(),
                     NULL);
    }

    g_object_set(self->active_source, "serial", self->device_serial.c_str(), NULL);


    gst_bin_add(GST_BIN(self), self->active_source);
    // bin takes ownership over source element
    // we want to hold all source elements outside of
    // the bin for indexing purposes
    g_object_ref(self->active_source);

    gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), NULL);
    auto target_pad = gst_element_get_static_pad(self->active_source, "src");

    if (!gst_ghost_pad_set_target(GST_GHOST_PAD(self->pad), target_pad))
    {
        GST_ERROR("Could not set target for ghostpad.");
    }
    else
    {
        GST_DEBUG("Ghost pad target set");
    }
    gst_object_unref(target_pad);

    gst_element_set_state(self->active_source, GST_STATE_READY);

    GValue val = G_VALUE_INIT;

    g_value_init(&val, G_TYPE_INT);
    g_value_set_int(&val, self->cam_buffers);
    // manually set all properties to ensure they are correctly applied
    apply_element_property(self, PROP_CAM_BUFFERS, &val, nullptr);

    // g_value_reset(&val);
    // g_value_init(&val, G_TYPE_INT);
    g_value_set_int(&val, self->num_buffers);

    apply_element_property(self, PROP_NUM_BUFFERS, &val, nullptr);

    // g_value_reset(&val);

    GValue val_bool = G_VALUE_INIT;

    g_value_init(&val_bool, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val_bool, self->drop_incomplete_frames);

    apply_element_property(self, PROP_DROP_INCOMPLETE_FRAMES, &val_bool, nullptr);

    // g_value_reset(&val);
    // g_value_init(&val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val_bool, self->do_timestamp);

    apply_element_property(self, PROP_DO_TIMESTAMP, &val_bool, nullptr);

    // query these as late as possible
    // src needs some time as things can happen async
    char* device_serial = nullptr;
    g_object_get(G_OBJECT(self->active_source), "serial", &device_serial, NULL);
    if (device_serial)
    {
        self->device_serial = device_serial;
        g_free(device_serial);
    }
    else
    {
        self->device_serial = {};
    }

    if (self->active_source == self->main_src)
    {
        char* type_str = nullptr;
        g_object_get(G_OBJECT(self->active_source), "type", &type_str, NULL);

        if (type_str)
        {
            self->device_type = tcam::tcam_device_from_string(type_str);
            g_free(type_str);
        }
        else
        {
            self->device_type = TCAM_DEVICE_TYPE_UNKNOWN;
        }
    }
    else if (self->active_source == self->pimipi_src)
    {
        self->device_type = TCAM_DEVICE_TYPE_PIMIPI;
    }
    else if (self->active_source == self->tegra_src)
    {
        self->device_type = TCAM_DEVICE_TYPE_TEGRA;
    }
    else
    {
        self->device_type = TCAM_DEVICE_TYPE_UNKNOWN;
    }

    GST_INFO("Opened device has serial: '%s' type: '%s'",
             self->device_serial.c_str(),
             tcam::tcam_device_type_to_string(self->device_type).c_str());

    return TRUE;
}


static GstStateChangeReturn gst_tcam_src_change_state(GstElement* element, GstStateChange change)
{

    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamSrc* self = GST_TCAM_SRC(element);

    switch (change)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_DEBUG("State change: NULL -> READY");

            if (!open_source_element(self))
            {
                GST_ERROR("Cannot proceed. Aborting");
                return GST_STATE_CHANGE_FAILURE;
            }
            else
            {
                GST_INFO("Opened source element");
            }

            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcam_src_parent_class)->change_state(element, change);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (change)
    {
        case GST_STATE_CHANGE_READY_TO_NULL:
        {

            break;
        }
        default:
            break;
    }
    return ret;
}


static void gst_tcam_src_init(GstTcamSrc* self)
{

    g_object_set(self, "message-forward", TRUE, NULL);
    self->active_source = nullptr;

    self->source_list = nullptr;
    new (&self->device_serial) std::string("");
    self->device_type = TCAM_DEVICE_TYPE_UNKNOWN;

    auto mainsrc_fact = gst_element_factory_find("tcammainsrc");
    if (mainsrc_fact)
    {
        self->main_src = gst_element_factory_make("tcammainsrc", "tcamsrc-mainsrc");
        if (self->main_src != nullptr)
        {
            self->source_list = g_slist_append(self->source_list, self->main_src);
        }
        gst_object_unref(mainsrc_fact);
    }

    auto pimipi_fact = gst_element_factory_find("tcampimipisrc");
    if (pimipi_fact)
    {
        self->pimipi_src = gst_element_factory_make("tcampimipisrc", "tcamsrc-pimipisrc");
        if (self->pimipi_src != nullptr)
        {
            self->source_list = g_slist_append(self->source_list, self->pimipi_src);
        }
        gst_object_unref(pimipi_fact);
    }

    auto tegra_fact = gst_element_factory_find("tcamtegrasrc");
    if (tegra_fact)
    {
        self->tegra_src = gst_element_factory_make("tcamtegrasrc", "tcamsrc-tegrasrc");
        if (self->tegra_src != nullptr)
        {
            self->source_list = g_slist_append(self->source_list, self->tegra_src);
        }
        gst_object_unref(tegra_fact);
    }

    self->pad = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);
    gst_element_add_pad(GST_ELEMENT(self), self->pad);

    self->cam_buffers = 10;
    self->do_timestamp = false;
    self->drop_incomplete_frames = true;
    self->num_buffers = -1;
}


static void gst_tcam_src_finalize(GObject* object)
{
    GstTcamSrc* self = GST_TCAM_SRC(object);

    if (self->active_source)
    {
        gst_element_set_state(self->active_source, GST_STATE_NULL);

        close_source_element(self);
    }
    g_slist_free(self->source_list);

    // source elements have to be destroyed manually as they are not in the bin
    if (self->main_src)
    {
        gst_object_unref(self->main_src);
        self->main_src = nullptr;
    }
    if (self->pimipi_src)
    {
        gst_object_unref(self->pimipi_src);
        self->pimipi_src = nullptr;
    }
    if (self->tegra_src)
    {
        gst_object_unref(self->tegra_src);
        self->tegra_src = nullptr;
    }
    (&self->device_serial)->std::string::~string();

    G_OBJECT_CLASS(gst_tcam_src_parent_class)->finalize(object);
}


static void gst_tcamsrc_dispose(GObject* object)
{
    GstTcamSrc* self = GST_TCAM_SRC(object);

    if (self->pad)
    {
        gst_element_remove_pad(GST_ELEMENT(self), self->pad);
        self->pad = nullptr;
    }

    G_OBJECT_CLASS(GST_ELEMENT_CLASS(parent_class))->dispose(object);
}


static void gst_tcam_src_set_property(GObject* object,
                                      guint prop_id,
                                      const GValue* value,
                                      GParamSpec* pspec)
{
    GstTcamSrc* self = GST_TCAM_SRC(object);

    apply_element_property(self, prop_id, value, pspec);
}


static void gst_tcam_src_get_property(GObject* object,
                                      guint prop_id,
                                      GValue* value,
                                      GParamSpec* pspec)
{
    GstTcamSrc* self = GST_TCAM_SRC(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            g_value_set_string(value, self->device_serial.c_str());
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source), "type"))
            {
                g_object_get_property(G_OBJECT(self->active_source), "type", value);
            }
            else
            {
                g_value_set_string(value,
                                   tcam::tcam_device_type_to_string(self->device_type).c_str());
            }
            break;
        }
        case PROP_CAM_BUFFERS:
        {
            if (self->active_source)
            {
                if (self->active_source
                    && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                    "camera-buffers"))
                {
                    g_object_get_property(G_OBJECT(self->active_source), "camera-buffers", value);
                }
                else
                {
                    GST_WARNING("Source element does not support camera-buffers.");
                }
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            if (self->active_source)
            {
                if (self->active_source
                    && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                    "num-buffers"))
                {
                    g_object_get_property(G_OBJECT(self->active_source), "num-buffers", value);
                }
                else
                {
                    GST_WARNING("Source element does not support num-buffers.");
                }
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        case PROP_DO_TIMESTAMP:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "do-timestamp"))
            {
                g_object_get_property(G_OBJECT(self->active_source), "do-timestamp", value);
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        case PROP_DROP_INCOMPLETE_FRAMES:
        {
            if (self->active_source
                && g_object_class_find_property(G_OBJECT_GET_CLASS(self->active_source),
                                                "drop-incomplete-buffer"))
            {
                g_object_get_property(
                    G_OBJECT(self->active_source), "drop-incomplete-buffer", value);
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        case PROP_STATE:
        {
            if (self->active_source)
            {
                if (self->active_source == self->main_src)
                {
                    g_object_get_property(G_OBJECT(self->active_source), "state", value);
                }
                else
                {
                    std::string tmp =
                        create_device_settings(self->device_serial, TCAM_PROP(self)).c_str();
                    g_value_set_string(value, tmp.c_str());
                }
            }
            else
            {
                GST_WARNING("No active source.");
            }
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_src_class_init(GstTcamSrcClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    gobject_class->finalize = gst_tcam_src_finalize;
    gobject_class->dispose = (GObjectFinalizeFunc)gst_tcamsrc_dispose;
    gobject_class->set_property = gst_tcam_src_set_property;
    gobject_class->get_property = gst_tcam_src_get_property;

    g_object_class_install_property(
        gobject_class,
        PROP_SERIAL,
        g_param_spec_string("serial",
                            "Camera serial",
                            "Serial of the camera",
                            NULL,
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_DEVICE_TYPE,
        g_param_spec_string("type",
                            "Camera type",
                            "type/backend of the camera",
                            "auto",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_CAM_BUFFERS,
        g_param_spec_int("camera-buffers",
                         "Number of Buffers",
                         "Number of buffers to use for retrieving images",
                         1,
                         256,
                         10,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
    g_object_class_install_property(
        gobject_class,
        PROP_NUM_BUFFERS,
        g_param_spec_int("num-buffers",
                         "Number of Buffers",
                         "Number of buffers to send before ending pipeline (-1 = unlimited)",
                         -1,
                         G_MAXINT,
                         -1,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
    g_object_class_install_property(
        gobject_class,
        PROP_DO_TIMESTAMP,
        g_param_spec_boolean("do-timestamp",
                             "Do timestamp",
                             "Apply current stream time to buffers",
                             true,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
                                                      | G_PARAM_CONSTRUCT)));
    g_object_class_install_property(
        gobject_class,
        PROP_DROP_INCOMPLETE_FRAMES,
        g_param_spec_boolean("drop-incomplete-buffer",
                             "Drop incomplete buffers",
                             "Drop buffer that are incomplete.",
                             true,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
                                                      | G_PARAM_CONSTRUCT)));


    g_object_class_install_property(
        gobject_class,
        PROP_STATE,
        g_param_spec_string("state",
                            "Property State",
                            "Property values the internal elements shall use",
                            "",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    GST_DEBUG_CATEGORY_INIT(tcam_src_debug, "tcamsrc", 0, "tcam interface");

    gst_element_class_set_static_metadata(element_class,
                                          "Tcam Video Source",
                                          "Source/Video",
                                          "Tcam based source",
                                          "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&tcam_src_template));

    element_class->change_state = gst_tcam_src_change_state;
}


static gboolean plugin_init(GstPlugin* plugin)
{
    GST_DEBUG_CATEGORY_INIT(tcam_src_debug, "tcamsrc", 0, "tcamsrc");

    gst_device_provider_register(
        plugin, "tcamdeviceprovider", GST_RANK_PRIMARY, TCAM_TYPE_DEVICE_PROVIDER);
    gst_element_register(plugin, "tcamsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_SRC);
    gst_element_register(plugin, "tcammainsrc", GST_RANK_PRIMARY, GST_TYPE_TCAM_MAINSRC);

    return TRUE;
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif


GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  tcamsrc,
                  "TCam Video Source",
                  plugin_init,
                  get_version(),
                  "Proprietary",
                  "tcamsrc",
                  "theimagingsource.com")
