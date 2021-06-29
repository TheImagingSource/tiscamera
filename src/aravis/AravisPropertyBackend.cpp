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

#include <arv.h>
#include "../logging.h"

namespace tcam::property
{

AravisPropertyBackend::AravisPropertyBackend(_ArvDevice* dev) : p_device(dev) {}

outcome::result<int64_t> AravisPropertyBackend::get_int(const std::string& name)
{
    GError* err = nullptr;
    int i = arv_device_get_integer_feature_value(p_device, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve integer {}: {}", name, err->message);
        g_clear_error(&err);
    }

    return i;
}

outcome::result<void> AravisPropertyBackend::set_int(const std::string& name, int64_t new_value)
{
    GError* err = nullptr;
    arv_device_set_integer_feature_value(p_device, name.c_str(), new_value, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set integer {}: {}", name, err->message);
        g_clear_error(&err);
    }
    return outcome::success();
}

outcome::result<double> AravisPropertyBackend::get_double(const std::string& name)
{
    GError* err = nullptr;
    double d = arv_device_get_float_feature_value(p_device, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve float {}: {}", name, err->message);
        g_clear_error(&err);
    }

    return d;
}

outcome::result<void> AravisPropertyBackend::set_double(const std::string& name, double new_value)
{
    GError* err = nullptr;
    arv_device_set_float_feature_value(p_device, name.c_str(), new_value, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set float {}: {}", name, err->message);
        g_clear_error(&err);
        return tcam::status::UndefinedError;
    }
    return outcome::success();
}

outcome::result<bool> AravisPropertyBackend::get_bool(const std::string& name)
{
    GError* err = nullptr;
    int b = arv_device_get_boolean_feature_value(p_device, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve bool: {}", err->message);
        g_clear_error(&err);
        // TODO: GError to outcome?
        return tcam::status::UndefinedError;
    }

    if (b < 0 || b > 1)
    {
        SPDLOG_ERROR("Bool has undefined internal value {} {}", name.c_str(), b);
        return tcam::status::PropertyOutOfBounds;
    }

    return b;
}

outcome::result<void> AravisPropertyBackend::set_bool(const std::string& name, bool new_value)
{
    GError* err = nullptr;

    arv_device_set_boolean_feature_value(p_device, name.c_str(), new_value, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set bool: {}", err->message);
        g_clear_error(&err);
    }

    return outcome::success();
}

outcome::result<void> AravisPropertyBackend::execute(const std::string& name)
{
    GError* err = nullptr;

    arv_device_execute_command(p_device, name.c_str(), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set button/command: {}", err->message);
        g_clear_error(&err);
    }

    return outcome::success();
}

outcome::result<std::string> AravisPropertyBackend::get_enum(const std::string& name)
{
    GError* err = nullptr;
    std::string ret = arv_device_get_string_feature_value(p_device, name.c_str(), &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve enum/string: {}", err->message);
        g_clear_error(&err);
        return tcam::status::PropertyValueDoesNotExist;
    }

    return ret;
}

outcome::result<void> AravisPropertyBackend::set_enum(const std::string& name,
                                                      const std::string& value)
{
    GError* err = nullptr;

    arv_device_set_string_feature_value(p_device, name.c_str(), value.c_str(), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set enum: {}", err->message);

        g_clear_error(&err);
    }

    return outcome::success();
}


} // namespace tcam::property
