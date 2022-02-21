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

#include "error.h"

namespace
{

struct status_code_entry
{
    const char* desc = nullptr;
    std::errc code = static_cast<std::errc>(0);
};

constexpr status_code_entry to_entry(int in) noexcept
{
    using namespace tcam;

    auto c = static_cast<tcam::status>(in);

    switch (c)
    {
        case tcam::status::Success:
        {
            return { "Success" };
        }
        case tcam::status::DeviceCouldNotBeOpened:
        {
            return { "Unable to open the device", std::errc::device_or_resource_busy };
        }
        case tcam::status::DeviceAccessBlocked:
        {
            return { "Device is in use by another process/user", std::errc::permission_denied };
        }
        case tcam::status::DeviceLost:
        {
            return { "Device has been lost", std::errc::owner_dead };
        }
        case tcam::status::PropertyNotImplemented:
        {
            return { "Property does not exist", std::errc::invalid_seek };
        }
        case tcam::status::PropertyValueOutOfBounds:
        {
            return { "Property value is out of bounds ", std::errc::result_out_of_range };
        }
        case tcam::status::PropertyNotWriteable:
        {
            return { "Property is locked", std::errc::operation_not_permitted };
        }
        case tcam::status::PropertyNoDefaultAvailable:
        {
            return { "Property has no default defined", std::errc::invalid_seek };
        }
        case tcam::status::FormatInvalid:
        {
            return { "Invalid video format", std::errc::invalid_argument };
        }
        case tcam::status::ResourceNotLockable:
        {
            return { "The needed resource could not be claimed", std::errc::no_lock_available };
        }
        case tcam::status::Timeout:
        {
            return { "Timeout", std::errc::timed_out };
        }
        case tcam::status::UndefinedError:
        {
            return { "Undefined Error", std::errc::no_message };
        }
        case tcam::status::NotImplemented:
        {
            return { "Not implemented", std::errc::bad_message };
        }
        case tcam::status::InvalidParameter:
        {
            return { "Invalid parameter", std::errc::invalid_argument };
        }
    };
    return {};
}


class tcam_error_category : public std::error_category
{

public:
    const char* name() const noexcept final
    {
        return "Tiscamera Error";
    }

    std::string message(int err) const final
    {
        auto str = to_entry(err).desc;
        if (str == nullptr)
        {
            return "Unknown Error";
        }
        return str;
    }

    std::error_condition default_error_condition(int err) const noexcept final
    {
        if (err == 0)
        { // 0 ^= success so use the default ctor of error_condition to build a efficient success value
            return {};
        }

        auto str = to_entry(err);
        if (str.desc == nullptr)
        { // when desc is empty, we just copy our category into the condition
            return std::error_condition(err, *this);
        }
        return str.code; // if we have a map, use that
    }
};

static tcam_error_category error_cat_;

} /* namespace */


std::error_category& tcam::error_category()
{
    return error_cat_;
}

std::error_code tcam::make_error_code(tcam::status e)
{
    return { static_cast<int>(e), error_cat_ };
}
