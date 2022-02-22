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

#include "aravis_property_impl.h"

#include "../logging.h"
#include "AravisPropertyBackend.h"
#include "aravis_utils.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>

using namespace tcam::aravis;

namespace
{
auto to_stdstring(const char* f) -> std::string
{
    return f == nullptr ? std::string {} : f;
}


tcamprop1::prop_static_info_str get_static_feature_node_info(ArvGcFeatureNode* node)
{
    tcamprop1::prop_static_info_str rval;

    rval.name = to_stdstring(arv_gc_feature_node_get_name(node));
    rval.iccategory = {};
    rval.display_name = to_stdstring(arv_gc_feature_node_get_display_name(node));
    rval.description = to_stdstring(arv_gc_feature_node_get_description(node));
    rval.visibility = to_Visibility(arv_gc_feature_node_get_visibility(node));
    rval.access = to_Access(arv_gc_feature_node_get_actual_access_mode(node));
    return rval;
}

#if 0 // not used
static void update_prop_info_from_tcamprop1_list(
    const tcamprop1::prop_static_info_str& prop_info,
    const tcamprop1::prop_static_info& static_info)
{
    if (prop_info.description != static_info.description)
        SPDLOG_INFO("Property '{}' description='{}' != tcamprop1-description='{}",
                    prop_info.name,
                    prop_info.description,
                    static_info.description);
    if (prop_info.display_name != static_info.display_name)
        SPDLOG_INFO("Property '{}' display_name='{}' != tcamprop1-display_name='{}",
                    prop_info.name,
                    prop_info.display_name,
                    static_info.display_name);
    if (prop_info.visibility != static_info.visibility)
        SPDLOG_INFO("Property '{}' visibility='{}' != tcamprop1-visibility='{}",
                    prop_info.name,
                    to_string(prop_info.visibility),
                    to_string(static_info.visibility));
        //if (prop_info.access != static_info.access)
        //    SPDLOG_INFO("Property '{}' display_name='{}' != tcamprop1-display_name='{}",
        //                prop_info.name,
        //                to_string(prop_info.access),
        //                to_string(static_info.access));
}
#endif


void update_with_tcamprop1_static_info(std::string_view tcamprop1_name,
                                       tcamprop1::prop_static_info_str& gc_node_info,
                                       tcamprop1::prop_type gc_node_type)
{
    auto static_info = tcamprop1::find_prop_static_info(tcamprop1_name);
    if (!static_info)
    {
        // This is not an error anymore, many GenICam properties might not have a entry in the prop_list
        // SPDLOG_DEBUG("tcamprop1 information for '{}' not found!", tcamprop1_name);
        return;
    }

    SPDLOG_DEBUG("tcamprop1 information for '{}' found. Overwriting category of '{}' with '{}'.",
                 tcamprop1_name,
                 gc_node_info.iccategory,
                 static_info.info_ptr->iccategory);

    gc_node_info.iccategory = static_info.info_ptr->iccategory;

    if (gc_node_info.description.empty())
        gc_node_info.description = static_info.info_ptr->description;
    if (gc_node_info.display_name.empty())
        gc_node_info.display_name = static_info.info_ptr->display_name;

    if (static_info.type != gc_node_type)
    {
        if (tcamprop1_name != "FocusAuto" && tcamprop1_name != "IrisAuto")  // skip these properties from warning about the type difference
        {

            SPDLOG_WARN("{} '{}' type != tcamprop1 type of '{}'.",
                        tcamprop1::to_string(gc_node_type),
                        tcamprop1_name,
                        tcamprop1::to_string(static_info.type));
        }
    }
}


auto to_IntRepresentation(ArvGcRepresentation rep) noexcept
{
    switch (rep)
    {
        case ARV_GC_REPRESENTATION_UNDEFINED:
            return tcamprop1 ::IntRepresentation_t::Linear;
        case ARV_GC_REPRESENTATION_LINEAR:
            return tcamprop1 ::IntRepresentation_t::Linear;
        case ARV_GC_REPRESENTATION_LOGARITHMIC:
            return tcamprop1 ::IntRepresentation_t::Logarithmic;
        case ARV_GC_REPRESENTATION_BOOLEAN:
            return tcamprop1 ::IntRepresentation_t::Boolean;
        case ARV_GC_REPRESENTATION_PURE_NUMBER:
            return tcamprop1 ::IntRepresentation_t::PureNumber;
        case ARV_GC_REPRESENTATION_HEX_NUMBER:
            return tcamprop1 ::IntRepresentation_t::HexNumber;
        case ARV_GC_REPRESENTATION_IPV4_ADDRESS:
            return tcamprop1 ::IntRepresentation_t::IPV4Address;
        case ARV_GC_REPRESENTATION_MAC_ADDRESS:
            return tcamprop1 ::IntRepresentation_t::MACAddress;
    }
    return tcamprop1 ::IntRepresentation_t::Linear;
}

auto to_FloatRepresentation(ArvGcRepresentation rep) noexcept
{
    switch (rep)
    {
        case ARV_GC_REPRESENTATION_UNDEFINED:
            return tcamprop1 ::FloatRepresentation_t::Linear;
        case ARV_GC_REPRESENTATION_LINEAR:
            return tcamprop1 ::FloatRepresentation_t::Linear;
        case ARV_GC_REPRESENTATION_LOGARITHMIC:
            return tcamprop1 ::FloatRepresentation_t::Logarithmic;
        case ARV_GC_REPRESENTATION_PURE_NUMBER:
            return tcamprop1 ::FloatRepresentation_t::PureNumber;
        default:
            return tcamprop1 ::FloatRepresentation_t::Linear;
    }
    return tcamprop1 ::FloatRepresentation_t::Linear;
}


tcam::property::PropertyFlags arv_gc_get_tcam_flags(ArvGcFeatureNode* node,
                                                    tcamprop1::Access_t access_mode) noexcept
{
    tcam::property::PropertyFlags flags = tcam::property::PropertyFlags::None;

    GError* err = nullptr;
    bool ret_avail = arv_gc_feature_node_is_available(node, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
        g_clear_error(&err);
    }
    else
    {
        if (ret_avail)
        {
            flags |= tcam::property::PropertyFlags::Available;
        }
    }

    bool ret_implemented = arv_gc_feature_node_is_implemented(node, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
        g_clear_error(&err);
    }
    else
    {
        if (ret_implemented)
        {
            flags |= tcam::property::PropertyFlags::Implemented;
        }
    }

    if (access_mode == tcamprop1::Access_t::RO)
    {
        flags |= tcam::property::PropertyFlags::Locked;
    }
    else
    {
        bool ret_locked = arv_gc_feature_node_is_locked(node, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
            g_clear_error(&err);
        }
        else
        {
            if (ret_locked)
            {
                flags |= tcam::property::PropertyFlags::Locked;
            }
        }
    }

    return flags;
}
} // namespace

