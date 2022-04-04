/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#include "tcamgstjson.h"

#include "../../../external/json/json.hpp"
#include "../../logging.h"

// for convenience
using json = nlohmann::json;

std::string tcam::gst::create_device_settings(TcamPropertyProvider* tcam)
{
    if (!tcam)
    {
        return {};
    }

    json j;

    auto& write_obj = j;

    GError* err_get_name = nullptr;
    GSList* names = tcam_property_provider_get_tcam_property_names(tcam, &err_get_name);
    if (err_get_name)
    {
        SPDLOG_ERROR("Failed to read property names from , err={}.", err_get_name->message);
        g_error_free(err_get_name);
        return {};
    }

    /**
     * If err is set, reports the error and returns true. Otherwise false is returned.
     */
    auto is_prop_error_consume = [](GError* err, const char* property_name) -> bool
    {
        if (!err)
        {
            return false;
        }
        SPDLOG_ERROR("Reading '{}' caused an error: {}", property_name, err->message);
        g_error_free(err);
        return true;
    };

    for (unsigned int i = 0; i < g_slist_length(names); ++i)
    {
        GError* err_get_prop = nullptr;

        const char* prop_name = static_cast<const char*>(g_slist_nth_data(names, i));

        auto prop_base = tcam_property_provider_get_tcam_property(tcam, prop_name, &err_get_prop);
        if (is_prop_error_consume(err_get_prop, prop_name))
        {
            continue;
        }

        if (tcam_property_base_get_access(prop_base) == TCAM_PROPERTY_ACCESS_WO)
            continue;
        if (!tcam_property_base_is_available(prop_base, nullptr))
            continue;

        switch (tcam_property_base_get_property_type(prop_base))
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                GError* err = nullptr;
                auto value =
                    tcam_property_integer_get_value(TCAM_PROPERTY_INTEGER(prop_base), &err);
                if (!is_prop_error_consume(err, prop_name))
                {
                    write_obj.push_back(json::object_t::value_type(prop_name, value));
                }
                break;
            }
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                GError* err = nullptr;
                auto value = tcam_property_float_get_value(TCAM_PROPERTY_FLOAT(prop_base), &err);
                if (!is_prop_error_consume(err, prop_name))
                {
                    write_obj.push_back(json::object_t::value_type(prop_name, value));
                }
                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                GError* err = nullptr;
                auto value =
                    tcam_property_enumeration_get_value(TCAM_PROPERTY_ENUMERATION(prop_base), &err);
                if (!is_prop_error_consume(err, prop_name))
                {
                    write_obj.push_back(json::object_t::value_type(prop_name, value));
                }
                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                GError* err = nullptr;
                auto value =
                    tcam_property_boolean_get_value(TCAM_PROPERTY_BOOLEAN(prop_base), &err);
                if (!is_prop_error_consume(err, prop_name))
                {
                    write_obj.push_back(json::object_t::value_type(prop_name, (bool)value));
                }
                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                // Nothing to do here
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING:
            {
                GError* err = nullptr;
                auto value =
                    tcam_property_string_get_value(TCAM_PROPERTY_STRING(prop_base), &err);
                if (!is_prop_error_consume(err, prop_name))
                {
                    if (value)
                    {
                        write_obj.push_back(json::object_t::value_type(prop_name, value));
                    }
                }
                g_free(value);
                break;
            }
        }

        g_object_unref(prop_base);
    }
    g_slist_free_full(names, ::g_free);

    int indent = 4;
    return j.dump(indent);
}

enum class apply_single_json_entry_rval
{
    success,
    error,
    locked_error,
};

