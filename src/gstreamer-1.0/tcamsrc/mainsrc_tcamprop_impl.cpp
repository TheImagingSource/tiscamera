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

#include "mainsrc_tcamprop_impl.h"

#include "gsttcammainsrc.h"
#include "mainsrc_device_state.h"

#include <algorithm>

namespace tcam::mainsrc
{

template<class TBase> struct TcamPropertyBase : TBase
{
    TcamPropertyBase(std::shared_ptr<tcam::property::IPropertyBase> prop) : m_prop(prop) {}

    std::shared_ptr<tcam::property::IPropertyBase> m_prop;

    virtual auto get_property_name() const noexcept -> std::string_view final
    {
        return m_prop->get_name();
    }
    virtual auto get_property_info() const noexcept -> tcamprop1::prop_static_info final
    {
        return m_prop->get_static_info();
    }

    virtual auto get_property_state(uint32_t /*flags = 0*/)
        -> outcome::result<tcamprop1::prop_state> final
    {
        auto flags = m_prop->get_flags();

        tcamprop1::prop_state ret = {};
        ret.is_implemented = flags & tcam::property::PropertyFlags::Implemented;
        ret.is_locked = tcam::property::is_locked(flags);
        ret.is_available = flags & tcam::property::PropertyFlags::Available;
        return ret;
    }
};


struct TcamPropertyInteger : TcamPropertyBase<tcamprop1::property_interface_integer>
{
    TcamPropertyInteger(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase { prop }
    {
    }

    virtual auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_integer> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        return tmp->get_range();
    }

    virtual auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<int64_t> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value(uint32_t /*flags*/) -> outcome::result<int64_t> final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        return tmp->get_value();
    }
    virtual auto set_property_value(int64_t value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        auto ret = tmp->set_value(value);
        if (ret)
        {
            return tcam::status::Success;
        }
        return ret.error();
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
    TcamPropertyFloat(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase { prop }
    {
    }

    virtual auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_float> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        return tmp->get_range();
    }

    virtual auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<double> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value(uint32_t /*flags*/) -> outcome::result<double> final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value(double value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());

        auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcam::status::Success;
        }
        return ret.error();
    }

    virtual auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());
        return tmp->get_representation();
    }

    virtual auto get_unit() const noexcept -> std::string_view final
    {
        auto tmp = static_cast<tcam::property::IPropertyFloat*>(m_prop.get());
        return tmp->get_unit();
    }
};


struct TcamPropertyBoolean : TcamPropertyBase<tcamprop1::property_interface_boolean>
{
    TcamPropertyBoolean(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase { prop }
    {
    }

    virtual auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<bool> final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        return tmp->get_default();
    }

    virtual auto get_property_value(uint32_t /*flags*/) -> outcome::result<bool> final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value(bool value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyBool*>(m_prop.get());

        auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcam::status::Success;
        }
        return ret.error();
    }
};


struct TcamPropertyEnumeration : TcamPropertyBase<tcamprop1::property_interface_enumeration>
{
    TcamPropertyEnumeration(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase { prop }
    {
    }

    virtual auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_enumeration> final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        return tcamprop1::prop_range_enumeration { tmp->get_entries() };
    }

    virtual auto get_property_default(uint32_t /*flags = 0 */)
        -> outcome::result<std::string_view> final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        auto entries = tmp->get_entries();

        auto it = std::find(entries.begin(), entries.end(), tmp->get_default());

        size_t index = 0;

        if (it != entries.end())
        {
            index = std::distance(entries.begin(), it);
        }
        return entries.at(index);
    }

    virtual auto get_property_value(uint32_t /*flags*/) -> outcome::result<std::string_view> final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        return tmp->get_value();
    }

    virtual auto set_property_value(std::string_view value, uint32_t /*flags*/)
        -> std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        auto ret = tmp->set_value_str(value);
        if (ret)
        {
            return tcam::status::Success;
        }
        return ret.error();
    }
};


struct TcamPropertyCommand : TcamPropertyBase<tcamprop1::property_interface_command>
{
    TcamPropertyCommand(std::shared_ptr<tcam::property::IPropertyBase> prop)
        : TcamPropertyBase { prop }
    {
    }

    auto execute_command(uint32_t /* flags */) -> std::error_code final
    {
        auto tmp = static_cast<tcam::property::IPropertyCommand*>(m_prop.get());
        auto ret = tmp->execute();
        if (ret)
        {
            return tcam::status::Success;
        }
        return ret.error();
    }
};
} // namespace tcam::mainsrc

auto tcam::mainsrc::make_wrapper_instance(
    const std::shared_ptr<tcam::property::IPropertyBase>& prop)
    -> std::unique_ptr<tcamprop1::property_interface>
{
    switch (prop->get_type())
    {
        case tcam::TCAM_PROPERTY_TYPE_INTEGER:
        {
            return std::make_unique<tcam::mainsrc::TcamPropertyInteger>(prop);
        }
        case tcam::TCAM_PROPERTY_TYPE_DOUBLE:
        {
            return std::make_unique<tcam::mainsrc::TcamPropertyFloat>(prop);
        }
        case tcam::TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            return std::make_unique<tcam::mainsrc::TcamPropertyBoolean>(prop);
        }
        case tcam::TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            return std::make_unique<tcam::mainsrc::TcamPropertyEnumeration>(prop);
        }
        case tcam::TCAM_PROPERTY_TYPE_BUTTON:
        {
            return std::make_unique<tcam::mainsrc::TcamPropertyCommand>(prop);
        }
        case tcam::TCAM_PROPERTY_TYPE_STRING:
            return nullptr;
        case tcam::TCAM_PROPERTY_TYPE_UNKNOWN:
            return nullptr;
    }
    return nullptr;
}

static auto tcammainsrc_get_provider_impl_from_interface(TcamPropertyProvider* iface)
    -> tcamprop1_gobj::tcam_property_provider*
{
    assert(iface != nullptr);

    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);
    assert(self != nullptr);
    assert(self->device != nullptr);

    return &self->device->get_container();
}

void tcam::mainsrc::gst_tcam_mainsrc_tcamprop_init(TcamPropertyProviderInterface* iface)
{
    tcamprop1_gobj::init_provider_interface<&tcammainsrc_get_provider_impl_from_interface>(iface);
}
