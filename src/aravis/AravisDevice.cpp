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

#include "../logging.h"
#include "../utils.h"
#include "AravisPropertyBackend.h"
#include "AravisAllocator.h"
#include "aravis_utils.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <dutils_img/fcc_to_string.h> // img::fcc_to_string
#include <stdexcept>

using namespace tcam;

AravisDevice::AravisDevice(const DeviceInfo& device_desc)
{
    device = device_desc;
    GError* err = NULL;

    this->arv_camera_ = arv_camera_new(this->device.get_info().identifier, &err);
    if (err)
    {
        SPDLOG_ERROR("Error while creating arv_camera: {}", err->message);
        g_clear_error(&err);
    }
    if (this->arv_camera_ == NULL)
    {
        throw std::runtime_error("Error while creating ArvCamera");
    }

    if (arv_camera_is_gv_device(this->arv_camera_))
    {
        if (!arv_gv_device_is_controller((ArvGvDevice*)arv_camera_get_device(this->arv_camera_)))
        {
            SPDLOG_ERROR("This process does not control the device!");
            throw std::runtime_error("Device already open.");
        }
        auto_set_packet_size();
        auto_set_control_lifetime();
    }

    backend_ = std::make_shared<tcam::aravis::AravisPropertyBackend>(*this);

    genicam_ = arv_device_get_genicam(arv_camera_get_device(this->arv_camera_));

    index_genicam();

    // make aravis notify us when the device can not be reached
    g_signal_connect(
        arv_camera_get_device(arv_camera_), "control-lost", G_CALLBACK(device_lost), this);

    allocator_ = std::make_shared<tcam::aravis::AravisAllocator>();
}


AravisDevice::~AravisDevice()
{
    if (arv_camera_ != NULL)
    {
        g_object_unref(arv_camera_);
        arv_camera_ = NULL;
    }
}


auto AravisDevice::fetch_test_itf_framerates(const VideoFormat& fmt)
    -> outcome::result<tcam::framerate_info>
{
    auto dev = arv_camera_get_device(arv_camera_);
    if (dev == nullptr)
    {
        return tcam::status::DeviceCouldNotBeOpened;
    }

    auto set_int = [dev](const char* name, int value) -> tcam::status
    {
        GError* error = nullptr;
        arv_device_set_integer_feature_value(dev, name, value, &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to set '{}'. Error: {}", name, error->message);
            return tcam::aravis::consume_GError(error);
        }
        return tcam::status::Success;
    };

    if (auto res = set_int("TestPixelFormat", fourcc2aravis(fmt.get_fourcc()));
        res != tcam::status::Success)
    {
        return res;
    }
    if (auto res = set_int("TestWidth", fmt.get_size().width); res != status::Success)
    {
        return res;
    }
    if (auto res = set_int("TestHeight", fmt.get_size().height); res != status::Success)
    {
        return res;
    }
    if (has_test_binning_h_)
    {
        if (auto res = set_int("TestBinningHorizontal", fmt.get_scaling().binning_h);
            res != status::Success)
            return res;
    }
    if (has_test_binning_v_)
    {
        if (auto res = set_int("TestBinningVertical", fmt.get_scaling().binning_v);
            res != status::Success)
            return res;
    }
    if (has_test_skipping_h_)
    {
        if (auto res = set_int("TestDecimationHorizontal", fmt.get_scaling().skipping_h);
            res != status::Success)
            return res;
    }
    if (has_test_skipping_v_)
    {
        if (auto res = set_int("TestDecimationVertical", fmt.get_scaling().skipping_v);
            res != status::Success)
            return res;
    }

    GError* error = nullptr;
    auto min = arv_device_get_float_feature_value(dev, "ResultingMinFPS", &error);
    if (error)
    {
        SPDLOG_ERROR("Failed to get 'ResultingMinFPS'. Error: {}", error->message);
        return aravis::consume_GError(error);
    }
    auto max = arv_device_get_float_feature_value(dev, "ResultingMaxFPS", &error);
    if (error)
    {
        SPDLOG_ERROR("Failed to get 'ResultingMaxFPS'. Error: {}", error->message);
        return aravis::consume_GError(error);
    }
    return tcam::framerate_info { min, max };
}

