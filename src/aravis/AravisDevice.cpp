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

#include "AravisDevice.h"

#include "AravisPropertyBackend.h"
#include "aravis_utils.h"
#include "../internal.h"
#include "../utils.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <dutils_img/fcc_to_string.h> // img::fcc_to_string


using namespace tcam;


AravisDevice::AravisFormatHandler::AravisFormatHandler(AravisDevice* dev) : device(dev) {}


std::vector<double> AravisDevice::AravisFormatHandler::get_framerates(
    const struct tcam_image_size& s,
    int pixelformat)
{
    std::vector<double> ret;
    auto dev = arv_camera_get_device(device->arv_camera);

    // TODO implement better way to check for availability
    GError* error = NULL;
    double min = 0.0;
    double max = 0.0;

    if (pixelformat != 0)
    {
        arv_device_set_integer_feature_value(
            dev, "TestPixelFormat", fourcc2aravis(pixelformat), &error);
        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }

        arv_device_set_integer_feature_value(dev, "TestWidth", s.width, &error);
        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }
        arv_device_set_integer_feature_value(dev, "TestHeight", s.height, &error);
        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }
        min = arv_device_get_float_feature_value(dev, "ResultingMinFPS", &error);
        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }
        max = arv_device_get_float_feature_value(dev, "ResultingMaxFPS", &error);
        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }
    }

    if (min == 0.0 && max == 0.0)
    {
        if (this->device->stream)
        {
            // this means a stream is active
            // do not touch settings
            return ret;
        }

        auto active_format = arv_camera_get_pixel_format(this->device->arv_camera, &error);

        arv_camera_set_pixel_format(this->device->arv_camera, fourcc2aravis(pixelformat), &error);

        // this means TestPixelFormat, TestWidth, TestHeight are not available
        // could be an UsbVision camera
        int x1, x2, y1, y2;
        arv_camera_get_region(this->device->arv_camera, &x1, &y1, &x2, &y2, &error);

        unsigned int height = y2 - y1;
        unsigned int width = x2 - x1;

        arv_camera_set_region(this->device->arv_camera, 0, 0, s.width, s.height, &error);

        arv_camera_get_frame_rate_bounds(this->device->arv_camera, &min, &max, &error);

        if (error)
        {
            g_error_free(error);
            error = nullptr;
        }

        arv_camera_set_pixel_format(this->device->arv_camera, active_format, &error);

        arv_camera_set_region(this->device->arv_camera, 0, 0, width, height, &error);
    }

    if (min == 0.0 && max == 0.0)
    {
        // this means either the camera is broken or we have a FPS enum
        // hope for the second and try it
        guint n_fps_values = 0;
        auto fps_values =
            arv_device_dup_available_enumeration_feature_values(dev, "FPS", &n_fps_values, &error);

        if (n_fps_values == 0)
        {
            // alternative failed
            // return empty vector and let format handle it
            SPDLOG_ERROR("Unable to determine what framerate settings are used.");
            return ret;
        }

        ret.reserve(n_fps_values);

        for (unsigned int i = 0; i < n_fps_values; ++i)
        {
            auto val = fps_values + i;

            ret.push_back((int)((10000000 / (double)*val) * 100 + 0.5) / 100.0);
        }

        if (fps_values)
        {
            g_free(fps_values);
        }

        // TestWidth, TestHeight do not exists.
        // return empty vector and let format handle it
        return ret;
    }

    SPDLOG_TRACE("Queried: {}x{} fourcc {} Received min: {} max {}",
                 s.width,
                 s.height,
                 img::fcc_to_string(pixelformat),
                 min,
                 max);

    ret = create_steps_for_range(min, max);

    return ret;
}


