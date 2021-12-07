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
#include "../compiler_defines.h"
#include "../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_info.h"

#include <arv.h>

VISIBILITY_INTERNAL

namespace tcam::property
{

class AravisPropertyBackend;

class AravisPropertyIntegerImpl : public IPropertyInteger
{

public:
    AravisPropertyIntegerImpl(const std::string& name,
                              ArvCamera* camera,
                              ArvGcNode* node,
                              std::shared_ptr<AravisPropertyBackend>);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }
    virtual std::string_view get_unit() const final;
    virtual tcamprop1::IntRepresentation_t get_representation() const final;

    virtual PropertyFlags get_flags() const final;

    virtual tcamprop1::prop_range_integer get_range() const final
    {
        return { m_min, m_max, m_step };
    }
    virtual int64_t get_default() const final
    {
        return m_default;
    };
    virtual outcome::result<int64_t> get_value() const final;

    virtual outcome::result<void> set_value(int64_t new_value) final;

private:
    std::weak_ptr<AravisPropertyBackend> m_cam;

    // display name
    std::string m_name;
    // some legacy cameras have incorrect
    // property names. those incorrect names are
    // still used for camera interactions
    std::string m_actual_name;

    int64_t m_min;
    int64_t m_max;
    int64_t m_step;
    int64_t m_default;

    ArvGcNode* p_node;
    const tcamprop1::prop_static_info_integer* p_static_info;
};


class AravisPropertyDoubleImpl : public IPropertyFloat
{

public:
    AravisPropertyDoubleImpl(const std::string& name,
                             ArvCamera* camera,
                             ArvGcNode* node,
                             std::shared_ptr<AravisPropertyBackend>);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }
    virtual std::string_view get_unit() const final;
    virtual tcamprop1::FloatRepresentation_t get_representation() const final;

    virtual PropertyFlags get_flags() const final;
    virtual tcamprop1::prop_range_float get_range() const final
    {
        return { m_min, m_max, m_step };
    }
    virtual double get_default() const final
    {
        return m_default;
    };
    virtual outcome::result<double> get_value() const final;

    virtual outcome::result<void> set_value(double new_value) final;

private:
    outcome::result<void> valid_value(double val) const;

    std::weak_ptr<AravisPropertyBackend> m_cam;

    // display name
    std::string m_name;
    // some legacy cameras have incorrect
    // property names. those incorrect names are
    // still used for camera interactions
    std::string m_actual_name;

    double m_min;
    double m_max;
    double m_step;
    double m_default;

    ArvGcNode* p_node;
    const tcamprop1::prop_static_info_float* p_static_info;
};


class AravisPropertyBoolImpl : public IPropertyBool
{
public:
    AravisPropertyBoolImpl(const std::string& name,
                           ArvCamera* camera,
                           ArvGcNode* node,
                           std::shared_ptr<AravisPropertyBackend> backend);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }

    virtual PropertyFlags get_flags() const final;

    virtual bool get_default() const final
    {
        return m_default;
    };
    virtual outcome::result<bool> get_value() const final;

    virtual outcome::result<void> set_value(bool new_value) final;

private:
    std::weak_ptr<AravisPropertyBackend> m_cam;

    // display name
    std::string m_name;
    // some legacy cameras have incorrect
    // property names. those incorrect names are
    // still used for camera interactions
    std::string m_actual_name;

    bool m_default;
    ArvGcNode* p_node;
    const tcamprop1::prop_static_info_boolean* p_static_info;
};


class AravisPropertyCommandImpl : public IPropertyCommand
{
public:
    AravisPropertyCommandImpl(const std::string& name,
                              ArvGcNode* node,
                              std::shared_ptr<AravisPropertyBackend> backend);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }

    virtual PropertyFlags get_flags() const final;

    virtual outcome::result<void> execute() final;

private:
    std::weak_ptr<AravisPropertyBackend> m_cam;

    // display name
    std::string m_name;
    // some legacy cameras have incorrect
    // property names. those incorrect names are
    // still used for camera interactions
    std::string m_actual_name;

    ArvGcNode* p_node;
    const tcamprop1::prop_static_info_command* p_static_info;
};


class AravisPropertyEnumImpl : public IPropertyEnum
{
public:
    AravisPropertyEnumImpl(const std::string& name,
                           ArvCamera* camera,
                           ArvGcNode* node,
                           std::shared_ptr<AravisPropertyBackend> backend);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }

    virtual PropertyFlags get_flags() const final;

    virtual outcome::result<void> set_value_str(const std::string_view& new_value) final;
    virtual outcome::result<void> set_value(int64_t new_value) final;

    virtual outcome::result<std::string_view> get_value() const final;
    virtual outcome::result<int64_t> get_value_int() const final;

    virtual std::string get_default() const final
    {
        return m_default;
    };

    virtual std::vector<std::string> get_entries() const final;

private:
    // outcome::result<void> valid_value(int value);
    std::vector<std::string> m_entries;
    //std::map<int, std::string> m_entries;
    std::weak_ptr<AravisPropertyBackend> m_cam;

    // display name
    std::string m_name;
    // some legacy cameras have incorrect
    // property names. those incorrect names are
    // still used for camera interactions
    std::string m_actual_name;

    std::string m_default;

    ArvGcNode* p_node;
    const tcamprop1::prop_static_info_enumeration* p_static_info;
};


} // namespace tcam::property

VISIBILITY_POP
