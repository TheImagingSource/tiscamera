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

#include "caps.h"

#include <algorithm>

#include <vector>
#include <string>
#include <QDebug>

namespace {

struct image_size
{
    int width;
    int height;

    bool operator<(const struct image_size& other) const
    {
        if (height <= other.height && width <= other.width)
        {
            return true;
        }
        return false;
    }

    bool operator==(const image_size& other) const
    {
        return (height == other.height
                && width == other.width);
    }
};

std::vector<image_size> get_standard_resolutions(const image_size& min,
                                                 const image_size& max,
                                                 const image_size& step)
{
    static const image_size resolutions[] = {
        { 128, 96 },    { 320, 240 },   { 360, 280 },   { 544, 480 },   { 640, 480 },
        { 352, 288 },   { 576, 480 },   { 720, 480 },   { 960, 720 },   { 1280, 720 },
        { 1440, 1080 }, { 1920, 1080 }, { 1920, 1200 }, { 2048, 1152 }, { 2048, 1536 },
        { 2560, 1440 }, { 3840, 2160 }, { 4096, 3072 }, { 7680, 4320 }, { 7680, 4800 },
    };

    std::vector<struct image_size> ret;
    ret.reserve(std::size(resolutions));
    for (const auto& r : resolutions)
    {
        if ((min < r) && (r < max) && (r.width % step.width == 0) && (r.height % step.height == 0))
        {
            ret.push_back(r);
        }
    }

    return ret;
}


std::vector<double> create_steps_for_range(double min, double max)
{
    std::vector<double> vec;

    if (max <= min)
        return vec;

    vec.push_back(min);

    // we do not want every framerate to have unnecessary decimals
    // e.g. 1.345678 instead of 1.00000
    double current_step = (int)min;

    // 0.0 is not a valid framerate
    if (current_step < 1.0)
        current_step = 1.0;

    while (current_step < max)
    {

        if (current_step < 20.0)
        {
            current_step += 1;
        }
        else if (current_step < 100.0)
        {
            current_step += 10.0;
        }
        else if (current_step < 1000.0)
        {
            current_step += 50.0;
        }
        else
        {
            current_step += 100.0;
        }
        if (current_step < max)
        {
            vec.push_back(current_step);
        }
    }

    if (vec.back() != max)
    {
        vec.push_back(max);
    }
    return vec;
}

std::vector<double> gvalue_to_fps_vector(const GValue* framerate)
{
    std::vector<double> framerates;

    if (G_VALUE_TYPE(framerate) == GST_TYPE_FRACTION_RANGE)
    {
        const GValue* fps_min = gst_value_get_fraction_range_min(framerate);
        const GValue* fps_max = gst_value_get_fraction_range_max(framerate);

        int num = gst_value_get_fraction_numerator(fps_min);
        int den = gst_value_get_fraction_denominator(fps_min);

        double fps_min_value;
        gst_util_fraction_to_double(num, den, &fps_min_value);

        num = gst_value_get_fraction_numerator(fps_max);
        den = gst_value_get_fraction_denominator(fps_max);

        double fps_max_value;
        gst_util_fraction_to_double(num, den, &fps_max_value);

//        qInfo("range for %dx%d: %f %f", res.width, res.height, fps_min_value, fps_max_value);

        framerates = create_steps_for_range(fps_min_value, fps_max_value);
    }
    else if (G_VALUE_TYPE(framerate) == GST_TYPE_LIST)
    {
    for (unsigned int x = 0; x < gst_value_list_get_size(framerate); ++x)
    {
        const GValue* val = gst_value_list_get_value(framerate, x);

        if (G_VALUE_TYPE(val) == GST_TYPE_FRACTION)
        {
            int num = gst_value_get_fraction_numerator(val);
            int den = gst_value_get_fraction_denominator(val);

            framerates.push_back((double)num / (double)den);
        }
        else
        {
            qWarning("Handling of framerate handling not implemented for non fraction types.\n");
            break;
        }
    }
    }
    else
    {
        qErrnoWarning("GValue type handling not implemented!");
    }
    return framerates;
}


std::vector<double> index_framerates(GstElement& element,
                                     const std::string& fmt,
                                     struct caps_resolution& res,
                                     const GValue* framerate)
{
    std::vector<double> framerates;

    if (G_VALUE_TYPE(framerate) == GST_TYPE_LIST)
    {
        framerates = gvalue_to_fps_vector(framerate);
    }
    else if (G_VALUE_TYPE(framerate) == GST_TYPE_FRACTION_RANGE)
    {
        std::string c = fmt;
        c += ",width=" + std::to_string(res.width);
        c += ",height=" + std::to_string(res.height);
        GstCaps* fps_caps = gst_caps_from_string(c.c_str());
        GstQuery* fps_query = gst_query_new_caps(fps_caps);
        gst_caps_unref(fps_caps);

        if (gst_element_query(&element, fps_query))
        {
            GstCaps* caps = nullptr;
            gst_query_parse_caps_result(fps_query, &caps);

            GstStructure* struc = gst_caps_get_structure(caps, 0);

            const GValue* fps = gst_structure_get_value(struc, "framerate");

            framerates = gvalue_to_fps_vector(fps);
        }
        else
        {
            framerates = gvalue_to_fps_vector(framerate);
        }

        gst_query_unref(fps_query);
    }

    std::sort(framerates.rbegin(), framerates.rend());

    return framerates;
}


std::vector<struct caps_format>::iterator find_format (std::vector<struct caps_format>& tmp, const std::string& name, const scaling& scale)
{
    auto ret = std::find_if(tmp.begin(), tmp.end(), [&name, &scale](const struct caps_format& fmt)
    {
        if (fmt.format == name
            && fmt.scale == scale)
        {
            return true;
        }
        return false;
    });

    return ret;
}


scaling create_scale(const GstStructure* structure)
{
    scaling ret;
    if (gst_structure_has_field(structure, "binning"))
    {
        std::string s = gst_structure_get_string(structure, "binning");

        ret.binning_h = std::stoi(s.substr(0, 1));
        ret.binning_v = std::stoi(s.substr(2, 1));
    }
    if (gst_structure_has_field(structure, "skipping"))
    {
        std::string s = gst_structure_get_string(structure, "skipping");

        ret.skipping_h = std::stoi(s.substr(0, 1));
        ret.skipping_v = std::stoi(s.substr(2, 1));
    }

    return ret;
}



}