AravisDevice::AravisDevice(const DeviceInfo& device_desc) : stream(NULL)
{
    device = device_desc;
    GError* err = NULL;

    this->arv_camera = arv_camera_new(this->device.get_info().identifier, &err);
    if (err)
    {
        SPDLOG_ERROR("Error while creating arv_camera: {}", err->message);
        g_clear_error(&err);
    }
    if (this->arv_camera == NULL)
    {
        throw std::runtime_error("Error while creating ArvCamera");
    }

    if (arv_camera_is_gv_device(this->arv_camera))
    {

        if (!arv_gv_device_is_controller((ArvGvDevice*)arv_camera_get_device(this->arv_camera)))
        {
            SPDLOG_ERROR("This process does not control the device!");
            throw std::runtime_error("Device already open.");
        }
        auto_set_packet_size();
        auto_set_control_lifetime();
    }

    format_handler = std::make_shared<AravisFormatHandler>(this);

    m_backend =
        std::make_shared<tcam::property::AravisPropertyBackend>(arv_camera_get_device(arv_camera));

    index_genicam();
    determine_active_video_format();

    // make aravis notify us when the device can not be reached
    g_signal_connect(
        arv_camera_get_device(arv_camera), "control-lost", G_CALLBACK(device_lost), this);
}


AravisDevice::~AravisDevice()
{
    if (arv_camera != NULL)
    {
        SPDLOG_INFO("Destroying arvcamera");
        g_object_unref(arv_camera);
        arv_camera = NULL;
    }
}


DeviceInfo AravisDevice::get_device_description() const
{
    return device;
}


bool AravisDevice::set_video_format(const VideoFormat& new_format)
{
    if (is_lost)
    {
        return false;
    }

    SPDLOG_DEBUG("Setting format to '{}'", new_format.to_string().c_str());
    GError* err = nullptr;

    // // arv_camera_set_frame_rate overwrites TriggerSelector and TriggerMode
    // // set them again after changing the framerate to ensure consistent behaviour
    const char* trig_selector = arv_device_get_string_feature_value(
        arv_camera_get_device(arv_camera), "TriggerSelector", &err);

    if (err)
    {
        SPDLOG_ERROR("Caught error: {}", err->message);
        g_clear_error(&err);

    }

    const char* trig_mode =
        arv_device_get_string_feature_value(arv_camera_get_device(arv_camera), "TriggerMode", &err);
    if (err)
    {
        SPDLOG_ERROR("Caught error: {}", err->message);
        g_clear_error(&err);

    }

    arv_camera_set_frame_rate(this->arv_camera, new_format.get_framerate(), &err);

    if (err)
    {
        SPDLOG_ERROR("Caught error: {}", err->message);
        g_clear_error(&err);

    }
    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera), "TriggerSelector", trig_selector, &err);
    if (err)
    {
        SPDLOG_ERROR("Caught error: {}", err->message);
        g_clear_error(&err);

    }
    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera), "TriggerMode", trig_mode, &err);

    if (err)
    {
        SPDLOG_ERROR("Caught error: {}", err->message);
        g_clear_error(&err);

    }
    arv_camera_set_pixel_format(this->arv_camera, fourcc2aravis(new_format.get_fourcc()), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set pixel format: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    // TODO: auto center

    unsigned int offset_x = 0;
    unsigned int offset_y = 0;

    arv_camera_set_region(this->arv_camera,
                          offset_x,
                          offset_y,
                          new_format.get_size().width,
                          new_format.get_size().height,
                          &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to set region: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    set_scaling(new_format.get_scaling());

    determine_active_video_format();

    return true;
}


VideoFormat AravisDevice::get_active_video_format() const
{
    return active_video_format;
}


void AravisDevice::determine_active_video_format()
{
    GError* err = nullptr;

    this->active_video_format.set_framerate(arv_camera_get_frame_rate(this->arv_camera, &err));

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve frame rate: {}", err->message);
        g_clear_error(&err);
    }

    active_video_format.set_fourcc(
        aravis2fourcc(arv_camera_get_pixel_format(this->arv_camera, &err)));
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve pixel format: {}", err->message);
        g_clear_error(&err);
    }

    int x1, x2, y1, y2;
    arv_camera_get_region(this->arv_camera, &x1, &y1, &x2, &y2, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve region: {}", err->message);
        g_clear_error(&err);
        return;
    }

    unsigned int height = y2 - y1;
    unsigned int width = x2 - x1;

    active_video_format.set_size(width, height);

    active_video_format.set_scaling(get_current_scaling());
}