tcam::property::PropertyFlags prop_base_impl::get_flags_impl() const
{
    aravis_backend_guard lck = acquire_backend_guard();
    if (!lck)
    {
        return tcam::property::PropertyFlags::None;
    }
    assert(feature_node_);
    return arv_gc_get_tcam_flags(feature_node_, access_mode_);
}

aravis_backend_guard prop_base_impl::acquire_backend_guard() const noexcept
{
    return aravis_backend_guard { backend_ };
}

tcamprop1::prop_static_info_str prop_base_impl::build_static_info(
    std::string_view category,
    std::string_view name_override) const noexcept
{
    auto res = get_static_feature_node_info(feature_node_);
    if (!name_override.empty())
        res.name = name_override;
    res.iccategory = category;
    return res;
}

prop_base_impl::prop_base_impl(const std::shared_ptr<AravisPropertyBackend>& cam,
                               ArvGcFeatureNode* feature_node)
    : backend_ { cam }, feature_node_ { feature_node }
{
    access_mode_ = to_Access(arv_gc_feature_node_get_actual_access_mode(feature_node_));
}

AravisPropertyIntegerImpl::AravisPropertyIntegerImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_ { ARV_GC_INTEGER(node) }
{
    static_info_ = build_static_info(category, name);

    unit_ = to_stdstring(arv_gc_integer_get_unit(arv_gc_node_));
    int_rep_ = to_IntRepresentation(arv_gc_integer_get_representation(arv_gc_node_));

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::Integer);
}

