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

#include "formats.h"

#include "general.h"

#include <gst/gst.h> // gst_init
#include <gst/base/gstbasetransform.h> // GST_BASE_TRANSFORM
#include <iomanip>
#include <iostream>
#include <regex>


namespace
{

void print_caps(const GstCaps& caps, const std::string& indent = "")
{
    char* cstr = gst_caps_to_string(&caps);
    std::string str = cstr;
    g_free(cstr);
    // use a regex to insert line breaks for increased readability
    std::regex e("; ");
    std::regex wb_e(" ");
    std::regex paren_e("\\((string|int|fraction)\\)");

    std::string end_replacement = ";\n" + indent;

    std::string caps_str = std::regex_replace(str, e, end_replacement);
    caps_str = std::regex_replace(caps_str, wb_e, "");
    caps_str = std::regex_replace(caps_str, paren_e, "");

    std::cout << indent << caps_str << std::endl;
}


void print_transform_caps(GstBaseTransform* base,
                          GstPadDirection direction,
                          GstCaps* caps,
                          const std::string& indent="")
{
    auto base_class = GST_BASE_TRANSFORM_GET_CLASS(base);
    auto ret = base_class->transform_caps(base, direction, caps, nullptr);

    if (ret)
    {
        print_caps(*ret, indent);

        gst_caps_unref(ret);
    }
    else
    {
        std::cerr << "Element query returned NULL." << std::endl;
    }
}


void print_conversion_table(GstBaseTransform* base)
{
    auto _print = [=] (GstCaps* caps, GstPadDirection direction)
    {
        for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
        {
            auto tmp = gst_helper::make_consume_ptr<GstCaps>(gst_caps_copy_nth(caps, i));

            // is internal, no free
            GstStructure* struc = gst_caps_get_structure(tmp.get(), 0);

            if (gst_structure_get_field_type(struc, "format") == GST_TYPE_LIST)
            {
                const GValue* val = gst_structure_get_value(struc, "format");

                for (unsigned int x = 0; x < gst_value_list_get_size(val); ++x)
                {
                    const GValue* format = gst_value_list_get_value(val, x);

                    GstCaps* tmp_fmt_caps = gst_caps_copy(tmp.get());
                    gst_caps_set_value(tmp_fmt_caps, "format", format);

                    print_caps(*tmp_fmt_caps);
                    print_transform_caps(base, direction, tmp_fmt_caps, "\t");

                    std::cout << std::endl;
                }
            }
            else
            {
                print_caps(*tmp);

                print_transform_caps(base, direction, tmp.get(), "\t");

                std::cout << std::endl;
            }
        }

    };

    std::cout << "Element converts INCOMING caps as follows:" << std::endl << std::endl;

    auto in = gst_helper::make_ptr<GstPad>(gst_element_get_static_pad(GST_ELEMENT(base), "sink"));
    auto caps_in = gst_helper::make_ptr<GstCaps>(gst_pad_query_caps(in.get(), NULL));

    _print(caps_in.get(), GST_PAD_SINK);

    std::cout << "Element expects input for caps COMING OUT as follows:" << std::endl << std::endl;

    auto out = gst_helper::make_ptr<GstPad>(gst_element_get_static_pad(GST_ELEMENT(base), "src"));
    auto caps_out = gst_helper::make_ptr<GstCaps>(gst_pad_query_caps(out.get(), NULL));

    _print(caps_out.get(), GST_PAD_SRC);
}

} // namespace

void tcam::tools::ctrl::list_gstreamer_1_0_formats (const std::string& serial)
{
    GstElement* source = gst_element_factory_make("tcamsrc", "source");

    if (!source)
    {
        std::cerr << "Unable to create source element." << std::endl;
        return;
    }

    if (!is_valid_device_serial(serial))
    {
        std::cerr << "Device with given serial does not exist." << std::endl;
        return;
    }

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);

    gst_element_set_state(source, GST_STATE_READY);

    GstPad* pad = gst_element_get_static_pad(source, "src");

    GstCaps* caps = gst_pad_query_caps(pad, NULL);

    char* cstr = gst_caps_to_string(caps);
    std::string str = cstr;
    g_free(cstr);

    if (caps)
    {
        std::cout << "Available gstreamer-1.0 caps:" << std::endl;

        print_caps(*caps);
    }
    else
    {
        std::cerr << "Unable to display caps. Conversion failed." << str << std::endl;
    }

    gst_caps_unref(caps);
    gst_object_unref(pad);

    gst_element_set_state(source, GST_STATE_NULL);

    gst_object_unref(source);
}


int tcam::tools::ctrl::convert(const std::string& element_name,
                               ElementPadDirection user_direction,
                               const std::string& caps_str)
{
    std::cout << "Probing " << element_name << ":" << std::endl;

    auto element = open_element(element_name.c_str());

    if (!element)
    {
        return 1;
    }

    auto caps = gst_helper::make_consume_ptr<GstCaps>(gst_caps_from_string(caps_str.c_str()));

    if (GST_IS_BASE_TRANSFORM(element.get()))
    {
        auto base = GST_BASE_TRANSFORM_CAST(element.get());

        // tcansform_caps is an optional function
        // all TIS elements have it. Others might not.
        if (GST_BASE_TRANSFORM_GET_CLASS(base)->transform_caps)
        {
            {
                ElementStateGuard state_guard(*element.get());
                state_guard.set_state(GST_STATE_READY);

                if (user_direction == ElementPadDirection::Both)
                {
                    print_conversion_table(base);
                }
                else
                {
                    GstPadDirection direction = GST_PAD_UNKNOWN;

                    if (user_direction == ElementPadDirection::Out)
                    {
                        direction = GST_PAD_SRC;
                    }
                    else if (user_direction == ElementPadDirection::In)
                    {
                        direction = GST_PAD_SINK;
                    }

                    print_transform_caps(base, direction, caps.get());
                }
            } // state_guard
        }
        else
        {
            std::cerr << "Element does not has transform_caps. Query not possible." << std::endl;
            return 2;
        }
    }

    return 0;
}