static auto fetch_FPS_enum_framerates(ArvDevice* dev)
    -> outcome::result<tcam::framerate_info>
{
    // 2022/02/02 Christopher: I think this is only used for very old cameras, so we can skip this
    // Another Note, this is not complete, I think there were excludes defined somewhere
    GError* error = nullptr;
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
        return tcam::status::DeviceCouldNotBeOpened;
    }

    std::vector<double> ret;
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
    return tcam::framerate_info{ ret };
}

// workaround for aravis behavior
// aravis expects FPS
// not all cameras have that node
// get_frame_rate(_bounds) uses FPS
static double get_framerate(ArvCamera* camera)
{
    ArvGcNode* node = arv_device_get_feature(arv_camera_get_device(camera), "AcquisitionFrameRate");

    double value = -1.0;
    GError* err = nullptr;

    if (node && ARV_IS_GC_FLOAT_NODE(node))
    {
        value = arv_gc_float_get_value(ARV_GC_FLOAT(node), &err);
    }
    else if (node && ARV_IS_GC_ENUMERATION(node))
    {
        // if it is an enum, it means that FPS
        // is also available
        // that has better 'values'
        auto val = arv_camera_get_integer(camera, "FPS", &err);
        value = ((10000000 / (double)val) * 100 + 0.5) / 100.0;
    }
    else
    {
        value = arv_camera_get_frame_rate(camera, &err);
    }

    if (err)
    {
        SPDLOG_ERROR("Unable to query framerate: {}", err->message);
        g_clear_error(&err);
    }

    return value;
}


static std::pair<double, double> get_framerate_bounds(ArvCamera* camera)
{
    ArvGcNode* node = arv_device_get_feature(arv_camera_get_device(camera), "AcquisitionFrameRate");

    double min = -1;
    double max = -1;
    GError* err = nullptr;

    if (node && ARV_IS_GC_FLOAT_NODE(node))
    {
        min = arv_gc_float_get_min(ARV_GC_FLOAT(node), &err);
        max = arv_gc_float_get_max(ARV_GC_FLOAT(node), &err);
    }
    else
    {
        arv_camera_get_frame_rate_bounds(camera, &min, &max, &err);
    }

    if (err)
    {
        SPDLOG_ERROR("Unable to query framerate bounds: {}", err->message);
        g_clear_error(&err);
    }

    return {min, max};
}


void set_frame_rate(ArvCamera* camera, double fps)
{
    ArvGcNode* node = arv_device_get_feature(arv_camera_get_device(camera), "AcquisitionFrameRate");

    GError* err = nullptr;
    if (node && ARV_IS_GC_FLOAT_NODE(node))
    {
        arv_gc_float_set_value(ARV_GC_FLOAT(node), fps, &err);
    }
    else if (node && ARV_IS_GC_ENUMERATION(node))
    {
        guint n_fps_values = 0;
        auto fps_values = arv_device_dup_available_enumeration_feature_values(
            arv_camera_get_device(camera), "FPS", &n_fps_values, &err);

        if (n_fps_values == 0)
        {
            // alternative failed
            // return empty vector and let format handle it
            SPDLOG_ERROR("Unable to determine what framerate settings are used. {}",
                         err->message);
            return;
        }

        for (unsigned int i = 0; i < n_fps_values; ++i)
        {
            auto val = fps_values[i];

            auto v = (int)((10000000 / (double)val) * 100 + 0.5) / 100.0;
            if (v == fps)
            {
                arv_camera_set_integer(camera, "FPS", val, &err);
                break;
            }
        }

        if (fps_values)
        {
            g_free(fps_values);
        }
        return;
    }
    else
    {
            arv_camera_set_frame_rate(camera, fps, &err);
    }

    if (err)
    {
        SPDLOG_ERROR("Failed to set framerate. error: {}", err->message);
        g_clear_error(&err);
    }
}


