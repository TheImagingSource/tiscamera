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

#include "tcamsrc_tcamprop_impl.h"

#include <gst/gst.h>
#include "gsttcamsrc.h"


/*

GSList* gst_tcamsrc_get_tcam_property_names(TcamProp* iface, GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return nullptr;
    }

    return tcam_prop_get_tcam_property_names(TCAM_PROP(self->active_source), err);
}

TcamPropertyInfo* gst_tcamsrc_get_tcam_description(TcamProp* self, const char* name, GError** err)
{
    return nullptr;
}


GSList* gst_tcamsrc_get_tcam_enum_entries(TcamProp* iface,
                                          const char* menu_name,
                                          GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return nullptr;
    }

    return tcam_prop_get_tcam_enum_entries(TCAM_PROP(self->active_source), menu_name, err);
}



TcamPropertyType gst_tcamsrc_get_tcam_type(TcamProp* iface,
                                           const char* name,
                                           GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return TCAM_PROPERTY_UNKNOWN;
    }

    return tcam_prop_get_tcam_property_type(TCAM_PROP(self->active_source), name, err);
}

gboolean gst_tcamsrc_get_tcam_range(TcamProp* self, const char* name, GValue* min, GValue* max, GValue* step, GError** err)
{

}

gboolean gst_tcamsrc_get_tcam_default(TcamProp* self, const char* name, GValue* default_value, GError** err)
{

}

//
// getter
//

gboolean gst_tcamsrc_get_tcam_property(TcamProp* self,
                                       const gchar* name,
                                       GValue* value,
                                       GValue* flags,
                                       GError** err)
{

}

const char* gst_tcamsrc_get_tcam_string(TcamProp* self,
                                        const char* name,
                                        GError** err)
{

}

gint64 gst_tcamsrc_get_tcam_int(TcamProp* self,
                                const char* name,
                                GError** err)
{

}

gdouble gst_tcamsrc_get_tcam_double(TcamProp* self,
                                    const char* name,
                                    GError** err)
{

}

gboolean gst_tcamsrc_get_tcam_bool(TcamProp* self,
                                   const char* name,
                                   GError** err)
{

}

//
// setter
//

gboolean gst_tcamsrc_set_tcam_property(TcamProp* iface, const gchar* name, const GValue* value, GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (!self->active_source)
    {
        GST_WARNING("Source must be in state READY or higher.");
        return FALSE;
    }

    return tcam_prop_set_tcam_property(TCAM_PROP(self->active_source), name, value, err);
}

gboolean gst_tcamsrc_set_tcam_string(TcamProp* self,
                            const char* name,
                            const char* value,
                            GError** err)
{

}

gboolean gst_tcamsrc_set_tcam_int(TcamProp* self,
                         const char* name,
                         gint64 value,
                         GError** err)
{

}

gboolean gst_tcamsrc_set_tcam_double(TcamProp* self,
                            const char* name,
                            gdouble value,
                            GError** err)
{

}

gboolean gst_tcamsrc_set_tcam_bool(TcamProp* self,
                          const char* name,
                          gboolean value,
                          GError** err)
{

}

gboolean gst_tcamsrc_set_tcam_execute(TcamProp* self,
                             const char* name,
                             GError** err)
{

}

*/

static GSList* gst_tcamsrc_get_tcam_property_names (TcamPropertyProvider* iface, GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);

    if (self->active_source)
    {
        return tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(self->active_source), err);
    }

    // TODO set err

    return nullptr;
}


static TcamPropertyBase* gst_tcamsrc_get_tcam_property(TcamPropertyProvider* iface, const char* name, GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);


    if (self->active_source)
    {
        return tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(self->active_source), name, err);
    }

    // TODO set err

    return nullptr;
}

void tcam::gst::src::gst_tcam_src_prop_init(TcamPropertyProviderInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamsrc_get_tcam_property_names;
    iface->get_tcam_property = gst_tcamsrc_get_tcam_property;

    // iface->get_tcam_property_names = gst_tcamsrc_get_tcam_property_names;
    // iface->get_tcam_description = gst_tcamsrc_get_tcam_description;
    // iface->get_tcam_enum_entries = gst_tcamsrc_get_tcam_enum_entries;
    // iface->get_tcam_type = gst_tcamsrc_get_tcam_type;
    // iface->get_tcam_range = gst_tcamsrc_get_tcam_range;
    // iface->get_tcam_default = gst_tcamsrc_get_tcam_default;
    // iface->get_tcam_property = gst_tcamsrc_get_tcam_property;
    // iface->get_tcam_string = gst_tcamsrc_get_tcam_string;
    // iface->get_tcam_int = gst_tcamsrc_get_tcam_int;
    // iface->get_tcam_double = gst_tcamsrc_get_tcam_double;
    // iface->get_tcam_bool = gst_tcamsrc_get_tcam_bool;
    // iface->set_tcam_property = gst_tcamsrc_set_tcam_property;
    // iface->set_tcam_string = gst_tcamsrc_set_tcam_string;
    // iface->set_tcam_int = gst_tcamsrc_set_tcam_int;
    // iface->set_tcam_double = gst_tcamsrc_set_tcam_double;
    // iface->set_tcam_bool = gst_tcamsrc_set_tcam_bool;
    // iface->set_tcam_execute = gst_tcamsrc_set_tcam_execute;

}
