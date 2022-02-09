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

#include "AravisPropertyBackend.h"

#include "../logging.h"
#include "AravisDevice.h"
#include "aravis_utils.h"

#include <arv.h>

using namespace tcam::aravis;

AravisPropertyBackend::AravisPropertyBackend(tcam::AravisDevice& parent, _ArvDevice* dev)
    : parent_(parent), device_(dev)
{
}

outcome::result<int64_t> AravisPropertyBackend::get_int(const std::string& name)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    auto i = arv_device_get_integer_feature_value(device_, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve integer {}: {}", name, err->message);
        return consume_GError( err );
    }
    return i;
}

outcome::result<void> AravisPropertyBackend::set_int(const std::string& name, int64_t new_value)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    arv_device_set_integer_feature_value(device_, name.c_str(), new_value, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set integer {}: {}", name, err->message);
        return consume_GError(err);
    }
    return outcome::success();
}

outcome::result<double> AravisPropertyBackend::get_double(const std::string& name)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    double d = arv_device_get_float_feature_value(device_, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve float {}: {}", name, err->message);
        return consume_GError(err);
    }
    return d;
}

outcome::result<void> AravisPropertyBackend::set_double(const std::string& name, double new_value)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    arv_device_set_float_feature_value(device_, name.c_str(), new_value, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set float {}: {}", name, err->message);
        return consume_GError(err);
    }
    return outcome::success();
}

outcome::result<bool> AravisPropertyBackend::get_bool(const std::string& name)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    gboolean b = arv_device_get_boolean_feature_value(device_, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve bool: {}", err->message);
        return consume_GError(err);
    }
    return b != FALSE;
}

outcome::result<void> AravisPropertyBackend::set_bool(const std::string& name, bool new_value)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    arv_device_set_boolean_feature_value(device_, name.c_str(), new_value ? TRUE : FALSE, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set bool: {}", err->message);
        return consume_GError(err);
    }
    return outcome::success();
}

outcome::result<void> AravisPropertyBackend::execute(const std::string& name)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    arv_device_execute_command(device_, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set button/command: {}", err->message);
        return consume_GError(err);
    }
    return outcome::success();
}

outcome::result<std::string_view> AravisPropertyBackend::get_enum(const std::string& name)
{
    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    std::string_view ret = arv_device_get_string_feature_value(device_, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve enum/string: {}", err->message);
        return consume_GError(err);
    }
    return ret;
}

outcome::result<void> AravisPropertyBackend::set_enum(const std::string& name,
                                                      const std::string_view& value)
{
    // simple insurance that everything is null terminated
    std::string val = std::string(value);

    std::scoped_lock lck { parent_.arv_camera_access_ };

    GError* err = nullptr;
    arv_device_set_string_feature_value(device_, name.c_str(), val.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set enum: {}", err->message);
        return consume_GError(err);
    }
    return outcome::success();
}