namespace
{
struct data_to_restore_active_format
{
    ArvPixelFormat format = 0;
    bool is_region = false;
    int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

static auto restore_data_fetch(ArvCamera* camera, bool has_offsets)
    -> std::optional<data_to_restore_active_format>
{
    GError* error = nullptr;

    auto active_format = arv_camera_get_pixel_format(camera, &error);
    if (error)
    {
        SPDLOG_ERROR("Failed to fetch pixel format. arv_camera_get_pixel_format error: {}",
                     error->message);
        return {};
    }

    if (has_offsets)
    {
        int x1, x2, y1, y2;
        arv_camera_get_region(camera, &x1, &y1, &x2, &y2, &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch restore region. arv_camera_get_region error: {}",
                         error->message);
            g_clear_error(&error);
            return {};
        }
        return data_to_restore_active_format { active_format, true, x1, x2, y1, y2 };
    }
    else
    {
        int width = arv_camera_get_integer(camera, "Width", &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch 'Width'. arv_camera_get_integer error: {}",
                         error->message);
            g_clear_error(&error);
            return {};
        }
        int height = arv_camera_get_integer(camera, "Height", &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch 'Height'. arv_camera_get_integer error: {}",
                         error->message);
            g_clear_error(&error);
            return {};
        }
        return data_to_restore_active_format { active_format, false, width, height };
    }
}

static void restore_data_reapply(ArvCamera* camera, data_to_restore_active_format data)
{
    GError* error = nullptr;
    if (data.is_region)
    {
        arv_camera_set_region(camera, data.x1, data.x2, data.y1, data.y1, &error);

        if (error)
        {
            SPDLOG_WARN("Failed to restore active format region due to: {}", error->message);
            g_clear_error(&error);
        }
    }
    else
    {
        arv_camera_set_integer(camera, "Width", data.x1, &error);
        if (error)
        {
            SPDLOG_WARN("Failed to restore 'Width' due to: {}", error->message);
            g_clear_error(&error);
        }
        arv_camera_set_integer(camera, "Height", data.x2, &error);
        if (error)
        {
            SPDLOG_WARN("Failed to restore 'Height' due to: {}", error->message);
            g_clear_error(&error);
        }
    }
}
} // namespace


outcome::result<tcam::framerate_info> tcam::AravisDevice::get_framerate_info(const VideoFormat& fmt)
{
    std::scoped_lock lck0 { arv_camera_access_mutex_ };

    if (has_test_format_interface_)
    {
        return fetch_test_itf_framerates(fmt);
    }

    auto dev = arv_camera_get_device(arv_camera_);
    if (has_FPS_enum_interface_)
    {
        return fetch_FPS_enum_framerates(dev);
    }

    if (stream_)
    {
        // this means a stream is active do not touch settings
        SPDLOG_ERROR("Failed to fetch FPS list because the camera is currently open and it does "
                     "not have 'TestPixelFormat'.");
        return tcam::status::InvalidParameter;
    }

    auto restore_data = restore_data_fetch(arv_camera_, has_offset_);
    if (!restore_data)
    {
        return tcam::status::InvalidParameter;
    }

    auto s = fmt.get_size();

    {
        GError* err = nullptr;
        arv_camera_set_pixel_format(arv_camera_, fourcc2aravis(fmt.get_fourcc()), &err);
        if (err)
        {
            SPDLOG_ERROR("Failed to set pixelformat. arv_camera_set_pixel_format error: {}",
                         err->message);
            g_clear_error(&err);
        }
    }

    if (has_offset_)
    {
        GError* error = nullptr;
        arv_camera_set_region(arv_camera_, 0, 0, s.width, s.height, &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch framerate list. arv_camera_set_region error: {}",
                         error->message);
            g_clear_error(&error);
        }
    }
    else
    {
        GError* error = nullptr;
        arv_camera_set_integer(arv_camera_, "Width", s.width, &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch framerate list. arv_camera_set_integer error: {}",
                         error->message);
            g_clear_error(&error);
        }
        arv_camera_set_integer(arv_camera_, "Height", s.height, &error);
        if (error)
        {
            SPDLOG_ERROR("Failed to fetch framerate list. arv_camera_set_integer error: {}",
                         error->message);
            g_clear_error(&error);
        }
    }

    auto fps_bounds = get_framerate_bounds(arv_camera_);

    double min = fps_bounds.first;
    double max = fps_bounds.second;

    // restore previous settings
    restore_data_reapply(arv_camera_, restore_data.value());

    SPDLOG_TRACE("Queried: {}x{} fourcc {} Received min: {} max {}",
                 s.width,
                 s.height,
                 fmt.get_fourcc_string(),
                 min,
                 max);

    return tcam::framerate_info { min, max };
}


