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

#include "../../../libs/tcamprop/src/tcam-property-1.0.h"
#include "../../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_info_list.h"
#include "../../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_info.h"
#include "../../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_interface.h"

#include "../../PropertyInterfaces.h"

#include <memory>
#include <algorithm>

namespace tcam::mainsrc
{

template<class TBase>
struct TcamPropertyBase : TBase
{
    TcamPropertyBase (std::shared_ptr<tcam::property::IPropertyBase> prop)
        : m_prop(prop)
    {}

    std::shared_ptr<tcam::property::IPropertyBase> m_prop;

    virtual auto get_property_name() const noexcept  -> std::string_view final { return m_prop->get_name(); }
    virtual auto get_property_info() const noexcept  -> tcamprop1::prop_static_info final
    {
        tcamprop1::prop_static_info ret;
        ret.name = m_prop->get_name();
        ret.iccategory = m_prop->get_category();
        ret.display_name = m_prop->get_display_name();
        ret.description = m_prop->get_description();

        ret.visibility = tcamprop1::Visibility_t::Beginner;

        return ret;
    }

    virtual auto get_property_state (uint32_t /*flags = 0*/) -> outcome::result<tcamprop1::prop_state> final
    {
        tcamprop1::prop_state ret = {};

        auto flags = m_prop->get_flags();

        ret.is_locked = tcam::property::is_locked(flags);

        return ret;
    }
};


struct TcamPropertyInteger : TcamPropertyBase<tcamprop1::property_interface_integer>
{
    TcamPropertyInteger ( std::shared_ptr<tcam::property::IPropertyBase> prop )
        : TcamPropertyBase{ prop }
    {}

    virtual auto get_property_range (uint32_t /* flags = 0 */) -> outcome::result<tcamprop1::prop_range_integer> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        tcamprop1::prop_range_integer ret = { tmp->get_min(), tmp->get_max(), tmp->get_step() };

        return ret;
    }

    virtual auto get_property_default (uint32_t /* flags = 0 */) -> outcome::result<int64_t> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value (uint32_t /*flags*/) -> outcome::result<int64_t> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        return tmp->get_value();
    }
    virtual auto set_property_value( int64_t value, uint32_t /*flags*/ )->std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcam::status::Success;
        }

        return tcam::status::UndefinedError;
    }

    virtual auto get_representation() const noexcept -> tcamprop1::IntRepresentation_t final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());
        return tmp->get_representation();
    }

    virtual auto get_unit() const noexcept -> std::string_view final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());
        return tmp->get_unit();
    }
};


struct TcamPropertyFloat : TcamPropertyBase<tcamprop1::property_interface_float>
{
    TcamPropertyFloat ( std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase{ prop }
    {}

    virtual auto get_property_range (uint32_t /* flags = 0 */) -> outcome::result<tcamprop1::prop_range_float> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        tcamprop1::prop_range_float ret = { tmp->get_min(), tmp->get_max(), tmp->get_step() };

        return ret;
    }

    virtual auto get_property_default (uint32_t /* flags = 0 */) -> outcome::result<double> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value( uint32_t /*flags*/ ) -> outcome::result<double> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value( double value, uint32_t /*flags*/ )->std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcam::status::Success;
        }

        return tcam::status::UndefinedError;
    }

    virtual auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t final
    {
        return tcamprop1::FloatRepresentation_t::PureNumber;
    }

    virtual auto get_unit() const noexcept -> std::string_view final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());
        return tmp->get_unit();
    }

};


struct TcamPropertyBoolean : TcamPropertyBase<tcamprop1::property_interface_boolean>
{
    TcamPropertyBoolean( std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase{ prop }
    {}

    virtual auto get_property_default (uint32_t /* flags = 0 */) -> outcome::result<bool> final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value( uint32_t /*flags*/ ) -> outcome::result<bool> final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value( bool value, uint32_t /*flags*/ )->std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcam::status::Success;
        }

        return tcam::status::UndefinedError;
    }
};


struct TcamPropertyEnumeration : TcamPropertyBase<tcamprop1::property_interface_enumeration>
{
    TcamPropertyEnumeration( std::shared_ptr<tcam::property::IPropertyBase> prop )
        : TcamPropertyBase{ prop }
    {}

    virtual auto get_property_range (uint32_t /* flags = 0 */) -> outcome::result<tcamprop1::prop_range_enumeration> final
    {
        tcamprop1::prop_range_enumeration range;

        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        range.enum_entries = tmp->get_entries();
        return range;
    }

    virtual auto get_property_default (uint32_t /*flags = 0 */) -> outcome::result<std::string_view> final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        auto entries = tmp->get_entries();

        auto it = std::find(entries.begin(), entries.end(), tmp->get_default());

        size_t index = 0;

        if (it != entries.end())
        {
            index = it - entries.begin();
        }

        return entries.at(index);
    }

    virtual auto get_property_value( uint32_t /*flags*/ ) -> outcome::result<std::string_view> final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value( std::string_view value, uint32_t /*flags*/ )->std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        auto ret = tmp->set_value_str(value);

        if (ret)
        {
            return tcam::status::Success;
        }

        return tcam::status::UndefinedError;
    }
};


struct TcamPropertyCommand : TcamPropertyBase<tcamprop1::property_interface_command>
{
    TcamPropertyCommand(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase {prop}
    {}

    auto execute_command( uint32_t /* flags */ )->std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyCommand*>(m_prop.get());
        auto ret = tmp->execute();

        if (ret)
        {
            return tcam::status::Success;
        }

        return tcam::status::UndefinedError;
    }
};


void gst_tcam_mainsrc_tcamprop_init (TcamPropertyProviderInterface* iface);

} // namespace tcam::mainsrc
