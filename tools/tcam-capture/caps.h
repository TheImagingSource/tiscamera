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

#ifndef CAPS_H
#define CAPS_H

#include <gst/gst.h>
#include <string>
#include <vector>




struct caps_resolution
{
    unsigned int width = 0;
    unsigned int height = 0;

    std::vector<double> framerates;
};

struct scaling
{
    unsigned int binning_h = 1;
    unsigned int binning_v = 1;

    unsigned int skipping_h = 1;
    unsigned int skipping_v = 1;

    bool is_default() const
    {
        if (binning_h == 1 && binning_v == 1 && skipping_h == 1 && skipping_v == 1)
        {
            return true;
        }
        return false;
    }

    bool operator==(const scaling& other) const
    {
        if (binning_h == other.binning_h && binning_v == other.binning_v
            && skipping_h == other.skipping_h && skipping_v == other.skipping_v)
        {
            return true;
        }
        return false;
    }

    std::string skipping_str() const
    {
        return std::to_string(skipping_h) + "x" + std::to_string(skipping_v);
    }

    std::string binning_str() const
    {
        return std::to_string(binning_h) + "x" + std::to_string(binning_v);
    }
};

struct caps_format
{
    std::string format;
    std::string gst_format;

    scaling scale;

    std::vector<struct caps_resolution> resolutions;
};

class Caps
{
public:
    explicit Caps(GstCaps* caps, GstElement& element);
    Caps(const Caps& other);
    ~Caps();

    static GstCaps* get_default_caps( GstCaps* intersect );

    bool has_resolution_ranges() const;
    std::vector<std::string> get_formats() const;
    std::string get_gst_name(const std::string& format) const;
    std::vector<std::pair<uint, uint>> get_resolutions(const std::string& format,
                                                       const scaling& scale = {}) const;

    std::vector<double> get_framerates(const std::string& format,
                                       const scaling& scale,
                                       unsigned int width,
                                       unsigned int height) const;

    bool has_binning() const;
    bool has_skipping() const;

    std::vector<std::string> get_binning(const std::string& format) const;
    std::vector<std::string> get_skipping(const std::string& format) const;
private:
    GstCaps* p_caps = nullptr;

    std::vector<caps_format> formats;

    std::vector<caps_format> generate_from_fixed_caps(GstElement& element);
    std::vector<caps_format> generate_from_caps_list(GstElement& element);

    void generate(GstElement& element);
};

#endif // CAPS_H
