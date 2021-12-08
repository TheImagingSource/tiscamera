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

class V4L2PropertyBackend;

class V4L2PropertyImplBase : public PropertyLock
{
protected:
    V4L2PropertyImplBase(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyImplBase(const v4l2_queryctrl& queryctrl,
                         const tcamprop1::prop_static_info* static_info,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);

    outcome::result<void> set_backend_value(int64_t new_value);
    outcome::result<int64_t> get_backend_value() const;

    outcome::result<void> set_backend_value(uint32_t ctrl_id, int64_t new_value);
    outcome::result<int64_t> get_backend_value(uint32_t ctrl_id) const;

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

    const std::string& get_internal_name() const noexcept
    {
        return name_;
    }
    tcamprop1::prop_static_info get_static_info_internal() const
    {
        if (p_static_info_base)
        {
            return *p_static_info_base;
        }
        return tcamprop1::prop_static_info { /*.name =*/name_, {}, {}, {} };
    }
    auto get_dependency_entry() const
    {
        return dependency_info_;
    }

private:
    std::string name_;

    uint32_t v4l2_id_ = 0;

    std::weak_ptr<V4L2PropertyBackend> m_cam;

    const tcamprop1::prop_static_info* p_static_info_base = nullptr;

    std::vector<std::weak_ptr<PropertyLock>> dependent_controls_;

    const tcam::property::dependency_entry* dependency_info_ = nullptr;
};

class V4L2PropertyIntegerImpl : public V4L2PropertyImplBase, public IPropertyInteger
{
public:
    V4L2PropertyIntegerImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend);

    V4L2PropertyIntegerImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend,
                            const tcamprop1::prop_static_info_integer* static_info,
                            tcam::v4l2::converter_scale scale);

    tcamprop1::prop_static_info get_static_info() const final
    {
        return get_static_info_internal();
    }

    std::string_view get_unit() const final;
    tcamprop1::IntRepresentation_t get_representation() const final;

    PropertyFlags get_flags() const final
    {
        return m_flags;
    }

    tcamprop1::prop_range_integer get_range() const final
    {
        return range_;
    }
    int64_t get_default() const final
    {
        return m_default;
    }

    outcome::result<int64_t> get_value() const final;
    outcome::result<void> set_value(int64_t new_value) final;

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

private:
    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;

    tcamprop1::prop_range_integer range_;
    int64_t m_default = 0;

    tcam::v4l2::converter_scale m_converter;
    const tcamprop1::prop_static_info_integer* p_static_info = nullptr;
};

class V4L2PropertyDoubleImpl : public V4L2PropertyImplBase, public IPropertyFloat
{
public:
    V4L2PropertyDoubleImpl(const v4l2_queryctrl& queryctrl,
                           const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyDoubleImpl(const v4l2_queryctrl& queryctrl,
                           const std::shared_ptr<V4L2PropertyBackend>& backend,
                           const tcamprop1::prop_static_info_float* static_info,
                           tcam::v4l2::converter_scale scale);

    tcamprop1::prop_static_info get_static_info() const final
    {
        return get_static_info_internal();
    }
    std::string_view get_unit() const final;
    tcamprop1::FloatRepresentation_t get_representation() const final;

    PropertyFlags get_flags() const final
    {
        return m_flags;
    }

    tcamprop1::prop_range_float get_range() const final
    {
        return range_;
    }

    double get_default() const final
    {
        return m_default;
    }
    outcome::result<double> get_value() const override;
    outcome::result<void> set_value(double new_value) override;

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

private:
    tcam::v4l2::converter_scale m_converter;

    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;

    tcamprop1::prop_range_float range_;

    double m_default;

    const tcamprop1::prop_static_info_float* p_static_info = nullptr;
};

class V4L2PropertyBoolImpl : public V4L2PropertyImplBase, public IPropertyBool
{
public:
    V4L2PropertyBoolImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyBoolImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend,
                         const tcamprop1::prop_static_info_boolean* static_info);


    tcamprop1::prop_static_info get_static_info() const final
    {
        return get_static_info_internal();
    }

    PropertyFlags get_flags() const final
    {
        return m_flags;
    }

    bool get_default() const final
    {
        return m_default;
    }
    outcome::result<bool> get_value() const override;
    outcome::result<void> set_value(bool new_value) override;

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

    // currently unused
    //bool should_set_dependent_locked() const override
    //{
    //    return get_value().value();
    //}

private:
    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;

    bool m_default = false;

    const tcamprop1::prop_static_info_boolean* p_static_info = nullptr;
};


class V4L2PropertyCommandImpl : public V4L2PropertyImplBase, public IPropertyCommand
{
public:
    V4L2PropertyCommandImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend);
    V4L2PropertyCommandImpl(const v4l2_queryctrl& queryctrl,
                            const std::shared_ptr<V4L2PropertyBackend>& backend,
                            const tcamprop1::prop_static_info_command* static_info);

    tcamprop1::prop_static_info get_static_info() const final
    {
        return get_static_info_internal();
    }

    PropertyFlags get_flags() const final
    {
        return m_flags;
    }

    outcome::result<void> execute() final;

    void set_locked(bool new_locked_state) final
    {
        lock(m_flags, new_locked_state);
    }

private:
    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;

    const tcamprop1::prop_static_info_command* p_static_info = nullptr;
};


class V4L2PropertyEnumImpl : public V4L2PropertyImplBase, public IPropertyEnum
{
public:
    V4L2PropertyEnumImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend);

    V4L2PropertyEnumImpl(const v4l2_queryctrl& queryctrl,
                         const std::shared_ptr<V4L2PropertyBackend>& backend,
                         const tcamprop1::prop_static_info_enumeration* static_info,
                         tcam::v4l2::fetch_menu_entries_func func);

    tcamprop1::prop_static_info get_static_info() const final
    {
        return get_static_info_internal();
    }


    PropertyFlags get_flags() const override
    {
        return m_flags;
    }

    outcome::result<void> set_value_str(const std::string_view& new_value) override;
    outcome::result<std::string_view> get_value() const override;

    std::string get_default() const override
    {
        return m_default;
    }

    std::vector<std::string> get_entries() const override;

    void set_locked(bool new_locked_state) final
    {
        lock(m_flags, new_locked_state);
    }
    bool should_set_dependent_locked() const final;
private:
    std::string_view get_entry_name(int value) const;
    outcome::result<int64_t> get_entry_value(std::string_view name) const;

    std::vector<tcam::v4l2::menu_entry> m_entries;

    PropertyFlags m_flags = PropertyFlags::Available | PropertyFlags::Implemented;

    std::string m_default;

    const tcamprop1::prop_static_info_enumeration* p_static_info = nullptr;
};

class prop_impl_33U_balance_white_auto : public V4L2PropertyEnumImpl
{
public:
    prop_impl_33U_balance_white_auto(const v4l2_queryctrl& queryctrl_balance_white_auto,
                                     const std::shared_ptr<V4L2PropertyBackend>& backend);

    outcome::result<void> set_value_str(const std::string_view& new_value) override;
};


} // namespace tcam::v4l2
