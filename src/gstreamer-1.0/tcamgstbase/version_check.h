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

#pragma once

#include <cstdio> // sscanf
#include <string>
#include <algorithm>

namespace tcam::gst
{

struct version_info
{
    int major_ = -1;
    int minor_ = -1;
    int patch_ = -1;

    std::string additional_;

    version_info() = default;

    version_info(int maj, int min, int pat, const std::string& add = "")
    : major_(maj), minor_(min), patch_(pat), additional_(add)
    {}

    explicit version_info(const std::string& version_string)
    {
        sscanf(version_string.c_str(), "%d.%d.%d", &major_, &minor_, &patch_);
    }


    bool operator<=(const version_info& o) const
    {
        //
        // check if the checked field is -1 aka invalid
        // if so assume we are bigger
        //
        if ((major_ == o.major_
            && minor_ == o.minor_
            && patch_ <= o.patch_)
            && o.patch_ != -1)
        {
            return true;
        }
            else if ((major_ == o.major_
                 && minor_ <= o.minor_)
                 && o.minor_ != -1)
        {
            return true;
        }
        else if (major_ <= o.major_ && o.major_ != -1)
        {
            return true;
        }


        return false;
    }


    bool operator<(const version_info& o) const
    {
        //
        // check if the checked field is -1 aka invalid
        // if so assume we are bigger
        //

        if (major_ < o.major_ && o.major_ != -1)
        {
            return true;
        }
        else if ((major_ == o.major_
                 && minor_ < o.minor_)
                 && o.minor_ != -1)
        {
            return true;
        }
        else if ((major_ == o.major_
                 && minor_ == o.minor_
                 && patch_ < o.patch_)
                 && o.patch_ != -1)
        {
            return true;
        }

        return false;
    }


    bool operator== (const version_info& other) const
    {
        if (major_ == other.major_
            && minor_ == other.minor_
            && patch_ == other.patch_
            && additional_ == other.additional_)
        {
            return true;
        }
        return false;
    }


};


bool is_version_compatible_with_tiscamera(const std::string& element_name);

} // namespace tcam::gst