Caps::Caps(GstCaps* caps, GstElement& element)
    : p_caps(gst_caps_copy(caps))
{
    generate(element);
}


Caps::Caps(const Caps& other)
{
    formats = other.formats;
    p_caps = gst_caps_copy(other.p_caps);
}


Caps::~Caps()
{
    gst_caps_unref(p_caps);
}

GstCaps* Caps::get_default_caps(GstCaps* intersect)
{
    // for performance sake, prefer bayer 8-bit
    static const char* defaults [] =
        {

            "video/x-bayer,format=pwl-rggb12m,width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format=pwl-rggb12,width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format=pwl-rggb16H12,width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format=pwl-rggb12m,width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format=pwl-rggb12,width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format=pwl-rggb16H12,width=1920,height=1080,framerate=15/1",

            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format={gbrg16,bggr16,rggb16,grbg16},width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format={gbrg16,bggr16,rggb16,grbg16},width=1920,height=1080,framerate=15/1",

            "video/x-raw,width=1920,height=1080,framerate=30/1",
            "video/x-raw,width=1920,height=1080,framerate=15/1",

            "video/x-bayer,format=pwl-rggb12m,width=640,height=480,framerate=30/1",
            "video/x-bayer,format=pwl-rggb12,width=640,height=480,framerate=30/1",
            "video/x-bayer,format=pwl-rggb16H12,width=640,height=480,framerate=30/1",
            "video/x-bayer,format=pwl-rggb12m,width=640,height=480,framerate=15/1",
            "video/x-bayer,format=pwl-rggb12,width=640,height=480,framerate=15/1",
            "video/x-bayer,format=pwl-rggb16H12,width=640,height=480,framerate=15/1",

            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=640,height=480,framerate=30/1",
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=640,height=480,framerate=15/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=640,height=480,framerate=30/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=640,height=480,framerate=15/1",
            "video/x-bayer,format={gbrg16,bggr16,rggb16,grbg16},width=640,height=480,framerate=30/1",
            "video/x-bayer,format={gbrg16,bggr16,rggb16,grbg16},width=640,height=480,framerate=15/1",

            "video/x-raw,width=640,height=480,framerate=30/1",
            "image/jpeg,width=1920,height=1080,framerate=30/1",
        };

    GstCaps* ret = nullptr;
    for (const auto& d : defaults)
    {
        GstCaps* test = gst_caps_from_string(d);

        if (gst_caps_can_intersect(intersect, test))
        {
            ret = gst_caps_intersect(intersect, test);
            gst_caps_unref(test);
            break;
        }
        gst_caps_unref(test);
    }
    if (!ret)
    {
        return gst_caps_copy(intersect);
    }
    return ret;
}

bool Caps::has_resolution_ranges() const
{
    std::string s = gst_caps_to_string(p_caps);

    auto it = s.find("width=(int)[");
    auto it2 = s.find("height=(int)[");

    if (it != std::string::npos || it2 != std::string::npos)
    {
        return true;
    }
    return false;
}