void AravisDevice::disable_chunk_mode()
{
    GError* err = nullptr;
    arv_camera_set_chunk_mode(arv_camera_, false, &err);
    if (err)
    {
        SPDLOG_DEBUG("Failed to set 'ChunkModeActive' to false. Ignoring for now. Err: {}",
                    err->message);
        g_clear_error(&err);
    }

    auto prop_GevGVSPExtendedIDMode =
        find_cam_property<tcam::property::IPropertyEnum>("GevGVSPExtendedIDMode");
    if (prop_GevGVSPExtendedIDMode)
    {
        auto res = prop_GevGVSPExtendedIDMode->set_value("Off");
        if (res.has_failure())
        {
            SPDLOG_WARN("Failed to set 'GevGVSPExtendedIDMode' to Off. Ignoring for now. Err: {}",
                        res.error().message());
        }
    }
}

bool AravisDevice::set_video_format(const VideoFormat& new_format)
{
    std::scoped_lock lck { arv_camera_access_mutex_ };

    if (is_lost_)
    {
        return false;
    }

//    SPDLOG_DEBUG("Setting format to '{}'", new_format.to_string());

    disable_chunk_mode();

    bool ret = false;
    GError* err = nullptr;

    // arv_camera_set_frame_rate overwrites TriggerSelector and TriggerMode
    // set them again after changing the framerate to ensure consistent behavior
    const char* trig_selector = arv_device_get_string_feature_value(
        arv_camera_get_device(arv_camera_), "TriggerSelector", &err);
    if (err)
    {
        SPDLOG_WARN("Failed to fetch TriggerSelector. error: {}", err->message);
        g_clear_error(&err);
    }

    const char* trig_mode = arv_device_get_string_feature_value(
        arv_camera_get_device(arv_camera_), "TriggerMode", &err);
    if (err)
    {
        SPDLOG_WARN("Failed to fetch TriggerMode. error: {}", err->message);
        g_clear_error(&err);
    }

    arv_camera_set_pixel_format(this->arv_camera_, fourcc2aravis(new_format.get_fourcc()), &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set pixel format: {}", err->message);
        g_clear_error(&err);
        goto set_video_format_finish;
    }

    if (has_offset_)
    {
        // preserve current offset
        int offset_x;
        int offset_y;
        arv_camera_get_region(arv_camera_, &offset_x, &offset_y, nullptr, nullptr, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to verify offsets: {}", err->message);
            g_clear_error(&err);
            goto set_video_format_finish;
        }

        arv_camera_set_region(this->arv_camera_,
                              offset_x,
                              offset_y,
                              new_format.get_size().width,
                              new_format.get_size().height,
                              &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to set region: {}", err->message);
            g_clear_error(&err);
            goto set_video_format_finish;
        }
    }
    else
    {
        arv_camera_set_integer(arv_camera_, "Width", new_format.get_size().width, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to set Width: {}", err->message);
            g_clear_error(&err);
            goto set_video_format_finish;
        }
        arv_camera_set_integer(arv_camera_, "Height", new_format.get_size().height, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to set Height: {}", err->message);
            g_clear_error(&err);
            goto set_video_format_finish;
        }
    }
    if (!set_scaling(new_format.get_scaling()))
    {
        goto set_video_format_finish;
    }

    set_frame_rate(arv_camera_, new_format.get_framerate());

    active_video_format_ = read_camera_current_video_format();
    //SPDLOG_DEBUG("Active format is now '{}'", active_video_format_.to_string());
    ret = true;

set_video_format_finish:

    // reset properties
    // NO FORMAT CHANGES AFTER THIS POINT
    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera_), "TriggerSelector", trig_selector, &err);
    if (err)
    {
        SPDLOG_ERROR("Failed to reset 'TriggerSelector' error: {}", err->message);
        g_clear_error(&err);
    }
    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera_), "TriggerMode", trig_mode, &err);

    if (err)
    {
        SPDLOG_ERROR("Failed to reset 'TriggerMode' error: {}", err->message);
        g_clear_error(&err);
    }

    return ret;
}

