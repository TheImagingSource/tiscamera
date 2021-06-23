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

#include <cstdint>

#include <memory>
#include <map>
#include <string>

#include "error.h"

namespace tcam::v4l2
{



class ConverterIntToDouble
{
public:
    virtual ~ConverterIntToDouble() = default;
    virtual double to_double (int64_t) = 0;
    virtual int64_t to_int (double) = 0;
};


class ConverterScale
{
public:

    virtual ~ConverterScale() = default;
    virtual double to_device(double) = 0;
    virtual double from_device(double) = 0;
};


enum class MappingType
{
    None,
    IntToEnum,
    IntToDouble,
    Scale,
};


std::shared_ptr<ConverterIntToDouble> find_int_to_double (uint32_t v4l2_id);

std::shared_ptr<ConverterScale> find_scale (uint32_t v4l2_id);

outcome::result<std::map<int, std::string>> find_menu_entries (uint32_t v4l2_id);

}