std::vector<std::string> Caps::get_formats() const
{
    std::vector<std::string> ret;

    for (const auto& f : formats)
    {
        auto iter = std::find_if(ret.begin(), ret.end(), [&f] (const std::string& f2)
        {
            if (f.format == f2)
            {
                return true;
            }
            return false;
        });
        if (iter == ret.end())
        {
            ret.push_back(f.format);
        }
    }

    return ret;
}


std::string Caps::get_gst_name(const std::string& format) const
{
    for (const auto& f : formats)
    {
        if (format == f.format)
        {
            return f.gst_format;
        }
    }

    return "";
}


std::vector<std::pair<uint, uint>> Caps::get_resolutions(const std::string& format, const scaling& scale) const
{
    std::vector<std::pair<uint, uint>> ret;

    for (const auto& f : formats)
    {
        if (f.format == format && f.scale == scale)
        {
            for (const auto& res : f.resolutions)
            {
                std::string s = std::to_string(res.width) + "x" + std::to_string(res.height);
                std::pair<uint, uint> p = std::pair<uint, uint>(res.width, res.height);
                ret.push_back(p);
            }
        }
    }

    return ret;
}


std::vector<double> Caps::get_framerates(const std::string& format,
                                         const scaling& scale,
                                         unsigned int width,
                                         unsigned int height) const
{
    for (const auto& f : formats)
    {
        if (f.format == format && f.scale == scale)
        {
            for (const auto& res : f.resolutions)
            {
                if (res.width == width && res.height == height)
                {
                    return res.framerates;
                }
            }
        }
    }

    return {};
}


std::vector<struct caps_format> Caps::generate_from_fixed_caps(GstElement& element)
{
    std::vector<struct caps_format> tmp;

    for (unsigned int i = 0; i < gst_caps_get_size(p_caps); ++i)
    {
        caps_format* fmt;
        int width;
        int height;

        GstStructure* structure = gst_caps_get_structure(p_caps, i);

        const char* name = gst_structure_get_name(structure);

        if (gst_structure_has_field(structure, "format"))
        {
            if (gst_structure_get_field_type(structure, "format") == G_TYPE_STRING)
            {
                const char* format = gst_structure_get_string(structure, "format");

                auto scale = create_scale(structure);

                auto fmt_struct = find_format(tmp, format, scale);

                if (fmt_struct == tmp.end())
                {
                    // new format
                    struct caps_format t = {};
                    tmp.push_back(t);

                    fmt = &(tmp.back());
                }
                else
                {
                    fmt = &(*fmt_struct);
                }

                fmt->format = format;
                fmt->gst_format = name;
                fmt->gst_format += ",format=";
                fmt->gst_format += format;
                fmt->scale = scale;
            }
            else if (gst_structure_get_field_type(structure, "format") == GST_TYPE_LIST)
            {
                qWarning("gst structure format list not implemented");
                continue;
            }
            else
            {
                qWarning("format handling not implemented for unexpected type in strcture: %s\n",
                         gst_structure_to_string(structure));
                continue;
            }

        }
        else
        {
            // only name
            // typically for things like AFU050 => jpeg

            auto scale = create_scale(structure);

            auto fmt_struct = find_format(tmp, name, scale);

            if (fmt_struct == tmp.end())
            {
                // new format
                struct caps_format t = {};
                tmp.push_back(t);

                fmt = &(tmp.back());
            }
            else
            {
                fmt = &(*fmt_struct);
            }

            fmt->format = name;
            fmt->gst_format = name;
            fmt->scale = scale;
        }


        struct caps_resolution res;

        GType width_type = gst_structure_get_field_type(structure, "width");

        if (width_type == GST_TYPE_INT_RANGE)
        {
            //qWarning("NOT IMPLEMENTED\n");
            continue;
        }
        else
        {
            gboolean ret = gst_structure_get_int(structure, "width", &width);

            if (!ret)
            {
                qWarning("Unable to query width\n");
                continue;
            }
        }

        GType height_type = gst_structure_get_field_type(structure, "height");

        if (height_type == GST_TYPE_INT_RANGE)
        {
            //qWarning("NOT IMPLEMENTED\n");
            continue;
        }
        else
        {
            gboolean ret = gst_structure_get_int(structure, "height", &height);

            if (!ret)
            {
                qWarning("Unable to query height\n");
                continue;
            }
        }

        res.width = width;
        res.height = height;

        const GValue* framerate = gst_structure_get_value(structure, "framerate");

        res.framerates = index_framerates(element, fmt->gst_format, res, framerate);

        fmt->resolutions.push_back(res);
    }

    return tmp;
}


