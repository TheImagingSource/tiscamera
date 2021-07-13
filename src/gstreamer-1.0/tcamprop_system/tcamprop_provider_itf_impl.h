
#pragma once

#include "tcamprop_provider_base.h"

#include <functional>
#include <gst/gst.h>
#include <string_view>

namespace tcamprop_system
{
enum class error_id
{
    parameter_nullptr,
    parameter_invalid_value,
    property_type_incompatible,
    property_not_found,
    property_failed_get_flags,
    property_failed_get_value,
    property_failed_get_range,
    set_function_failed,
};

using report_error = std::function<void(error_id id, std::string_view dsc)>;

GSList* tcamprop_impl_get_tcam_property_names(
    tcamprop_system::property_list_interface* prop_list_itf,
    const report_error& report_err_func);
gboolean tcamprop_impl_get_tcam_property(tcamprop_system::property_list_interface* prop_list_itf,
                                         const gchar* name,
                                         GValue* value,
                                         GValue* min,
                                         GValue* max,
                                         GValue* def,
                                         GValue* stp,
                                         GValue* type,
                                         GValue* flags,
                                         GValue* category,
                                         GValue* group,
                                         const report_error& report_err_func);
gboolean tcamprop_impl_set_tcam_property(tcamprop_system::property_list_interface* prop_list_itf,
                                         const gchar* name,
                                         const GValue* value,
                                         const report_error& report_err_func);

gchar* tcamprop_impl_get_property_type(tcamprop_system::property_list_interface* prop_list_itf,
                                       const gchar* name,
                                       const report_error& report_err_func);
GSList* tcamprop_impl_get_menu_entries(tcamprop_system::property_list_interface* prop_list_itf,
                                       const gchar* name,
                                       const report_error& report_err_func);
} // namespace tcamprop_system