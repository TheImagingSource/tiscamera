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

#include "AravisPropertyBackend.h"
#include "../logging.h"

namespace
{

tcam::property::AccessMode arv_access_to_tcam(ArvGcAccessMode mode)
{
    switch (mode)
    {
        case ARV_GC_ACCESS_MODE_RW:
        {
            return tcam::property::AccessMode::RW;
        }
        case ARV_GC_ACCESS_MODE_RO:
        {
            return tcam::property::AccessMode::RO;
        }
        case ARV_GC_ACCESS_MODE_WO:
        {
            return tcam::property::AccessMode::WO;
        }
        case ARV_GC_ACCESS_MODE_UNDEFINED:
        default:
        {
            return tcam::property::AccessMode::NoAccess;
        }
    }
}


tcam::Visibility arv_visibility_to_tcam(ArvGcVisibility visibility)
{

    // typedef enum {
    //     ARV_GC_VISIBILITY_UNDEFINED = -1,
    //     ARV_GC_VISIBILITY_INVISIBLE,
    //     ARV_GC_VISIBILITY_GURU,
    //     ARV_GC_VISIBILITY_EXPERT,
    //     ARV_GC_VISIBILITY_BEGINNER
    // } ArvGcVisibility;

    switch (visibility)
    {
        case ARV_GC_VISIBILITY_UNDEFINED:
        case ARV_GC_VISIBILITY_BEGINNER:
        {
            return tcam::Visibility::Beginner;
        }
        case ARV_GC_VISIBILITY_EXPERT:
        {
            return tcam::Visibility::Expert;
        }
        case ARV_GC_VISIBILITY_GURU:
        {
            return tcam::Visibility::Guru;
        }
        case ARV_GC_VISIBILITY_INVISIBLE:
        default:
        {
            return tcam::Visibility::Invisible;
        }
    }
}


tcam::property::PropertyFlags arv_flags_to_tcam(ArvGcNode* node)
{

    tcam::property::PropertyFlags flags = tcam::property::PropertyFlags::None;

    GError* err = nullptr;

    bool ret_avail = arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
        g_error_free(err);
    }
    else
    {
        if (ret_avail)
        {
            flags |= tcam::property::PropertyFlags::Available;
        }
    }

    bool ret_implemented = arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
        g_error_free(err);
    }
    else
    {
        if (ret_implemented)
        {
            flags |= tcam::property::PropertyFlags::Implemented;
        }
    }

    bool ret_locked = arv_gc_feature_node_is_locked(ARV_GC_FEATURE_NODE(node), &err);
    auto access_mode =
        arv_access_to_tcam(arv_gc_feature_node_get_actual_access_mode(ARV_GC_FEATURE_NODE(node)));

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve node flag information: {}", err->message);
        g_error_free(err);
    }
    else
    {
        if (ret_locked || access_mode == tcam::property::AccessMode::RO)
        {
            flags |= tcam::property::PropertyFlags::Locked;
        }
    }

    return flags;
}


} // namespace


namespace tcam::property
{


AravisPropertyIntegerImpl::AravisPropertyIntegerImpl(const std::string& name,
                                                     ArvCamera* camera,
                                                     ArvGcNode* node,
                                                     std::shared_ptr<AravisPropertyBackend> backend)
    : m_cam(backend), m_name(name), p_node(node)
{
    GError* err = nullptr;
    m_actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);

    m_default = arv_device_get_integer_feature_value(
        arv_camera_get_device(camera), m_actual_name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve aravis int - {}: {}", name, err->message);
        g_clear_error(&err);
    }

    m_step = 1;

    arv_device_get_integer_feature_bounds(
        arv_camera_get_device(camera), m_actual_name.c_str(), &m_min, &m_max, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve aravis int bounds: {}", err->message);
        g_clear_error(&err);
    }
}


PropertyFlags AravisPropertyIntegerImpl::get_flags() const
{
    return arv_flags_to_tcam(p_node);
}


