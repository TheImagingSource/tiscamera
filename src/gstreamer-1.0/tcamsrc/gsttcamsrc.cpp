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

#include "../../base_types.h"
#include "../../../libs/tcamprop/src/tcam-property-1.0.h"
#include "../../logging.h"
#include "../../public_utils.h"
#include "../../version.h"
#include "../tcamgstbase/tcamgstjson.h"
#include "tcamsrc_tcamprop_impl.h"
#include "gsttcamdeviceprovider.h" // only needed because of plugin_init
#include "gsttcammainsrc.h" // only needed because of plugin_init
#include "tcambind.h"

#include <unistd.h>

using namespace tcam;

GST_DEBUG_CATEGORY_STATIC(tcam_src_debug);
#define GST_CAT_DEFAULT tcam_src_debug


static gboolean open_source_element(GstTcamSrc* self);



#define gst_tcam_src_parent_class parent_class

G_DEFINE_TYPE_WITH_CODE(GstTcamSrc,
                        gst_tcam_src,
                        GST_TYPE_BIN,
                        //G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROP, tcam::gst::src::gst_tcam_src_prop_init)
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER, tcam::gst::src::gst_tcam_src_prop_init))

enum
{
    SIGNAL_DEVICE_OPEN,
    SIGNAL_DEVICE_CLOSE,
    SIGNAL_LAST,
};

static guint gst_tcamsrc_signals[SIGNAL_LAST] = {
    0,
};


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
        // gst_bin_remove(GST_BIN(self), self->active_source);

        //gst_object_unref(self->active_source);
        //self->active_source = nullptr;
    }
    return true;
}