static auto apply_single_json_entry(
    TcamPropertyProvider* tcam,
    json::iterator iter,
    std::function<void(std::string_view, std::string_view)> report_error_func)
    -> apply_single_json_entry_rval
{
    auto property_name = iter.key();

    try
    {
        GError* err = nullptr;
        auto prop_base = tcam_property_provider_get_tcam_property(tcam, property_name.c_str(), &err);
        if (err)
        {
            report_error_func(property_name, err->message);
            g_error_free(err);
            return apply_single_json_entry_rval::error;
        }
        
        if (tcam_property_base_get_access(prop_base) == TCAM_PROPERTY_ACCESS_RO)
            return apply_single_json_entry_rval::error;
        if (!tcam_property_base_is_available(prop_base, nullptr))
            return apply_single_json_entry_rval::locked_error;


        switch (tcam_property_base_get_property_type(prop_base))
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                if (!iter.value().is_number())
                {
                    report_error_func(
                        property_name, fmt::format("Value '{}' is not a number", iter.value().dump()));
                    break;
                }
                tcam_property_integer_set_value(
                    TCAM_PROPERTY_INTEGER(prop_base), iter.value(), &err);
                break;
            }
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                if (!iter.value().is_number())
                {
                    report_error_func(
                        property_name, fmt::format("Value '{}' is not a number", iter.value().dump()));
                    break;
                }
                tcam_property_float_set_value(TCAM_PROPERTY_FLOAT(prop_base), iter.value(), &err);
                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                tcam_property_enumeration_set_value(TCAM_PROPERTY_ENUMERATION(prop_base),
                                                    iter.value().get<std::string>().c_str(),
                                                    &err);

                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                if (!iter.value().is_boolean() && !iter.value().is_number_unsigned())
                {
                    report_error_func(
                        property_name,
                        fmt::format("Value '{}' is not a boolean", iter.value().dump()));
                    break;
                }

                tcam_property_boolean_set_value(
                    TCAM_PROPERTY_BOOLEAN(prop_base), iter.value(), &err);
                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                tcam_property_command_set_command(TCAM_PROPERTY_COMMAND(prop_base), &err);
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING:
            {
                if (!iter.value().is_string())
                {
                    report_error_func(
                        property_name,
                        fmt::format("Value '{}' is not a string", iter.value().dump()));
                    break;
                }

                tcam_property_string_set_value(
                    TCAM_PROPERTY_STRING(prop_base), iter.value().get<std::string>().c_str(), &err);
                break;
            }
        }

        g_object_unref(prop_base);

        if (err)
        {
            if (err->domain == tcam_error_quark() && err->code == TCAM_ERROR_PROPERTY_NOT_WRITEABLE)
            {
                g_error_free(err);
                return apply_single_json_entry_rval::locked_error;
            }
            else
            {
                report_error_func(property_name,
                                  fmt::format("Setting value to {} failed with {}",
                                              iter.value().dump(),
                                              err->message));

                g_error_free(err);
                return apply_single_json_entry_rval::error;
            }
        }
        return apply_single_json_entry_rval::success;
    }
    catch (const std::exception& ex)
    {
        report_error_func(property_name, ex.what());
        return apply_single_json_entry_rval::error;
    }
}


bool tcam::gst::load_device_settings(TcamPropertyProvider* tcam, const std::string& json_data)
{
    if (!tcam)
    {
        return false;
    }

    json props;
    try
    {
        props = json::parse(json_data);
    }
    catch (json::parse_error& e)
    {
        SPDLOG_ERROR("Unable to parse property settings. JSON parser returned: {}", e.what());
        return false;
    }

    // Report functions
    auto report_error = [](std::string_view property_name, std::string_view error_desc)
    {
        SPDLOG_WARN("Failed to write property '{}', due to: {}", property_name, error_desc);
    };

    // we use a list of iterators to ease implementation of retries
    std::vector<json::iterator> prop_entry_list;
    prop_entry_list.reserve(props.size());
    for (auto iter = props.begin(); iter != props.end(); ++iter)
    {
        prop_entry_list.push_back(iter);
    }

    /* This works like this:
     * Walk current list
     *  if one returns 'locked' as the error, add that entry to the retry-list
     *  If one succeeds we set the according flag. 
     *  If the retry-list contains elements and at least one property could be successfully written, retry using the remaining items in the retry list.
     */

    // we need this flag to prevent us continually re-trying the same items.
    bool at_least_one_success = false;
    do {
        at_least_one_success = false;
        // this is the list of items which were blocked by a locked flag
        std::vector<json::iterator> retry_list;
        for (auto&& it : prop_entry_list)
        {
            auto res = apply_single_json_entry(tcam, it, report_error);
            if (res == apply_single_json_entry_rval::locked_error)
            {
                retry_list.push_back(it);
            }
            else if (res == apply_single_json_entry_rval::success)
            {
                at_least_one_success = true;
            }
        }
        // move contents of the retry_list into the list which will be walked in the next cycle
        prop_entry_list = std::move(retry_list);
    } while (at_least_one_success && !prop_entry_list.empty());

    // generate the error message list for the properties we could not write due to being 'locked'
    for (auto&& entry : prop_entry_list)
    {
        report_error(entry.key(), "Failed to write locked property");
    }

    return props.size() != prop_entry_list.size(); // we have at least one successfully 'set' entry
}