tcam::VideoFormat AravisDevice::read_camera_current_video_format()
{
    VideoFormat format;

    format.set_framerate(get_framerate(this->arv_camera_));

    GError* err = nullptr;

    format.set_fourcc(aravis2fourcc(arv_camera_get_pixel_format(this->arv_camera_, &err)));
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve pixel format: {}", err->message);
        g_clear_error(&err);
    }

    unsigned int height = 0;
    unsigned int width = 0;

    if (has_offset_)
    {
        int x1, x2, y1, y2;
        arv_camera_get_region(arv_camera_, &x1, &y1, &x2, &y2, &err);

        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve region: {}", err->message);
            g_clear_error(&err);
            return format;
        }

        height = y2;
        width = x2;
    }
    else
    {
        width = arv_camera_get_integer(arv_camera_, "Width", &err);
        if (err)
        {
            SPDLOG_ERROR("Error while retrieving format width: {}", width);
            g_clear_error(&err);
        }

        height = arv_camera_get_integer(arv_camera_, "Height", &err);
        if (err)
        {
            SPDLOG_ERROR("Error while retrieving format height: {}", height);
            g_clear_error(&err);
        }
    }

    format.set_size(width, height);

    format.set_scaling(get_current_scaling());

    return format;
}

void AravisDevice::auto_set_control_lifetime()
{
    const int default_heartbeat_ms = 3000;
    int heartbeat_ms =
        tcam::get_environment_variable_int("TCAM_GIGE_HEARTBEAT_MS").value_or(default_heartbeat_ms);

    arv_camera_set_integer(arv_camera_, "GevHeartbeatTimeout", heartbeat_ms, NULL);
    SPDLOG_DEBUG("Setting heartbeat timeout to {} ms.", heartbeat_ms);
}

void AravisDevice::auto_set_packet_size()
{
    auto env_packet_size = tcam::get_environment_variable_int("TCAM_GIGE_PACKET_SIZE");

    if (!env_packet_size)
    {
        GError* err = nullptr;
        guint packet_size = arv_camera_gv_auto_packet_size(this->arv_camera_, &err);
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
        GError* err = nullptr;
        arv_camera_gv_set_packet_size(arv_camera_, env_packet_size.value(), &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to set packet size: {}", err->message);
            g_clear_error(&err);
            return;
        }
        SPDLOG_INFO("Set packet size according to environment to: {}", env_packet_size.value());
    }
}

void AravisDevice::device_lost(ArvGvDevice* device __attribute__((unused)), void* user_data)
{
    AravisDevice* self = (AravisDevice*)user_data;

    self->is_lost_ = true;

    self->notify_device_lost();
}

//// genicam handling
void AravisDevice::index_genicam()
{
    assert(genicam_ != nullptr);

    generate_properties_from_genicam();

    has_test_format_interface_ = has_genicam_property("TestPixelFormat");

    has_test_binning_h_ = has_genicam_property("TestBinningHorizontal");
    has_test_binning_v_ = has_genicam_property("TestBinningVertical");
    has_test_skipping_h_ = has_genicam_property("TestDecimationHorizontal");
    has_test_skipping_v_ = has_genicam_property("TestDecimationVertical");

    has_FPS_enum_interface_ = false;
    if (auto fps_node = get_genicam_property_node("FPS"); fps_node != nullptr)
    {
        auto node_type = arv_dom_node_get_node_name(ARV_DOM_NODE(fps_node));
        if (node_type != nullptr && node_type == std::string_view("Enumeration"))
        {
            has_FPS_enum_interface_ = true;
        }
    }
    // this seems to be a check if arv_camera_get_x_offset_bounds works
    has_offset_ = [this]
    {
        GError* eeee = nullptr;

        int ww = 0;
        int hh = 0;
        arv_camera_get_x_offset_bounds(arv_camera_, &ww, &hh, &eeee);
        if (eeee)
        {
            g_error_free(eeee);
            return false;
        }
        return true;
    }();

    generate_scaling_information();

    active_video_format_ = read_camera_current_video_format();

    generate_video_formats();

    set_video_format(this->active_video_format_); // reset after generate_video_formats
}