outcome::result<int64_t> AravisPropertyIntegerImpl::get_value() const
{
    aravis_backend_guard lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }

    GError* err = nullptr;
    auto rval = arv_gc_integer_get_value(arv_gc_node_, &err);
    if (err)
        return consume_GError(err);
    return rval;
}

outcome::result<void> AravisPropertyIntegerImpl::set_value(int64_t new_value)
{
    aravis_backend_guard lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }

    GError* err = nullptr;
    arv_gc_integer_set_value(arv_gc_node_, new_value, &err);
    if (err)
        return consume_GError(err);
    return outcome::success();
}

tcamprop1::prop_range_integer AravisPropertyIntegerImpl::get_range() const
{
    aravis_backend_guard lck = acquire_backend_guard();
    if (!lck)
    {
        return {};
    }

    tcamprop1::prop_range_integer range = {};
    GError* err = nullptr;
    range.stp = arv_gc_integer_get_inc(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_TRACE("arv_gc_integer_get_inc for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
        range.stp = 1;
    }
    if (range.stp == 0)
        range.stp = 1;
    range.min = arv_gc_integer_get_min(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_ERROR("arv_gc_integer_get_min for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
    }
    range.max = arv_gc_integer_get_max(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_ERROR("arv_gc_integer_get_max for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
    }
    return range;
}

outcome::result<int64_t> AravisPropertyIntegerImpl::get_default() const
{
    return tcam::status::PropertyNoDefaultAvailable;
}

AravisPropertyDoubleImpl::AravisPropertyDoubleImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_(ARV_GC_FLOAT(node))
{
    static_info_ = build_static_info(category, name);

    unit_ = to_stdstring(arv_gc_float_get_unit(arv_gc_node_));
    float_rep_ = to_FloatRepresentation(arv_gc_float_get_representation(arv_gc_node_));

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::Float);
}

outcome::result<void> AravisPropertyDoubleImpl::set_value(double new_value)
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    arv_gc_float_set_value(arv_gc_node_, new_value, &err);
    if (err)
        return consume_GError(err);
    return outcome::success();
}

outcome::result<double> AravisPropertyDoubleImpl::get_value() const
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    auto ret = arv_gc_float_get_value(arv_gc_node_, &err);
    if (err)
        return consume_GError(err);
    return ret;
}

tcamprop1::prop_range_float AravisPropertyDoubleImpl::get_range() const
{
    aravis_backend_guard lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return {};
    }

    tcamprop1::prop_range_float range = {};

    GError* err = nullptr;
    range.min = arv_gc_float_get_min(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_ERROR("arv_gc_float_get_min for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
    }
    range.max = arv_gc_float_get_max(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_ERROR("arv_gc_float_get_max for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
    }
    range.stp = arv_gc_float_get_inc(arv_gc_node_, &err);
    if (err)
    {
        SPDLOG_TRACE("arv_gc_float_get_inc for '{}': {}", get_name(), err->message);
        g_clear_error(&err);
    }
    return range;
}

AravisPropertyBoolImpl::AravisPropertyBoolImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_(ARV_GC_BOOLEAN(node))
{
    static_info_ = build_static_info(category, name);

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::Boolean);
}

outcome::result<bool> AravisPropertyBoolImpl::get_value() const
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    auto ret = arv_gc_boolean_get_value(arv_gc_node_, &err);
    if (err)
        return consume_GError(err);
    return ret;
}


outcome::result<void> AravisPropertyBoolImpl::set_value(bool new_value)
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    arv_gc_boolean_set_value(arv_gc_node_, new_value, &err);
    if (err)
        return consume_GError(err);
    return outcome::success();
}


