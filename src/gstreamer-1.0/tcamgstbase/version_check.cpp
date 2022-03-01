/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "version_check.h"
#include "../tcamgstbase/tcamgstbase.h"

#include "../../version.h"
#include "../../logging.h"

#include <string_view>
#include <iterator>
#include <climits> // INT_MAX

namespace
{

struct version_desc
{
    std::string_view name;

    tcam::gst::version_info minimal_;
    tcam::gst::version_info maximum_;
};


static const version_desc version_restrictions [] =
{
    {
        "tcamdutils",
        {1, 0, 0, ""},
        {INT_MAX, INT_MAX, INT_MAX, ""},
    },
    {
        "tcamdutils-cuda",
        {1, 2, 0, ""},
        {INT_MAX, INT_MAX, INT_MAX, ""},
    },
};

} // namespace

tcam::gst::version_info get_tiscamera_version ()
{
    return {
        atoi(get_version_major()),
        atoi(get_version_minor()),
        atoi(get_version_patch()),
        get_version_modifier()
    };
}


bool tcam::gst::is_version_compatible_with_tiscamera(const std::string& element_name)
{


    auto reference = std::find_if(std::begin(version_restrictions), std::end(version_restrictions),
                                  [=] (const version_desc& info)
                                  {
                                      return info.name == element_name;
                                  });

    if (reference == std::end(version_restrictions))
    {
        SPDLOG_ERROR("No compatability information available for {}", element_name);
        return false;
    }

    std::string version_str = tcam::gst::get_plugin_version(element_name.c_str());

    version_info version(version_str);

    if (reference->minimal_ <= version)
    {
        if (version <= reference->maximum_)
            return true;

    }
    return false;
}