std::vector<VideoFormatDescription> AravisDevice::get_available_video_formats()
{
    if (this->available_videoformats.empty())
    {
        this->index_genicam();
    }

    return available_videoformats;
}

void AravisDevice::auto_set_control_lifetime()
{
    std::string env_packet_size = tcam::get_environment_variable("TCAM_GIGE_HEARTBEAT_MS", "0");

    int hearbeat_ms = 0;

    try
    {
        hearbeat_ms = std::stoi(env_packet_size);
    }
    catch (...)
    {
        SPDLOG_WARN("Unable to interpret the value for TCAM_GIGE_HEARTBEAT_MS. Falling back to "
                    "default values.");
    }

    if (hearbeat_ms != 0)
    {
        arv_camera_set_integer(arv_camera, "GevHeartbeatTimeout", hearbeat_ms, NULL);
        SPDLOG_DEBUG("Setting heartbeat timeout to {} ms.", hearbeat_ms);
    }
    else
    {
        static const int default_heartbeat_ms = 3000;
        arv_camera_set_integer(arv_camera, "GevHeartbeatTimeout", default_heartbeat_ms, NULL);

        SPDLOG_DEBUG("Setting heartbeat timeout to default {} ms.", default_heartbeat_ms);
    }
}


void AravisDevice::auto_set_packet_size()
{
    std::string env_packet_size = tcam::get_environment_variable("TCAM_GIGE_PACKET_SIZE", "0");

    int eps = 0;

    try
    {
        eps = std::stoi(env_packet_size);
    }
    catch (...)
    {
        SPDLOG_WARN("Unable to interpret the value for TCAM_GIGE_PACKET_SIZE. Falling back to "
                    "default values.");
    }

    GError* err = nullptr;

    if (eps == 0)
    {
        guint packet_size = arv_camera_gv_auto_packet_size(this->arv_camera, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to determine auto packet size: {}", err->message);
            g_clear_error(&err);
            return;
        }
        SPDLOG_INFO("Automatically set packet size to {} bytes", packet_size);
    }
    else
    {
        arv_camera_gv_set_packet_size(arv_camera, eps, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to set packet size: {}", err->message);
            g_clear_error(&err);
            return;
        }
        SPDLOG_INFO("Set packet size according to environment to: {}", eps);
    }
}


void AravisDevice::device_lost(ArvGvDevice* device __attribute__((unused)), void* user_data)
{
    AravisDevice* self = (AravisDevice*)user_data;

    self->is_lost = true;

    self->notify_device_lost();
}


//// genicam handling

void AravisDevice::index_genicam()
{
    if (this->arv_camera == nullptr)
    {
        return;
    }
    genicam = arv_device_get_genicam(arv_camera_get_device(this->arv_camera));

    iterate_genicam("Root");
    index_properties("Root");
    determine_active_video_format();
    index_genicam_format(NULL);
    set_video_format(this->active_video_format);
}


void AravisDevice::iterate_genicam(const char* feature)
{


    ArvGcNode* node = arv_gc_get_node(genicam, feature);

    if (ARV_IS_GC_FEATURE_NODE(node)
        && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), NULL)
        && arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL))
    {

        if (ARV_IS_GC_CATEGORY(node))
        {
            const GSList* features;
            const GSList* iter;

            features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));

            for (iter = features; iter != NULL; iter = iter->next)
            {
                iterate_genicam((char*)iter->data);
            }
            return;
        }

        static std::vector<std::string> format_member = { "AcquisitionStart",
                                                          "AcquisitionStop",
                                                          "AcquisitionMode",
                                                          // "Binning",
                                                          "SensorWidth",
                                                          "SensorHeight",
                                                          "Width",
                                                          "Height",
                                                          "FPS",
                                                          "AcquisitionFrameRate",
                                                          "PixelFormat" };


        // is part of the format description
        if (std::find(format_member.begin(), format_member.end(), feature)
                 != format_member.end())
        {
            // index_genicam_format(camera, node, frmt_mapping);
            this->format_nodes.push_back(node);
        }
    }
}


