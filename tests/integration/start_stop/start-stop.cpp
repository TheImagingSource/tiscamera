/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#include <CLI11.hpp>
#include <cstdio>
#include <gst/gst.h>
#include <map>
#include <unistd.h>


GstElement* pipeline;

static void create_pipeline()
{
    GError* err = NULL;
    pipeline = gst_parse_launch("tcambin name=tcam \
                                ! videoconvert \
                                ! fakesink",
                                &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline.\n");
        return;
    }
}


static void set_caps(const std::string& caps)
{
    GstElement* tcam = gst_bin_get_by_name(GST_BIN(pipeline), "tcam");

    if (!tcam)
    {
        printf("Unable to retrieve tcambin from pipeline.\n");
        exit(1);
    }

    g_object_set(G_OBJECT(tcam), "device-caps", caps.c_str(), nullptr);
}


static void set_serial(const char* serial)
{
    auto tcam = gst_bin_get_by_name(GST_BIN(pipeline), "tcam");

    if (tcam)
    {
        g_object_set(G_OBJECT(tcam), "serial", serial, nullptr);
    }
    else
    {
        printf("Unable to retrieve tcambin from pipeline.\n");
        exit(1);
    }
}


int main(int argc, char* argv[])
{
    CLI::App app { "start-stop test" };

    std::string serial;
    app.add_option("-s,--serial", serial, "Serial number of the file that shall be used.", false);

    std::string caps_str;
    app.add_option("-c,--caps", caps_str, "GStreamer caps the device shall use.", false);

    GstState rest_state { GST_STATE_NULL };

    std::map<std::string, GstState> state_map { { "NULL", GST_STATE_NULL },
                                                { "READY", GST_STATE_READY } };

    app.add_option("-r,--rest", rest_state, "\"Stop\" state that shall be used", "NULL")
        ->transform(CLI::CheckedTransformer(state_map, CLI::ignore_case));

    // allow --gst-debug etc
    app.allow_extras(true);

    CLI11_PARSE(app, argc, argv);

    //
    // actual test
    //

    gst_init(&argc, &argv);

    create_pipeline();

    if (!serial.empty())
    {
        set_serial(serial.c_str());
    }

    if (!caps_str.empty())
    {
        set_caps(caps_str);
    }

    for (unsigned int i = 0; i < 5; ++i)
    {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);

        sleep(5);

        gst_element_set_state(pipeline, rest_state);
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);

    return 0;
}
