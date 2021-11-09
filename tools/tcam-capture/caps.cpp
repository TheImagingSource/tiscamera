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

#include "tcamgstbase.h"
#include <vector>
#include <string>



namespace {


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

}



Caps::Caps(GstCaps* caps)
    : p_caps(gst_caps_copy(caps))
{
    generate();
}


GstCaps* Caps::get_default_caps() const
{
    // for performance sake, prefer bayer 8-bit
    static const char* defaults [] =
        {
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=1920,height=1080,framerate=30/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=1920,height=1080,framerate=15/1",
            "video/x-raw,width=1920,height=1080,framerate=30/1",
            "video/x-raw,width=1920,height=1080,framerate=15/1",
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=640,height=480,framerate=30/1",
            "video/x-bayer,format={gbrg,bggr,rggb,grbg},width=640,height=480,framerate=15/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=640,height=480,framerate=30/1",
            "video/x-bayer,format={gbrg10m,bggr10m,rggb10m,grbg10m},width=640,height=480,framerate=15/1",
            "video/x-raw,width=640,height=480,framerate=30/1",
            "image/jpeg,width=1920,height=1080,framerate=30/1",
        };

    GstCaps* ret = nullptr;
    for (const auto& d : defaults)
    {
        GstCaps* test = gst_caps_from_string(d);

        if (gst_caps_can_intersect(p_caps, test))
        {
            ret = gst_caps_intersect(p_caps, test);
            gst_caps_unref(test);
            break;
        }
        gst_caps_unref(test);
    }
    if (!ret)
    {
        return gst_caps_copy(p_caps);
    }
    return ret;
}


single_format Caps::get_default_format() const
{
    return m_default_settings;
}


bool Caps::has_resolution_ranges() const
{
    std::string s = gst_caps_to_string(p_caps);

    auto it = s.find("[");

    if (it != std::string::npos)
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
            return f.gst_format;
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
    std::vector<double> ret;

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

    return ret;
}


void Caps::generate()
{
    m_default_settings = {};
    std::vector<struct caps_format> tmp;

    auto find_format = [&tmp](const std::string& name, const scaling& scale)
    {
        return std::find_if(tmp.begin(), tmp.end(), [&name, &scale](const struct caps_format& fmt)
        {
            if (fmt.format == name
                    && fmt.scale == scale)
            {
                return true;
            }
            return false;
        });
    };

    auto create_scale = [] (const GstStructure* structure) -> scaling
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
    };

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

                auto fmt_struct = find_format(format, scale);

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

            auto fmt_struct = find_format(name, scale);

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
            res.is_range = false;
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

        std::vector<double> framerates;
        if (G_VALUE_TYPE(framerate) == GST_TYPE_LIST)
        {
            for (unsigned int x = 0; x < gst_value_list_get_size(framerate); ++x)
            {
                const GValue* val = gst_value_list_get_value(framerate, x);

                if (G_VALUE_TYPE(val) == GST_TYPE_FRACTION)
                {
                    int num = gst_value_get_fraction_numerator(val);
                    int den = gst_value_get_fraction_denominator(val);

                    framerates.push_back(num / den);
                }
                else
                {
                    qWarning(
                        "Handling of framerate handling not implemented for non fraction types.\n");
                    break;
                }
            }
            res.framerates = framerates;
            res.is_fps_range = false;
        }
        else if (G_VALUE_TYPE(framerate) == GST_TYPE_FRACTION_RANGE)
        {
            const GValue* min = gst_value_get_fraction_range_min(framerate);
            const GValue* max = gst_value_get_fraction_range_max(framerate);

            int num = gst_value_get_fraction_numerator(min);
            int den = gst_value_get_fraction_denominator(min);

            double fps_min;
            gst_util_fraction_to_double(num, den, &fps_min);

            num = gst_value_get_fraction_numerator(max);
            den = gst_value_get_fraction_denominator(max);

            double fps_max;
            gst_util_fraction_to_double(num, den, &fps_max);

            res.framerates = create_steps_for_range(fps_min, fps_max);
            res.is_fps_range = true;
        }

        fmt->resolutions.push_back(res);
    }

    formats = tmp;
}


GstCaps* Caps::find_caps(const QString, // format
                         int, // width
                         int, // height
                         double) const // framerate
{
    qWarning("NOT IMPLEMENTED");

    return nullptr;
}

const GstCaps& Caps::get_caps() const
{
    return *p_caps;
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
