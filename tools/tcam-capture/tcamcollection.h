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

#include <gst/gst.h>
#include <tcam-property-1.0.h>
#include <vector>
#include <map>
#include <string>

class TcamCollection
{
private:
    std::vector<GstElement*> m_elements;

    std::map<std::string, GstElement*> m_prop_origin;

public:
    TcamCollection() = default;

    explicit TcamCollection(GstBin* pipeline);

    std::vector<std::string> get_names() const;

    TcamPropertyBase* get_property(const std::string& name);

    gboolean set_property(const std::string& name, const GValue* value);

    GstElement* origin_of_property(const std::string& name);

};