AravisPropertyCommandImpl::AravisPropertyCommandImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_(ARV_GC_COMMAND(node))
{
    static_info_ = build_static_info(category, name);

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::Command);
}

outcome::result<void> AravisPropertyCommandImpl::execute()
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    arv_gc_command_execute(arv_gc_node_, &err);
    return consume_GError(err);
}


AravisPropertyEnumImpl::AravisPropertyEnumImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_(ARV_GC_ENUMERATION(node))
{
    static_info_ = build_static_info(category, name);

    GError* err = nullptr;
    auto entries = arv_gc_enumeration_get_entries(arv_gc_node_);
    for (auto entry = entries; entry != nullptr; entry = entry->next)
    {
        auto ptr = ARV_GC_ENUM_ENTRY(entry->data);
        auto value = arv_gc_enum_entry_get_value(ptr, &err);
        if (err)
        {
            SPDLOG_ERROR(
                "Failed to retrieve enum-enry value in '{}'. Error: {}.", get_name(), err->message);
            g_clear_error(&err);
            continue;
        }

        auto entry_name =
            to_stdstring(arv_gc_feature_node_get_display_name(ARV_GC_FEATURE_NODE(ptr)));
        if (entry_name.empty())
        {
            entry_name = to_stdstring(arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(ptr)));
        }
        entries_.push_back(enum_entry { entry_name, value });
    }

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::Enumeration);
}

outcome::result<std::string_view> AravisPropertyEnumImpl::get_default() const
{
    return tcam::status::PropertyNoDefaultAvailable;
}

outcome::result<void> AravisPropertyEnumImpl::set_value(std::string_view new_value)
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    for (auto& e : entries_)
    {
        if (e.display_name == new_value)
        {
            GError* err = nullptr;
            arv_gc_enumeration_set_int_value(arv_gc_node_, e.value, &err);
            if (err)
                return consume_GError(err);
            return outcome::success();
        }
    }
    return tcam::status::PropertyValueOutOfBounds;
}

outcome::result<std::string_view> AravisPropertyEnumImpl::get_value() const
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    auto current_value = arv_gc_enumeration_get_int_value(arv_gc_node_, &err);
    if (err)
        return consume_GError(err);

    for (auto& e : entries_)
    {
        if (e.value == current_value)
            return e.display_name;
    }
    return tcam::status::PropertyValueOutOfBounds;
}

AravisPropertyStringImpl::AravisPropertyStringImpl(
    std::string_view name,
    std::string_view category,
    ArvGcNode* node,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : prop_base_impl(backend, ARV_GC_FEATURE_NODE(node)), arv_gc_node_(ARV_GC_STRING(node))
{
    static_info_ = build_static_info(category, name);

    update_with_tcamprop1_static_info(name, static_info_, tcamprop1::prop_type::String);
}

std::error_code AravisPropertyStringImpl::set_value(std::string_view new_value)
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    arv_gc_string_set_value(arv_gc_node_, std::string { new_value }.c_str(), &err);
    if (err)
        return consume_GError(err);
    return {};
}

outcome::result<std::string> AravisPropertyStringImpl::get_value() const
{
    auto lck = acquire_backend_guard();
    if (!lck)
    {
        SPDLOG_ERROR("Unable to lock backend.");
        return tcam::status::ResourceNotLockable;
    }
    GError* err = nullptr;
    auto current_value = arv_gc_string_get_value(arv_gc_node_, &err);
    if (err)
        return consume_GError(err);

    return std::string { current_value };
}

balance_ratio_raw_to_wb_channel::balance_ratio_raw_to_wb_channel(
    const std::shared_ptr<IPropertyEnum>& sel,
    const std::shared_ptr<IPropertyInteger>& val,
    const std::string& sel_entry,
    const tcamprop1::prop_static_info_float* info_entry,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : selector_(sel), value_(val), selector_entry_(sel_entry), prop_entry_(info_entry),
      backend_(backend)
{
}

outcome::result<double> balance_ratio_raw_to_wb_channel::get_value() const
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return tcam::status::ResourceNotLockable;

    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        return sel_res.error();
    }
    auto res = value_->get_value();
    if (!res)
    {
        return res.error();
    }
    return res.value() / 64.0;
}