outcome::result<int64_t> AravisPropertyIntegerImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_actual_name);
        if (ret)
        {
            return ret.value();
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock aravis device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AravisPropertyIntegerImpl::set_value(int64_t new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        auto r = ptr->set_int(m_actual_name, new_value);
        if (!r)
        {
            return r.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock write device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AravisPropertyIntegerImpl::valid_value(int64_t val) const
{
    if (get_min() > val || val > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


AravisPropertyDoubleImpl::AravisPropertyDoubleImpl(const std::string& name,
                                                   ArvCamera* camera,
                                                   ArvGcNode* node,
                                                   std::shared_ptr<AravisPropertyBackend> backend)
    : m_cam(backend), m_name(name), p_node(node)
{
    GError* err = nullptr;
    m_actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);

    m_default = arv_device_get_float_feature_value(
        arv_camera_get_device(camera), m_actual_name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve aravis float: {}", err->message);
        g_clear_error(&err);
    }

    m_step = 0.01;

    arv_device_get_float_feature_bounds(
        arv_camera_get_device(camera), m_actual_name.c_str(), &m_min, &m_max, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve aravis float bounds: {}", err->message);
        g_clear_error(&err);
    }
}


PropertyFlags AravisPropertyDoubleImpl::get_flags() const
{
    return arv_flags_to_tcam(p_node);
}


outcome::result<void> AravisPropertyDoubleImpl::set_value(double new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        auto r = ptr->set_double(m_actual_name, new_value);
        if (!r)
        {
            return r.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock write device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<double> AravisPropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_double(m_actual_name);
        if (ret)
        {
            return ret.value();
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock aravis device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AravisPropertyDoubleImpl::valid_value(double value) const
{
    if (get_min() > value || value > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


AravisPropertyBoolImpl::AravisPropertyBoolImpl(const std::string& name,
                                               ArvCamera* camera,
                                               ArvGcNode* node,
                                               std::shared_ptr<AravisPropertyBackend> backend)
    : m_cam(backend), m_name(name), p_node(node)
{
    GError* err = nullptr;
    m_actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);
    m_default = arv_device_get_boolean_feature_value(
        arv_camera_get_device(camera), m_actual_name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve aravis bool: {}", err->message);
        g_clear_error(&err);
    }

    m_cam = backend;
}


PropertyFlags AravisPropertyBoolImpl::get_flags() const
{
    return arv_flags_to_tcam(p_node);
}


outcome::result<bool> AravisPropertyBoolImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_bool(m_actual_name);
        auto ret = ptr->get_bool(m_actual_name);
        if (ret)
        {
            return ret.value();
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AravisPropertyBoolImpl::set_value(bool new_value)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->set_bool(m_actual_name, new_value);
        if (!ret)
        {
            return ret.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


AravisPropertyCommandImpl::AravisPropertyCommandImpl(const std::string& name,
                                                     ArvGcNode* node,
                                                     std::shared_ptr<AravisPropertyBackend> backend)
    : m_cam(backend), m_name(name), p_node(node)
{
    m_actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);
}


PropertyFlags AravisPropertyCommandImpl::get_flags() const
{
    return arv_flags_to_tcam(p_node);
}


outcome::result<void> AravisPropertyCommandImpl::execute()
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->execute(m_actual_name);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock aravis backend. Cannot execute command property {}", m_name);
        return tcam::status::ResourceNotLockable;
    }
}


AravisPropertyEnumImpl::AravisPropertyEnumImpl(const std::string& name,
                                               ArvCamera* camera,
                                               ArvGcNode* node,
                                               std::shared_ptr<AravisPropertyBackend> backend)
    : m_cam(backend), m_name(name), p_node(node)
{
    m_actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);

    unsigned int n_entries = 0;
    GError* err = nullptr;

    const char** entries = arv_camera_dup_available_enumerations_as_strings(
        camera, m_actual_name.c_str(), &n_entries, &err);
    if (err)
    {
        SPDLOG_ERROR("Error while retrieving enum values: {}", err->message);
    }

    if (entries)
    {
        m_entries.reserve(n_entries);
        for (unsigned int i = 0; i < n_entries; ++i) { m_entries.push_back(entries[i]); }
    }

    const char* def_ret = arv_device_get_string_feature_value(
        arv_camera_get_device(camera), m_actual_name.c_str(), &err);

    if (err)
    {
        SPDLOG_ERROR("Error while retrieving current enum value: {}", err->message);
        m_default = "";
    }
    else
    {
        m_default = def_ret;
    }
}


PropertyFlags AravisPropertyEnumImpl::get_flags() const
{
    return arv_flags_to_tcam(p_node);
}


outcome::result<void> AravisPropertyEnumImpl::set_value_str(const std::string& new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_enum(m_actual_name, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock aravis backend. Cannot execute command property {}", m_name);
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AravisPropertyEnumImpl::set_value(int64_t /*new_value*/)
{
    return tcam::status::NotImplemented;
}


outcome::result<std::string> AravisPropertyEnumImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_enum(m_actual_name);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock aravis backend. Cannot get value command property {}", m_name);
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<int64_t> AravisPropertyEnumImpl::get_value_int() const
{
    return tcam::status::NotImplemented;
}

std::vector<std::string> AravisPropertyEnumImpl::get_entries() const
{
    return m_entries;
}


} // namespace tcam::property