static bool is_device_already_open(GstTcamSrc* self)
{
    char* serial_str = nullptr;

    g_object_get(G_OBJECT(self->active_source), "serial", &serial_str, NULL);

    bool is_serial_same = self->device_serial == serial_str;

    g_free(serial_str);

    return is_serial_same;
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

                    bool sep_ret = tcambind::separate_serial_and_type(string_value, s, t);
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
                // if (self->active_source == self->main_src)
                // {
                //     g_object_set_property(G_OBJECT(self->active_source), "state", value);
                // }
                // else
                // {
                //     bool state = tcam::gst::load_device_settings(
                //         TCAM_PROP(self), self->device_serial, g_value_get_string(value));
                //     if (!state)
                //     {
                //         GST_WARNING("Device may be in an undefined state.");
                //     }
                // }
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


static void emit_device_open(GstElement* /*object*/, void* user_data)
{
    // emit our own instance of 'device-open'. user-data is the tcamsrc instance
    g_signal_emit(G_OBJECT(user_data), gst_tcamsrc_signals[SIGNAL_DEVICE_OPEN], 0);
}


static void emit_device_close(GstElement* /*object*/, void* user_data)
{
    // emit our own instance of 'device-open'. user-data is the tcamsrc instance
    g_signal_emit(G_OBJECT(user_data), gst_tcamsrc_signals[SIGNAL_DEVICE_CLOSE], 0);
}


static std::string get_device_serial( GstDevice& device)
{
    GstStructure* struc = gst_device_get_properties(&device);

    auto str_ptr = gst_structure_get_string(struc, "serial");
    std::string serial = str_ptr ? str_ptr : std::string {};

    gst_structure_free(struc);

    return serial;
}


static tcam::TCAM_DEVICE_TYPE get_device_type(GstDevice& device)
{
    GstStructure* struc = gst_device_get_properties(&device);

    auto str_ptr = gst_structure_get_string( struc, "type" );
    std::string type = str_ptr ? str_ptr : std::string{};

    gst_structure_free(struc);

    return tcam::tcam_device_from_string( type );
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
    GList* devices = gst_device_monitor_get_devices(self->p_monitor);

    if (!devices)
    {
        GST_ERROR("No devices to open");
        return FALSE;
    }

    if (!devices->data)
    {
        g_list_free(devices);
        GST_ERROR("No devices to open");
        return FALSE;
    }

    auto get_source = [self] (TCAM_DEVICE_TYPE t)
    {
        switch (t)
        {
            case TCAM_DEVICE_TYPE_ARAVIS:
            case TCAM_DEVICE_TYPE_V4L2:
            case TCAM_DEVICE_TYPE_LIBUSB:
            {
                return self->main_src;
            }
            case TCAM_DEVICE_TYPE_PIMIPI:
            {
                return self->pimipi_src;
            }
            case TCAM_DEVICE_TYPE_TEGRA:
            {
                return self->tegra_src;
            }
            default:
            {
                return (GstElement*)nullptr;
            }
        }
    };

    // no serial and no type, so take first
    if (self->device_serial.empty()
        && self->device_type == TCAM_DEVICE_TYPE_UNKNOWN)
    {
        GstDevice* dev = (GstDevice*)devices->data;
        TCAM_DEVICE_TYPE type = get_device_type(*dev);

        //
        // explicitly use the sources
        // NO `gst_device_create_element` or such
        // as those may cause indirection
        // e.g. tcammainsrc returns tcamsrc as element
        //

        self->device_serial = get_device_serial(*dev);
        self->active_source = get_source(type);
        self->device_type = type;

        if (!self->active_source)
        {
            GST_ERROR("Unable to identify device. No Stream possible.");
            g_list_free(devices);
            return FALSE;
        }

    }
    // type but no serial
    else if (self->device_serial.empty()
             && self->device_type != TCAM_DEVICE_TYPE_UNKNOWN)
    {
        for (GList* i = devices; i != nullptr; i = g_list_next(i))
        {
            GstDevice* dev = (GstDevice*)i->data;
            auto cur_type = get_device_type(*dev);
            if (cur_type == self->device_type)
            {
                self->active_source = get_source(self->device_type);
                self->device_serial = get_device_serial(*dev);
            }
        }
    }
    // serial but no type
    else if (!self->device_serial.empty()
             && self->device_type == TCAM_DEVICE_TYPE_UNKNOWN)
    {
        for (GList* i = devices; i != nullptr; i = g_list_next(i))
        {
            GstDevice* dev = (GstDevice*)i->data;
            std::string serial = get_device_serial(*dev);

            GST_DEBUG("Comparing '%s' to '%s'",
                      self->device_serial.c_str(),
                      serial.c_str());

            if (serial == self->device_serial)
            {
                self->device_type = get_device_type(*dev);
                self->active_source = get_source(self->device_type);
                break;
            }
        }
    }
    // serial AND type
    else
    {
        for (GList* i = devices; i != nullptr; i = g_list_next(i))
        {
            GstDevice* dev = (GstDevice*)i->data;

            std::string serial = get_device_serial(*dev);
            auto type = get_device_type(*dev);

            if (serial == self->device_serial
                && type == self->device_type)
            {
                self->active_source = get_source(type);
                break;
            }
        }
    }

    g_list_free_full(devices, gst_object_unref);

    if (!self->active_source)
    {
        GST_ERROR("Unable to open a source element. Stream not possible.");
        return FALSE;
    }

    g_signal_connect(G_OBJECT(self->active_source),
                     "device-open",
                     G_CALLBACK(emit_device_open),
                     self);

    g_signal_connect(G_OBJECT(self->active_source),
                     "device-close",
                     G_CALLBACK(emit_device_close),
                     self);

    //
    // apply stuff to actual source
    //

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
            close_source_element(self);
            break;
        }
        default:
            break;
    }
    return ret;
}


static void gst_tcam_src_init(GstTcamSrc* self)
{
    self->p_monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(self->p_monitor, "Video/Source/tcam", nullptr);

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

    gst_object_unref(self->p_monitor);

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
                // if (self->active_source == self->main_src)
                // {
                //     g_object_get_property(G_OBJECT(self->active_source), "state", value);
                // }
                // else
                // {
                //     std::string tmp =
                //         tcam::gst::create_device_settings(self->device_serial, TCAM_PROP(self))
                //             .c_str();
                //     g_value_set_string(value, tmp.c_str());
                // }
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
    gobject_class->dispose = gst_tcamsrc_dispose;
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

    gst_tcamsrc_signals[SIGNAL_DEVICE_OPEN] = g_signal_new("device-open",
                                                           G_TYPE_FROM_CLASS(klass),
                                                           G_SIGNAL_RUN_LAST,
                                                           0,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr,
                                                           G_TYPE_NONE,
                                                           0,
                                                           G_TYPE_NONE);
    gst_tcamsrc_signals[SIGNAL_DEVICE_CLOSE] = g_signal_new("device-close",
                                                            G_TYPE_FROM_CLASS(klass),
                                                            G_SIGNAL_RUN_LAST,
                                                            0,
                                                            nullptr,
                                                            nullptr,
                                                            nullptr,
                                                            G_TYPE_NONE,
                                                            0,
                                                            G_TYPE_NONE);

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