outcome::result<void> balance_ratio_raw_to_wb_channel::set_value(double new_value)
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return tcam::status::ResourceNotLockable;

    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        return sel_res.error();
    }
    return value_->set_value(new_value * 64.);
}

balance_ratio_to_wb_channel::balance_ratio_to_wb_channel(
    const std::shared_ptr<IPropertyEnum>& sel,
    const std::shared_ptr<IPropertyFloat>& val,
    const std::string& sel_entry,
    const tcamprop1::prop_static_info_float* info_entry,
    const std::shared_ptr<AravisPropertyBackend>& backend)
    : selector_(sel), value_(val), selector_entry_(sel_entry), prop_entry_(info_entry),
      backend_(backend)
{
}

tcam::property::PropertyFlags balance_ratio_to_wb_channel::get_flags() const
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return PropertyFlags::None;
    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        // do nothing
        //return sel_res.error();
    }
    return value_->get_flags();
}

tcamprop1::prop_range_float balance_ratio_to_wb_channel::get_range() const
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return {};
    // We are not absolutely sure what the range is, we ask the control (e.g. cameras could provide a factor > 4.0
    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        // do nothing
        //return sel_res.error();
    }
    return value_->get_range();
}

outcome::result<double> balance_ratio_to_wb_channel::get_value() const
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return tcam::status::ResourceNotLockable;
    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        return sel_res.error();
    }
    auto res = value_->get_value();
    if (!res)
    {
        return res.error();
    }
    return res.value();
}

outcome::result<void> balance_ratio_to_wb_channel::set_value(double new_value)
{
    auto guard = aravis_backend_guard::acquire(backend_);
    if (!guard)
        return tcam::status::ResourceNotLockable;
    if (auto sel_res = selector_->set_value(selector_entry_); !sel_res)
    {
        return sel_res.error();
    }
    return value_->set_value(new_value);
}

focus_auto_enum_override::focus_auto_enum_override(
    const std::shared_ptr<IPropertyCommand>& property_to_override,
    const std::shared_ptr<AravisPropertyBackend>& /*backend*/)
    : property_to_override_(property_to_override)
{
}

tcamprop1::prop_static_info focus_auto_enum_override::get_static_info() const
{
    return tcamprop1::prop_list::FocusAuto;
}

tcam::property::PropertyFlags focus_auto_enum_override::get_flags() const
{
    return property_to_override_->get_flags();
}

outcome::result<void> focus_auto_enum_override::set_value(std::string_view new_value)
{
    if (new_value == "Once")
        return property_to_override_->execute();
    return outcome::success();
}

iris_auto_enum_override::iris_auto_enum_override(
    const std::shared_ptr<IPropertyBool>& property_to_override,
    const std::shared_ptr<AravisPropertyBackend>& /*backend*/)
    : property_to_override_(property_to_override)
{
}

tcamprop1::prop_static_info iris_auto_enum_override::get_static_info() const
{
    return property_to_override_->get_static_info();
}

tcam::property::PropertyFlags iris_auto_enum_override::get_flags() const
{
    return property_to_override_->get_flags();
}

outcome::result<void> iris_auto_enum_override::set_value(std::string_view new_value)
{
    if (new_value == "Off") {
        return property_to_override_->set_value(false);
    }
    else if (new_value == "Continuous")
    {
        return property_to_override_->set_value(true);
    }
    return tcam::status::PropertyValueOutOfBounds;
}

outcome::result<std::string_view> iris_auto_enum_override::get_value() const
{
    auto rval = property_to_override_->get_value();
    if (!rval.has_value())
        return rval.error();
    
    auto val = rval.value();
    if (val)
        return "Continuous";
    else
        return "Off";
}

outcome::result<std::string_view> iris_auto_enum_override::get_default() const
{
    return "Off";
}

std::vector<std::string> iris_auto_enum_override::get_entries() const
{
    return std::vector<std::string> { "Off", "Continuous" };
}