namespace
{
struct pixel_format_entry
{
    ArvPixelFormat pixel_format;
    std::string pixel_desc;
};

static auto fetch_pixel_format_list(ArvCamera* camera) -> std::vector<pixel_format_entry>
{
    GError* err = nullptr;
    unsigned int n_formats = 0;
    gint64* pixel_format_ints = arv_camera_dup_available_pixel_formats(camera, &n_formats, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve available pixel formats: {}", err->message);
        g_clear_error(&err);
        return {};
    }

    unsigned n2_formats = 0;
    const char** format_str =
        arv_camera_dup_available_pixel_formats_as_strings(camera, &n2_formats, &err);
    if (err)
    {
        g_free(pixel_format_ints);

        SPDLOG_ERROR("Unable to retrieve pixel format description strings: {}", err->message);
        g_clear_error(&err);
        return {};
    }

    if (n_formats != n2_formats)
    {
        g_free(pixel_format_ints);
        g_free(format_str);

        SPDLOG_ERROR(
            "Format retrieval encountered nonsensical information n_formats={}, n2_formats={}",
            n_formats,
            n2_formats);
        return {};
    }

    std::vector<pixel_format_entry> rval;
    rval.reserve(n_formats);
    for (unsigned int idx = 0; idx < n_formats; ++idx)
    {
        rval.push_back(pixel_format_entry { static_cast<ArvPixelFormat>(pixel_format_ints[idx]),
                                            format_str[idx] });
    }
    g_free(pixel_format_ints);
    g_free(format_str);

    return rval;
}
} // namespace

