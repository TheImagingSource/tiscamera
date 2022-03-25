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

#pragma once

#include "../PropertyInterfaces.h"
#include "../VideoFormat.h"
#include "../error.h"
#include "../property_dependencies.h"
#include "v4l2_genicam_conversion.h"

#include <linux/videodev2.h>
#include <memory>
#include <string>
#include <tcamprop1.0_base/tcamprop_property_info.h>

namespace tcam::v4l2
{
using tcam::property::IPropertyBase;
using tcam::property::IPropertyBool;
using tcam::property::IPropertyCommand;
using tcam::property::IPropertyEnum;
using tcam::property::IPropertyFloat;
using tcam::property::IPropertyInteger;
using tcam::property::PropertyFlags;
using tcam::property::PropertyLock;

using tcam::property::IPropertyFloat2;
using tcam::property::IPropertyInteger2;

class V4L2PropertyBackend;

class V4L2PropertyBackendWrapper
{
public:
    V4L2PropertyBackendWrapper(const v4l2_queryctrl& queryctrl,
                               const std::shared_ptr<V4L2PropertyBackend>& backend)
        : v4l2_id_(queryctrl.id), device_ptr_(backend)
    {
    }

    outcome::result<void> set_backend_value(int64_t new_value);
    outcome::result<int64_t> get_backend_value() const;

    outcome::result<void> set_backend_value(uint32_t ctrl_id, int64_t new_value);
    outcome::result<int64_t> get_backend_value(uint32_t ctrl_id) const;

private:
    uint32_t v4l2_id_ = 0;

    std::weak_ptr<V4L2PropertyBackend> device_ptr_;
};

class V4L2PropertyLockImpl : public PropertyLock
{
protected:
    V4L2PropertyLockImpl(std::string_view name);

    /**
     * Function called when this property wants to know if dependent properties should be locked.
     */
    virtual bool should_set_dependent_locked() const
    {
        return false;
    }
    void set_dependent_properties(std::vector<std::weak_ptr<PropertyLock>>&& controls) override;
    auto get_dependent_names() const -> std::vector<std::string_view> override;

protected:
    void update_dependent_lock_state();

    auto get_dependency_entry() const noexcept
    {
        return dependency_info_;
    }

private:
    std::vector<std::weak_ptr<PropertyLock>> dependent_controls_;

    const tcam::property::dependency_entry* dependency_info_ = nullptr;
};

template<class TPropertyInterface>
class V4L2PropertyImplBase : public V4L2PropertyLockImpl, public TPropertyInterface
{
public:
    tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info_base)
        {
            return *p_static_info_base;
        }
        return tcamprop1::prop_static_info { /*.name =*/get_internal_name(), {}, {}, {} };
    }

    PropertyFlags get_flags() const final
    {
        return m_flags;
    }
    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

protected:
    V4L2PropertyImplBase(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend)
        : V4L2PropertyLockImpl((const char*)queryctrl.name), backend_(queryctrl, backend),
          name_((const char*)queryctrl.name)
    {
    }
    V4L2PropertyImplBase(const v4l2_queryctrl& queryctrl,
                         const tcamprop1::prop_static_info* static_info,
                         const std::shared_ptr<V4L2PropertyBackend>& backend)
        : V4L2PropertyLockImpl(static_info->name), backend_(queryctrl, backend),
          name_(static_info->name), p_static_info_base(static_info)
    {
    }

    std::string_view get_internal_name() const noexcept
    {
        return name_;
    }

protected:
    V4L2PropertyBackendWrapper backend_;

private:
    std::string name_;

    const tcamprop1::prop_static_info* p_static_info_base = nullptr;

    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;
};

class V4L2PropertyIntegerImpl : public V4L2PropertyImplBase<IPropertyInteger>
{
public:
    V4L2PropertyIntegerImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend);

    V4L2PropertyIntegerImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend,
                            const tcamprop1::prop_static_info_integer* static_info,
                            const tcam::v4l2::converter_scale_init_integer& scale);

    std::string_view get_unit() const final;
    tcamprop1::IntRepresentation_t get_representation() const final;

    tcamprop1::prop_range_integer get_range() const final
    {
        return range_;
    }
    outcome::result<int64_t> get_default() const final
    {
        return default_;
    }

    outcome::result<int64_t> get_value() const final;
    outcome::result<void> set_value(int64_t new_value) final;

private:
    tcamprop1::prop_range_integer range_;
    int64_t default_ = 0;

    tcam::v4l2::converter_scale m_converter;
    const tcamprop1::prop_static_info_integer* p_static_info = nullptr;
};

class V4L2PropertyDoubleImpl : public V4L2PropertyImplBase<IPropertyFloat>
{
public:
    V4L2PropertyDoubleImpl(const v4l2_queryctrl& queryctrl,
                           const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyDoubleImpl(const v4l2_queryctrl& queryctrl,
                           const std::shared_ptr<V4L2PropertyBackend>& backend,
                           const tcamprop1::prop_static_info_float* static_info,
                           const tcam::v4l2::converter_scale_init_float& scale);

    std::string_view get_unit() const final;
    tcamprop1::FloatRepresentation_t get_representation() const final;

    tcamprop1::prop_range_float get_range() const final
    {
        return range_;
    }

    outcome::result<double> get_default() const final
    {
        return default_;
    }
    outcome::result<double> get_value() const override;
    outcome::result<void> set_value(double new_value) override;

private:
    tcam::v4l2::converter_scale converter_;

    tcamprop1::prop_range_float range_;

    double default_ = 0.;

    const tcamprop1::prop_static_info_float* p_static_info = nullptr;
};

class V4L2PropertyBoolImpl : public V4L2PropertyImplBase<IPropertyBool>
{
public:
    V4L2PropertyBoolImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyBoolImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend,
                         const tcamprop1::prop_static_info_boolean* static_info);

