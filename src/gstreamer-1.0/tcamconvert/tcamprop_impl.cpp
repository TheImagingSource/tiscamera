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

#include "tcamprop_impl.h"

#include <tcamprop_system/tcamprop_provider_itf_impl.h>

namespace
{

auto make_report_func(TcamProp* iface)
{
    auto report_func = [iface](tcamprop_system::error_id id, std::string_view dsc) {
        if (id == tcamprop_system::error_id::property_not_found)
        {
            return;
        }

        auto gst_elem = GST_ELEMENT(iface);
        GST_WARNING_OBJECT(gst_elem, "Error: %s", std::string(dsc).c_str());
    };
    return report_func;
}

} // namespace

static GSList* gst_tcamconvert_get_tcam_property_names(TcamProp* iface)
{
    auto* self = tcamconvert::get_property_list_interface(iface);

    return tcamprop_system::tcamprop_impl_get_tcam_property_names(self, make_report_func(iface));
}

static gchar* gst_tcamconvert_get_property_type(TcamProp* iface, const gchar* name)
{
    auto* self = tcamconvert::get_property_list_interface(iface);
    return tcamprop_system::tcamprop_impl_get_property_type(self, name, make_report_func(iface));
}

static gboolean gst_tcamconvert_get_tcam_property(TcamProp* iface,
                                                  const gchar* name,
                                                  GValue* value,
                                                  GValue* min,
                                                  GValue* max,
                                                  GValue* def,
                                                  GValue* stp,
                                                  GValue* type,
                                                  GValue* flags,
                                                  GValue* category,
                                                  GValue* group)
{
    auto* self = tcamconvert::get_property_list_interface(iface);
    return tcamprop_system::tcamprop_impl_get_tcam_property(self,
                                                            name,
                                                            value,
                                                            min,
                                                            max,
                                                            def,
                                                            stp,
                                                            type,
                                                            flags,
                                                            category,
                                                            group,
                                                            make_report_func(iface));
}

static gboolean gst_tcamconvert_set_tcam_property(TcamProp* iface,
                                                  const gchar* name,
                                                  const GValue* value)
{
    auto* self = tcamconvert::get_property_list_interface(iface);
    return tcamprop_system::tcamprop_impl_set_tcam_property(
        self, name, value, make_report_func(iface));
}

static GSList* gst_tcamconvert_get_menu_entries(TcamProp* iface, const char* name)
{
    auto* self = tcamconvert::get_property_list_interface(iface);
    return tcamprop_system::tcamprop_impl_get_menu_entries(self, name, make_report_func(iface));
}

void tcamconvert::gst_tcamconvert_prop_init(TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamconvert_get_tcam_property_names;
    iface->get_tcam_property_type = gst_tcamconvert_get_property_type;
    iface->get_tcam_menu_entries = gst_tcamconvert_get_menu_entries;
    iface->get_tcam_property = gst_tcamconvert_get_tcam_property;
    iface->set_tcam_property = gst_tcamconvert_set_tcam_property;
}