void AravisDevice::generate_video_formats()
{
    GError* err = nullptr;

    tcam_image_size sensor_size = get_sensor_size();

    if (has_offset_)
    {
        // reset region to entirety of the sensor
        // depending on the camera model (z12)
        // the width/height boundaries may be skewed
        // due to internal auto offset settings
        arv_camera_set_region(arv_camera_, 0, 0, sensor_size.width, sensor_size.height, &err);

        if (err)
        {
            SPDLOG_ERROR("set_region caused error: {}", err->message);
            g_clear_error(&err);
        }
    }
    else
    {
        arv_camera_set_integer(arv_camera_, "Width", sensor_size.width, &err);
        if (err)
        {
            SPDLOG_ERROR("set_region caused error: {}", err->message);
            g_clear_error(&err);
        }
        arv_camera_set_integer(arv_camera_, "Height", sensor_size.height, &err);
        if (err)
        {
            SPDLOG_ERROR("set_region caused error: {}", err->message);
            g_clear_error(&err);
        }
    }

#if 0 // Needed ??
    if (auto pixel_node = has_genicam_property("PixelFormat"); !pixel_node)
    {
        SPDLOG_ERROR("Found no PixelFormat Node in GenICam document");
        return;
    }
#endif

    for (auto&& entry : fetch_pixel_format_list(arv_camera_))
    {
        const gint64 pixel_format_id = entry.pixel_format;
        const auto pixel_format_desc = entry.pixel_desc;

        const uint32_t fcc = aravis2fourcc(pixel_format_id);
        if (fcc == 0)
        {
            SPDLOG_INFO(
                "Input format '{}' ({:#x}) not supported.", pixel_format_desc, pixel_format_id);
            continue;
        }

        tcam_video_format_description desc = {};
        desc.fourcc = fcc;

        strncpy(desc.description, pixel_format_desc.c_str(), sizeof(desc.description) - 1);

        arv_camera_set_pixel_format(this->arv_camera_, pixel_format_id, &err);
        if (err)
        {
            SPDLOG_ERROR("Failed to set PixelFormat. Error: {}.", err->message);
            g_clear_error(&err);
            continue;
        }

        int width_min = 0;
        int width_max = 0;

        int height_min = 0;
        int height_max = 0;

        arv_camera_get_width_bounds(this->arv_camera_, &width_min, &width_max, &err);
        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve width bounds: {}", err->message);
            g_clear_error(&err);
            continue;
        }

        arv_camera_get_height_bounds(this->arv_camera_, &height_min, &height_max, &err);

        if (err)
        {
            SPDLOG_ERROR("Unable to retrieve height bounds: {}", err->message);
            g_clear_error(&err);
            continue;
        }

        auto width_step = arv_camera_get_width_increment(this->arv_camera_, &err);
        if (err)
        {
            SPDLOG_WARN("Unable to retrieve width increment: {}", err->message);
            g_clear_error(&err);
            width_step = 1;
        }

        auto height_step = arv_camera_get_height_increment(this->arv_camera_, &err);
        if (err)
        {
            SPDLOG_WARN("Unable to retrieve height increment: {}", err->message);
            g_clear_error(&err);
            height_step = 1;
        }

        tcam_image_size min_resolution = { (unsigned int)width_min, (unsigned int)height_min };
        tcam_image_size max_resolution = { (unsigned int)width_max, (unsigned int)height_max };

        framerate_mapping rf = {};
        rf.resolution.max_size = max_resolution;
        rf.resolution.min_size = min_resolution;

        if (min_resolution == max_resolution)
        {
            rf.resolution.type = TCAM_RESOLUTION_TYPE_FIXED;
        }
        else
        {
            rf.resolution.type = TCAM_RESOLUTION_TYPE_RANGE;
            rf.resolution.width_step_size = width_step;
            rf.resolution.height_step_size = height_step;
        }

        if (auto framerate_res = get_framerate_info(VideoFormat { fcc, max_resolution, {}, 0 });
            framerate_res.has_error())
        {
            continue;
        }
        else
        {
            rf.framerates = framerate_res.value().to_list();
        }

        std::vector<framerate_mapping> res_vec;

        // Do not add rf here
        // we will already add this information
        // later with the appropriate scaling
        //res_vec.push_back(rf);

        SPDLOG_DEBUG("Adding format desc: {} ({:x}) ", desc.description, desc.fourcc);

        if(scale_.scaling_info_list.empty())
        {
            res_vec.push_back(rf);
        }
        else
        {
            for (const auto& scaling_info : scale_.scaling_info_list)
            {
                if (rf.resolution.type == TCAM_RESOLUTION_TYPE_FIXED)
                {
                    if (scaling_info.legal_resolution(sensor_size, rf.resolution.max_size))
                    {
                        auto new_rf = rf;

                        new_rf.resolution.scaling = scaling_info;

                        res_vec.push_back(new_rf);
                    }
                }
                else
                {
                    // TODO: use TestBinning etc to have values calculated via genicam
                    auto binned_max_size = scaling_info.allowed_max(sensor_size);

                    // ensure max is divisible by step
                    binned_max_size.width -= binned_max_size.width % width_step;
                    binned_max_size.height -= binned_max_size.height % height_step;

                    tcam_resolution_description res = {
                        TCAM_RESOLUTION_TYPE_RANGE,
                        min_resolution, binned_max_size,
                        (unsigned)width_step, (unsigned)height_step,
                        scaling_info
                    };
                    // SPDLOG_ERROR("{}x{} max:{}x{} =>",
                    //              new_rf.resolution.min_size.width, new_rf.resolution.min_size.height,
                    //              new_rf.resolution.max_size.width, new_rf.resolution.max_size.height);

                    VideoFormat fmt { fcc, binned_max_size, scaling_info, 0 };
                    if (auto framerate_res = get_framerate_info(fmt); !framerate_res.has_error())
                    {
                        res_vec.push_back({ res, framerate_res.value().to_list() });
                    }
                }
            }
        }

        available_videoformats_.push_back(VideoFormatDescription(nullptr, desc, res_vec));
    }
}


tcam_image_size AravisDevice::get_sensor_size() const
{
    auto width = find_cam_property<tcam::property::IPropertyInteger>("SensorWidth");
    auto height = find_cam_property<tcam::property::IPropertyInteger>("SensorHeight");

    if (!width || !height)
    {
        SPDLOG_ERROR("Unable to find property SensorWidth/SensorHeight");
        return {};
    }

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

    return { (uint32_t)res_w.value(), (uint32_t)res_h.value() };
}

bool AravisDevice::has_genicam_property(const char* name) const
{
    return arv_gc_get_node(genicam_, name) != nullptr;
}

ArvGcNode* AravisDevice::get_genicam_property_node(const char* name) const
{
    return arv_gc_get_node(genicam_, name);
}