    outcome::result<bool> get_default() const final
    {
        return m_default;
    }
    outcome::result<bool> get_value() const override;
    outcome::result<void> set_value(bool new_value) override;

private:
    bool m_default = false;

    //const tcamprop1::prop_static_info_boolean* p_static_info = nullptr;
};


class V4L2PropertyCommandImpl : public V4L2PropertyImplBase<IPropertyCommand>
{
public:
    V4L2PropertyCommandImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyCommandImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend,
                            const tcamprop1::prop_static_info_command* static_info);

    outcome::result<void> execute() override;

private:
    //const tcamprop1::prop_static_info_command* p_static_info = nullptr;
};


class V4L2PropertyEnumImpl : public V4L2PropertyImplBase<IPropertyEnum>
{
public:
    V4L2PropertyEnumImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);

    V4L2PropertyEnumImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend,
                         const tcamprop1::prop_static_info_enumeration* static_info,
                         tcam::v4l2::fetch_menu_entries_func func);

    outcome::result<void> set_value(std::string_view new_value) override;
    outcome::result<std::string_view> get_value() const override;

    outcome::result<std::string_view> get_default() const final
    {
        return m_default;
    }

    std::vector<std::string> get_entries() const override;

    bool should_set_dependent_locked() const final;

private:
    std::string_view get_entry_name(int value) const;
    outcome::result<int64_t> get_entry_value(std::string_view name) const;

    std::vector<tcam::v4l2::menu_entry> m_entries;

    std::string m_default;

    const tcamprop1::prop_static_info_enumeration* p_static_info = nullptr;
};

class prop_impl_33U_balance_white_auto : public V4L2PropertyEnumImpl
{
public:
    prop_impl_33U_balance_white_auto(const v4l2_queryctrl& queryctrl_balance_white_auto,
                                     const std::shared_ptr<V4L2PropertyBackend>& backend);

    outcome::result<void> set_value(std::string_view new_value) override;
};

class prop_impl_sensor_dim : public IPropertyInteger2
{
public:
    prop_impl_sensor_dim(const tcamprop1::prop_static_info_integer* prop, int64_t value)
        : static_info_ { prop }, value_(value)
    {
    }

    tcamprop1::prop_static_info_integer get_static_info_ext() const final
    {
        return *static_info_;
    }

    PropertyFlags get_flags() const final
    {
        return PropertyFlags::Implemented | PropertyFlags::Available | PropertyFlags::Locked;
    }
    tcamprop1::prop_range_integer get_range() const final
    {
        return { value_, value_, 1 };
    }
    outcome::result<int64_t> get_default() const final
    {
        return value_;
    }
    outcome::result<int64_t> get_value() const final
    {
        return value_;
    }
    outcome::result<void> set_value(int64_t /*new_value*/) final
    {
        return tcam::status::PropertyNotWriteable;
    }

private:
    const tcamprop1::prop_static_info_integer* static_info_ = nullptr;

    int64_t value_ = 0;
};


class prop_impl_offset_auto_center : public IPropertyEnum, public V4L2PropertyLockImpl
{
public:
    prop_impl_offset_auto_center(const std::shared_ptr<IPropertyInteger>& offset_x,
                                 const std::shared_ptr<IPropertyInteger>& offset_y,
                                 tcam_image_size dim);

    static auto create_if_needed(const std::vector<std::shared_ptr<IPropertyBase>>& properties,
                                 tcam_image_size sensor_dim)
        -> std::shared_ptr<prop_impl_offset_auto_center>;

    tcamprop1::prop_static_info get_static_info() const final;

    PropertyFlags get_flags() const final
    {
        return PropertyFlags::Implemented | PropertyFlags::Available;
    }
    outcome::result<void> set_value(std::string_view new_value) final;

    outcome::result<std::string_view> get_value() const final
    {
        return enabled_ ? "On" : "Off";
    }

    outcome::result<std::string_view> get_default() const final
    {
        return "On";
    }

    std::vector<std::string> get_entries() const final
    {
        return { "Off", "On" };
    }

    bool should_set_dependent_locked() const final
    {
        return enabled_;
    }

    void set_locked(bool /*new_locked_state*/) final
    {
        // we don't really need to do anything here
    }


    void set_format(const tcam::VideoFormat& current_fmt);

private:
    const tcamprop1::prop_static_info_integer* static_info_ = nullptr;

    bool enabled_ = true;

    tcam_image_size sensor_dim_ = {};

    std::shared_ptr<IPropertyInteger> prop_offset_x_;
    std::shared_ptr<IPropertyInteger> prop_offset_y_;

    tcam::VideoFormat current_format_;

    void update_offsets();
};
} // namespace tcam::v4l2