void AravisDevice::index_genicam_format(ArvGcNode* /* node */)
{
    // storing current setting and restoring them is done
    // by the caller.
    generate_scales();

    // genicam formats behave like follows:
    // All framerates are valid for all frame sizes
    // All frame sizes are valid for all formats

    // we index genicam our selves because the aravis api does not allow
    // the retrieval of all required information
    // e.g. if the camera only offers certain framerates we want to know them

    // the introduction of standard resolutions is not done here
    // but in the gstreamer source
    // here a simple device representation is created

    GError* err = nullptr;

    // We search for the wanted node and save the intermediate result
    std::string node_to_use;
    auto find_node = [&node_to_use](ArvGcNode* node) {
        return (node_to_use.compare(arv_gc_feature_node_get_name((ArvGcFeatureNode*)node)) == 0);
    };

    // current setting that have to be restored
    determine_active_video_format();
    auto reset_format = this->active_video_format;


    // reset region to entirety of the sensor
    // depending on the camera model (z12)
    // the width/height boundaries may be skewed
    // due to internal auto offset settings
    ArvDevice* dev = arv_camera_get_device(this->arv_camera);
    arv_camera_set_region(arv_camera,
                          0,
                          0,
                          arv_device_get_integer_feature_value(dev, "SensorWidth", &err),
                          arv_device_get_integer_feature_value(dev, "SensorHeight", &err),
                          &err);

    // work your way from bottom to top
    // start with frame rates and use everthing until all format descriptions are complete

    node_to_use = "FPS";
    auto fps_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);

    if (fps_node == format_nodes.end())
    {
        node_to_use = "AcquisitionFrameRate";
        fps_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);
    }

    std::vector<double> fps;

    // TODO are there cameras that only have fixed resolutions?

    int width_min = 0;
    int width_max = 0;
    int width_step = 0;

    int height_min = 0;
    int height_max = 0;
    int height_step = 0;

    node_to_use = "PixelFormat";

    auto pixel_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);

    if (pixel_node != format_nodes.end())
    {

        // we know there are formats
        // use official API to retrieve info
        // if needed we can perform additional genicam interpretation here

        unsigned int n_formats;
        gint64* format_ptr =
            arv_camera_dup_available_pixel_formats(this->arv_camera, &n_formats, &err);

        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve available pixel formats: {}", err->message);
            g_clear_error(&err);
        }

        unsigned n2_formats;
        const char** format_str =
            arv_camera_dup_available_pixel_formats_as_strings(this->arv_camera, &n2_formats, &err);

        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve pixel format description strings: {}", err->message);
            g_clear_error(&err);
        }

        if (n_formats != n2_formats)
        {
            SPDLOG_ERROR("Format retrieval encountered nonsensical information");
        }

        for (unsigned int i = 0; i < n_formats; ++i)
        {
            struct tcam_video_format_description desc = {};

            desc.fourcc = format_ptr[i];

            desc.fourcc = aravis2fourcc(desc.fourcc);

            if (desc.fourcc == 0)
            {
                SPDLOG_ERROR(
                    "Input format no supported! \"{:x}\" - {}", format_ptr[i], format_str[i]);
                continue;
            }

            strncpy(desc.description, format_str[i], sizeof(desc.description) - 1);

            desc.resolution_count = 1;
            framerate_mapping rf = {};

            arv_camera_set_pixel_format(this->arv_camera, format_ptr[1], &err);

            if (err)
            {
                SPDLOG_ERROR("%s", err->message);
                g_error_free(err);
                err = nullptr;
                continue;
            }

            arv_camera_get_width_bounds(this->arv_camera, &width_min, &width_max, &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve width bounds: {}", err->message);
                g_clear_error(&err);
            }

            arv_camera_get_height_bounds(this->arv_camera, &height_min, &height_max, &err);

            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve height bounds: {}", err->message);
                g_clear_error(&err);
            }

            width_step = arv_camera_get_width_increment(this->arv_camera, &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve height bounds: {}", err->message);
                g_clear_error(&err);
            }

            height_step = arv_camera_get_height_increment(this->arv_camera, &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve height bounds: {}", err->message);
                g_clear_error(&err);
            }

            tcam_image_size min = { (unsigned int)width_min, (unsigned int)height_min };
            tcam_image_size max = { (unsigned int)width_max, (unsigned int)height_max };


            rf.resolution.max_size = max;
            rf.resolution.min_size = min;

            if (min == max)
            {
                rf.resolution.type = TCAM_RESOLUTION_TYPE_FIXED;
            }
            else
            {
                rf.resolution.type = TCAM_RESOLUTION_TYPE_RANGE;
                rf.resolution.width_step_size = width_step;
                rf.resolution.height_step_size = height_step;
            }

            fps = this->format_handler->get_framerates(max, desc.fourcc);

            rf.resolution.framerate_count = fps.size();
            rf.framerates = fps;

            std::vector<struct framerate_mapping> res_vec;
            res_vec.push_back(rf);

            SPDLOG_DEBUG("Adding format desc: {} ({:x}) ", desc.description, desc.fourcc);

            tcam_image_size sensor_size = get_sensor_size();

            for (const auto& s : m_scale.scales)
            {
                if (rf.resolution.type == TCAM_RESOLUTION_TYPE_FIXED)
                {
                    if (s.legal_resolution(sensor_size, rf.resolution.max_size))
                    {
                        auto new_rf = rf;

                        new_rf.resolution.scaling = s;

                        res_vec.push_back(new_rf);
                    }
                }
                else
                {
                    auto new_rf = rf;

                    // TODO: use TestBinning etc to have values calculated via genicam
                    new_rf.resolution.max_size = s.allowed_max(sensor_size);

                    // ensure max is divisible by step
                    new_rf.resolution.max_size.width -= new_rf.resolution.max_size.width % new_rf.resolution.width_step_size;
                    new_rf.resolution.max_size.height -= new_rf.resolution.max_size.height % new_rf.resolution.height_step_size;

                    // SPDLOG_ERROR("{}x{} max:{}x{} =>",
                    //              new_rf.resolution.min_size.width, new_rf.resolution.min_size.height,
                    //              new_rf.resolution.max_size.width, new_rf.resolution.max_size.height);

                    new_rf.resolution.scaling = s;

                    res_vec.push_back(new_rf);

                }
            }

            this->available_videoformats.push_back(
                VideoFormatDescription(format_handler, desc, res_vec));
        }
    }
    else
    {
        SPDLOG_ERROR("NO PixelFormat Node");
    }

    // reset format to settings before indexing
    set_video_format(reset_format);
}


tcam_image_size AravisDevice::get_sensor_size() const
{
    auto pw = tcam::property::find_property(m_internal_properties, "SensorWidth");
    auto ph = tcam::property::find_property(m_internal_properties, "SensorHeight");

    if (!pw || !ph)
    {
        SPDLOG_ERROR("Unable to find property SensorWidth/SensorHeight");
        return {};
    }

    auto width = dynamic_cast<tcam::property::IPropertyInteger*>(pw.get());
    auto height = dynamic_cast<tcam::property::IPropertyInteger*>(ph.get());

    auto res_w = width->get_value();

    if (!res_w)
    {
        SPDLOG_ERROR("Unable to retrieve SensorWidth value");
        return {};
    }

    auto res_h = height->get_value();

    if (!res_h)
    {
        SPDLOG_ERROR("Unable to retrieve SensorHeight value");
        return {};
    }

    return {(uint32_t)res_w.value(), (uint32_t)res_h.value()};
}
