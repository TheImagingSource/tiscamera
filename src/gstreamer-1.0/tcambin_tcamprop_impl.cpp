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

#include "tcambin_tcamprop_impl.h"

#include "tcambin_data.h"

#include <algorithm>
#include <gst-helper/gst_gvalue_helper.h>
#include <tcamprop1.0_consumer/tcamprop1_consumer.h>
#include <tcamprop1.0_gobject/tcam_gerror.h>
#include <tcamprop1.0_gobject/tcam_property_provider.h>
#include <tcamprop1.0_gobject/tcam_property_provider_simple_functions.h>

static GSList* gst_tcambin_get_tcam_property_names(TcamPropertyProvider* iface, GError** err)
{
    tcambin_data& self = *GST_TCAMBIN(iface)->data;

    if (self.src == nullptr)
    {
        tcamprop1_gobj::set_gerror(err, tcamprop1::status::device_not_opened);
        return nullptr;
    }

    std::vector<std::string> convert_name_list;
    // if dutils(-cuda) is present then first fetch the names from that
    // tcamconvert does not offer properties
    if (self.tcam_converter && TCAM_IS_PROPERTY_PROVIDER(self.tcam_converter))
    {
        convert_name_list = tcamprop1_consumer::get_property_names_noerror(
            TCAM_PROPERTY_PROVIDER(self.tcam_converter));
    }
    auto src_prop_list_res =
        tcamprop1_consumer::get_property_names(TCAM_PROPERTY_PROVIDER(self.src));
    if (src_prop_list_res.has_error())
    {
        tcamprop1_gobj::set_gerror(err, src_prop_list_res.error());
        return nullptr;
    }

    // merge both list;
    if (convert_name_list.empty())
    {
        return gst_helper::gst_string_vector_to_GSList(src_prop_list_res.value());
    }

    auto& merged_prop_list = src_prop_list_res.value();
    merged_prop_list.insert(
        merged_prop_list.end(), convert_name_list.begin(), convert_name_list.end());

    std::sort(merged_prop_list.begin(), merged_prop_list.end());
    merged_prop_list.erase(std::unique(merged_prop_list.begin(), merged_prop_list.end()),
                           merged_prop_list.end());

    return gst_helper::gst_string_vector_to_GSList(merged_prop_list);
}


static TcamPropertyBase* gst_tcambin_get_tcam_property(TcamPropertyProvider* iface,
                                                       const char* name,
                                                       GError** err)
{
    tcambin_data& self = *GST_TCAMBIN(iface)->data;

    if (name == nullptr)
    {
        tcamprop1_gobj::set_gerror(err, tcamprop1::status::parameter_null);
        return nullptr;
    }

    if (self.src == nullptr)
    {
        tcamprop1_gobj::set_gerror(err, tcamprop1::status::device_not_opened);
        return nullptr;
    }

    if (self.tcam_converter && TCAM_IS_PROPERTY_PROVIDER(self.tcam_converter))
    {
        auto res = tcam_property_provider_get_tcam_property(
            TCAM_PROPERTY_PROVIDER(self.tcam_converter), name, nullptr);
        if (res)
        {
            return res;
        }
    }

    return tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(self.src), name, err);
}


void tcam::gst::bin::gst_tcambin_tcamprop_init(TcamPropertyProviderInterface* iface)
{
    iface->get_tcam_property_names = gst_tcambin_get_tcam_property_names;
    iface->get_tcam_property = gst_tcambin_get_tcam_property;

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
