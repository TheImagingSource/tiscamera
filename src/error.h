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

#include "compiler_defines.h"

#include <outcome/outcome.hpp>
#include <system_error>

namespace outcome = OUTCOME_V2_NAMESPACE;

namespace tcam
{

enum class status : int
{
    Success = 0,
    UndefinedError,
    Timeout,
    NotImplemented,
    InvalidParameter,

    DeviceCouldNotBeOpened,
    DeviceAccessBlocked,
    DeviceLost,

    PropertyNotImplemented,
    //PropertyNotAvailable,
    PropertyNotWriteable,
    PropertyValueOutOfBounds,
    PropertyNoDefaultAvailable,

    FormatInvalid,
    ResourceNotLockable,
};

VISIBILITY_DEFAULT

std::error_code make_error_code(tcam::status e);

std::error_category& error_category();

VISIBILITY_POP

} // namespace tcam

namespace std
{
template<> struct is_error_code_enum<tcam::status> : true_type
{
};
} // namespace std
