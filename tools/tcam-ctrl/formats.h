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

#ifndef FORMATS_H
#define FORMATS_H

#include <string>

namespace tcam::tools::ctrl
{

/**
 * @brief print function for VideoFormatDescriptions as gstreamer-1.0 caps
 * @param available_formats - format descriptions that shell be printed
 */
void list_gstreamer_1_0_formats(const std::string& serial);


enum class ElementPadDirection
{
    Both,
    In,
    Out,
};

int convert(const std::string& element_name,
            ElementPadDirection direction = ElementPadDirection::Both,
            const std::string& caps_str = "");

} // namespace tcam::tools::ctrl

#endif /* FORMATS_H */
