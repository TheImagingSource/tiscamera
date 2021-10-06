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

#include "device.h"

Device::Device()
{
    p_caps = nullptr;
}

Device::Device(const std::string& model,
               const std::string& serial,
               const std::string& type,
               GstCaps* caps)
    : m_serial(serial), m_model(model), m_type(type)
{
    if (caps)
    {
        p_caps = gst_caps_copy(caps);
    }
    else
    {
        p_caps = nullptr;
    }
}
Device::Device(const Device& other)
    : m_serial(other.m_serial), m_model(other.m_model), m_type(other.m_type)
{
    if (other.p_caps)
    {
        p_caps = gst_caps_copy(other.p_caps);
    }
    else
    {
        p_caps = nullptr;
    }
}


Device& Device::operator=(const Device& other)
{
    m_serial = other.m_serial;
    m_model = other.m_model;
    m_type = other.m_type;

    if (other.p_caps)
    {
        p_caps = gst_caps_copy(other.p_caps);
    }
    else
    {
        p_caps = nullptr;
    }

    return *this;
}

bool Device::operator==(const Device& other) const
{
    if (other.m_serial == m_serial && other.m_model == m_model && other.m_type == m_type)
    {
        return true;
    }
    return false;
}

Device::~Device()
{
    if (p_caps)
    {
        gst_caps_unref(p_caps);
    }
}

std::string Device::serial() const
{
    return m_serial;
}

std::string Device::serial_long() const
{
    return m_serial + "-" + m_type;
}

std::string Device::model() const
{
    return m_model;
}

std::string Device::type() const
{
    return m_type;
}

std::string Device::str() const
{
    return m_model + "-" + m_serial + "-" + m_type;
}

void Device::set_caps(GstCaps* caps)
{
    if (p_caps)
    {
        gst_caps_unref(p_caps);
    }

    p_caps = gst_caps_copy(caps);

}

GstCaps* Device::caps() const
{
    return p_caps;
}
