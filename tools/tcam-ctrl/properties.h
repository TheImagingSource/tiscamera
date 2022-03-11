/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include <string>

namespace tcam::tools::ctrl
{

enum class OutputType
{
    ConsoleFirendly,
    AsIs,
};

/**
 * @brief print function for properties
 * @param properties - Property collection that shall be printed
 */
void print_properties (const std::string& serial);


void print_tcam_properties (const std::string& serial,
                            OutputType print_type = OutputType::ConsoleFirendly);

void load_tcam_properties (const std::string& serial, const std::string& property_string);

void print_state_json (const std::string& serial);

void load_state_json_string (const std::string& serial, const std::string& json_str);

} // namespace tcam::tools::ctrl
