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

#include "gsttcamsrc.h"

#include <tcamprop1.0_gobject/tcam_gerror.h>
#include <tcamprop1.0_gobject/tcam_property_provider_simple_functions.h>

static GSList* gst_tcamsrc_get_tcam_property_names(TcamPropertyProvider* iface, GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);
    auto src = tcamsrc::get_active_source(self);
    if (src)
    {
        return tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(src), err);
    }

    tcamprop1_gobj::set_gerror(err, TCAM_ERROR_DEVICE_NOT_OPENED);

    return nullptr;
}


static TcamPropertyBase* gst_tcamsrc_get_tcam_property(TcamPropertyProvider* iface,
                                                       const char* name,
                                                       GError** err)
{
    GstTcamSrc* self = GST_TCAM_SRC(iface);
    auto src = tcamsrc::get_active_source(self);
    if (src)
    {
        return tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(src), name, err);
    }

    tcamprop1_gobj::set_gerror(err, TCAM_ERROR_DEVICE_NOT_OPENED);

    return nullptr;
}

void tcam::gst::src::gst_tcam_src_prop_init(TcamPropertyProviderInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamsrc_get_tcam_property_names;
    iface->get_tcam_property = gst_tcamsrc_get_tcam_property;

    iface->set_tcam_boolean = tcamprop1_gobj::provider_set_tcam_boolean;
    iface->set_tcam_integer = tcamprop1_gobj::provider_set_tcam_integer;
    iface->set_tcam_float = tcamprop1_gobj::provider_set_tcam_float;
    iface->set_tcam_enumeration = tcamprop1_gobj::provider_set_tcam_enumeration;
    iface->set_tcam_command = tcamprop1_gobj::provider_set_tcam_command;

    iface->get_tcam_boolean = tcamprop1_gobj::provider_get_tcam_boolean;
    iface->get_tcam_integer = tcamprop1_gobj::provider_get_tcam_integer;
    iface->get_tcam_float = tcamprop1_gobj::provider_get_tcam_float;
    iface->get_tcam_enumeration = tcamprop1_gobj::provider_get_tcam_enumeration;
}