std::vector<struct caps_format> Caps::generate_from_caps_list(GstElement& element)
{
    std::vector<struct caps_format> tmp;

    for (unsigned int i = 0; i < gst_caps_get_size(p_caps); ++i)
    {
        caps_format* fmt;
        // int width;
        // int height;

        GstStructure* structure = gst_caps_get_structure(p_caps, i);

        GType height_type = gst_structure_get_field_type(structure, "height");
        GType width_type = gst_structure_get_field_type(structure, "width");

        if (width_type != GST_TYPE_INT_RANGE || height_type != GST_TYPE_INT_RANGE)
        {
            continue;
        }

        const char* name = gst_structure_get_name(structure);

        if (gst_structure_has_field(structure, "format"))
        {
            if (gst_structure_get_field_type(structure, "format") == G_TYPE_STRING)
            {
                const char* format = gst_structure_get_string(structure, "format");

                auto scale = create_scale(structure);

                auto fmt_struct = find_format(tmp, format, scale);

                if (fmt_struct == tmp.end())
                {
                    // new format
                    struct caps_format t = {};
                    tmp.push_back(t);

                    fmt = &(tmp.back());
                }
                else
                {
                    fmt = &(*fmt_struct);
                }

                fmt->format = format;
                fmt->gst_format = name;
                fmt->gst_format += ",format=";
                fmt->gst_format += format;
                fmt->scale = scale;
            }
            else if (gst_structure_get_field_type(structure, "format") == GST_TYPE_LIST)
            {
                qWarning("gst structure format list not implemented");
                continue;
            }
            else
            {
                qWarning("format handling not implemented for unexpected type in strcture: %s\n",
                         gst_structure_to_string(structure));
                continue;
            }
        }

        image_size min = {};
        image_size max = {};
        image_size step = {};

        const GValue* width_value = gst_structure_get_value(structure, "width");

        min.width = gst_value_get_int_range_min(width_value);
        max.width = gst_value_get_int_range_max(width_value);
        step.width = gst_value_get_int_range_step(width_value);

        const GValue* height_value = gst_structure_get_value(structure, "height");

        min.height = gst_value_get_int_range_min(height_value);
        max.height = gst_value_get_int_range_max(height_value);
        step.height = gst_value_get_int_range_step(height_value);


        auto resolutions = get_standard_resolutions(min, max, step);

        // ensure both min and max are in the listing
        if (!std::any_of(resolutions.begin(), resolutions.end(),
                         [&min](const image_size& res) {return res == min;}))
        {
            resolutions.insert(resolutions.begin(), min);
        }

        if (!std::any_of(resolutions.begin(), resolutions.end(),
                         [&max](const image_size& res) { return res == max; }))
        {
            resolutions.push_back(max);
        }

        const GValue* framerate = gst_structure_get_value(structure, "framerate");

        for (const auto& resolution : resolutions )
        {
            struct caps_resolution res;

            res.width = resolution.width;
            res.height =resolution.height;

            res.framerates = index_framerates(element, fmt->gst_format, res, framerate);
            fmt->resolutions.push_back(res);
        }

    }

    return tmp;
}

void Caps::generate(GstElement& element)
{
    if (has_resolution_ranges())
    {
        formats = generate_from_caps_list(element);
    }
    else
    {
        formats = generate_from_fixed_caps(element);
    }
}

bool Caps::has_binning() const
{
    for (guint i = 0; i < gst_caps_get_size(p_caps); i++)
    {
        // transfer: none
        auto struc = gst_caps_get_structure(p_caps, i);
        if (gst_structure_has_field(struc, "binning"))
        {
            return true;
        }
    }
    return false;
}

bool Caps::has_skipping() const
{
    for (guint i = 0; i < gst_caps_get_size(p_caps); i++)
    {
        // transfer: none
        auto struc = gst_caps_get_structure(p_caps, i);
        if (gst_structure_has_field(struc, "skipping"))
        {
            return true;
        }
    }
    return false;
}


std::vector<std::string> Caps::get_binning(const std::string& format) const
{
    std::vector<std::string> ret;

    for (const auto& f : formats)
    {
        if (f.format == format)
        {
            if (std::find(ret.begin(), ret.end(), f.scale.binning_str()) == ret.end())
            {
                ret.push_back(f.scale.binning_str());
            }
        }
    }

    return ret;
}


std::vector<std::string> Caps::get_skipping(const std::string& format) const
{
    std::vector<std::string> ret;

    for (const auto& f : formats)
    {
        if (f.format == format)
        {
            ret.push_back(f.scale.skipping_str());
        }
    }

    return ret;
}
