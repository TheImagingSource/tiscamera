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

#include "aravis_utils.h"
#include "internal.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>


using namespace tcam;


AravisDevice::AravisPropertyHandler::AravisPropertyHandler(AravisDevice* dev) : device(dev) {}


bool AravisDevice::AravisPropertyHandler::get_property(Property& p)
{
    for (auto& pr : properties)
    {
        if (pr.prop->get_ID() == p.get_ID())
        {
            if (!pr.prop->is_external())
            {
                device->update_property(pr);
            }

            p.set_struct(pr.prop->get_struct());
            return true;
        }
    }
    return false;
}


bool AravisDevice::AravisPropertyHandler::set_property(const Property& p)
{
    return device->set_property(p);
}


AravisDevice::AravisFormatHandler::AravisFormatHandler(AravisDevice* dev) : device(dev) {}


std::vector<double> AravisDevice::AravisFormatHandler::get_framerates(
    const struct tcam_image_size& s,
    int pixelformat)
{
    std::vector<double> ret;
    auto dev = arv_camera_get_device(device->arv_camera);

    // TODO implement better way to check for availability
    GError** error = NULL;

    if (pixelformat != 0)
    {
        arv_device_set_integer_feature_value(
            dev, "TestPixelFormat", fourcc2aravis(pixelformat), error);
    }
    arv_device_set_integer_feature_value(dev, "TestWidth", s.width, error);
    arv_device_set_integer_feature_value(dev, "TestHeight", s.height, error);

    double min = arv_device_get_float_feature_value(dev, "ResultingMinFPS", error);
    double max = arv_device_get_float_feature_value(dev, "ResultingMaxFPS", error);

    if (min == 0.0 && max == 0.0)
    {
        // this means TestPixelFormat, TestWidth, TestHeight are not available
        // could be an UsbVision camera
        int x1, x2, y1, y2;
        arv_camera_get_region(this->device->arv_camera, &x1, &y1, &x2, &y2, error);

        unsigned int height = y2 - y1;
        unsigned int width = x2 - x1;

        arv_camera_set_region(this->device->arv_camera, 0, 0, s.width, s.height, error);

        min = arv_device_get_float_feature_value(dev, "MinFPS", error);
        max = arv_device_get_float_feature_value(dev, "MaxFPS", error);

        arv_camera_set_region(this->device->arv_camera, 0, 0, width, height, error);
    }

    if (min == 0.0 && max == 0.0)
    {
        // this means either the camera is broken or we have a FPS enum
        // hope for the second and try it
        guint n_fps_values = 0;
        auto fps_values =
            arv_device_dup_available_enumeration_feature_values(dev, "FPS", &n_fps_values, error);

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
                 pixelformat,
                 min,
                 max);

    ret = create_steps_for_range(min, max);

    return ret;
}


AravisDevice::AravisDevice(const DeviceInfo& device_desc) : handler(nullptr), stream(NULL)
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

    arv_options.auto_socket_buffer = false;
    arv_options.packet_timeout = 40;
    arv_options.frame_retention = 200;

    if (arv_camera_is_gv_device(this->arv_camera))
    {
        auto_set_packet_size();
        determine_packet_request_ratio();
    }

    handler = std::make_shared<AravisPropertyHandler>(this);
    format_handler = std::make_shared<AravisFormatHandler>(this);

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


std::vector<std::shared_ptr<Property>> AravisDevice::getProperties()
{
    std::vector<std::shared_ptr<Property>> vec;

    for (auto& p : handler->properties) { vec.push_back(p.prop); }
    SPDLOG_DEBUG("Returning {} properties", vec.size());

    return vec;
}


bool AravisDevice::set_property(const Property& p)
{
    if (is_lost)
    {
        return false;
    }

    auto f = [p](const property_mapping& m) {
        return p.get_name().compare(m.prop->get_name()) == 0;
    };

    auto pm = std::find_if(handler->properties.begin(), handler->properties.end(), f);

    if (pm == handler->properties.end())
    {
        return false;
    }

    auto _device = arv_camera_get_device(arv_camera);

    Property::VALUE_TYPE value_type = pm->prop->get_value_type();

    GError* err = nullptr;

    switch (value_type)
    {
        case Property::INTEGER:
        {
            SPDLOG_DEBUG(
                "Integer {}: {}", pm->arv_ident.c_str(), ((PropertyInteger&)p).get_value());

            arv_device_set_integer_feature_value(
                _device, pm->arv_ident.c_str(), ((PropertyInteger&)p).get_value(), &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to set integer: {}", err->message);
                g_clear_error(&err);
                break;
            }
            pm->prop->set_struct(p.get_struct());
            break;
        }
        case Property::INTSWISSKNIFE:
        {
            SPDLOG_DEBUG("Swissknife");
            arv_device_set_integer_feature_value(
                _device, pm->arv_ident.c_str(), ((PropertyInteger&)(p)).get_value(), &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to set Swissknife: {}", err->message);
                g_clear_error(&err);
                break;
            }

            break;
        }
        case Property::FLOAT:
        {
            double d = 0.0;

            if (p.get_type() == TCAM_PROPERTY_TYPE_INTEGER)
            {
                d = ((PropertyInteger&)(p)).get_value();
            }
            else
            {
                d = ((PropertyDouble&)(p)).get_value();
            }

            SPDLOG_DEBUG("Sending property change for (float) {} {}", p.get_name().c_str(), d);
            arv_device_set_float_feature_value(_device, pm->arv_ident.c_str(), d, &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to set float: {}", err->message);
                g_clear_error(&err);
                break;
            }
            pm->prop->set_struct(p.get_struct());

            break;
        }
        case Property::BUTTON:
        case Property::COMMAND:
        {
            //arv_device_
            arv_device_execute_command(_device, pm->arv_ident.c_str(), &err);

            if (err)
            {
                SPDLOG_ERROR("Unable to set button/command: {}", err->message);
                g_clear_error(&err);
                break;
            }

            break;
        }
        case Property::BOOLEAN:
        {
            SPDLOG_DEBUG("Bool {}", pm->arv_ident.c_str());
            if (((PropertyBoolean&)p).get_value())
            {
                arv_device_set_integer_feature_value(_device, pm->arv_ident.c_str(), 1, &err);
            }
            else
            {
                arv_device_set_integer_feature_value(_device, pm->arv_ident.c_str(), 0, &err);
            }

            if (err)
            {
                SPDLOG_ERROR("Unable to set bool: {}", err->message);
                g_clear_error(&err);
                break;
            }

            pm->prop->set_struct(p.get_struct());
            break;
        }
        case Property::STRING:
        case Property::ENUM:
        {
            if (p.get_type() == TCAM_PROPERTY_TYPE_BOOLEAN)
            {
                if (((PropertyBoolean&)p).get_value())
                {
                    arv_device_set_integer_feature_value(_device, pm->arv_ident.c_str(), 1, &err);
                }
                else
                {
                    arv_device_set_integer_feature_value(_device, pm->arv_ident.c_str(), 0, &err);
                }

                if (err)
                {
                    SPDLOG_ERROR("Unable to set enum: {}", err->message);
                    g_clear_error(&err);
                    break;
                }

                pm->prop->set_struct(p.get_struct());
            }
            else if (p.get_type() == TCAM_PROPERTY_TYPE_INTEGER)
            {
                guint value_count = 0;
                gint64* values = arv_device_dup_available_enumeration_feature_values(
                    _device, pm->arv_ident.c_str(), &value_count, &err);

                int64_t p_val = ((PropertyInteger&)p).get_value();

                for (unsigned int i = 0; i < value_count; ++i)
                {
                    if (p_val == values[i])
                    {
                        arv_device_set_integer_feature_value(
                            _device, pm->arv_ident.c_str(), p_val, &err);

                        if (err)
                        {
                            SPDLOG_ERROR("Unable to set enum: {}", err->message);
                            g_clear_error(&err);
                        }
                        break;
                    }
                }
            }
            else if (p.get_type() == TCAM_PROPERTY_TYPE_ENUMERATION)
            {
                std::string val = ((PropertyEnumeration&)p).get_value();

                SPDLOG_DEBUG("Setting '{}' to '{}'", pm->arv_ident.c_str(), val.c_str());
                arv_device_set_string_feature_value(
                    _device, pm->arv_ident.c_str(), val.c_str(), &err);

                if (err)
                {
                    SPDLOG_ERROR("Unable to set enum: {}", err->message);
                    g_clear_error(&err);
                    break;
                }
            }

            break;
        }
        case Property::UNDEFINED:
        default:
        {
            SPDLOG_ERROR("{} NOT SUPPORTED!!!", p.get_name().c_str());
            break;
        }
    }
    return true;
}


bool AravisDevice::get_property(Property& p)
{
    return handler->get_property(p);
}


void AravisDevice::update_property(struct property_mapping& mapping)
{
    if (is_lost)
    {
        return;
    }
    auto& p = mapping.prop;

    auto _device = arv_camera_get_device(arv_camera);

    GError* err = nullptr;

    switch (p->get_value_type())
    {
        case Property::VALUE_TYPE::BOOLEAN:
        {
            // bool b = arv_device_get_boolean_feature_value(device,
            //                                               mapping.arv_ident.c_str());
            int b = arv_device_get_integer_feature_value(_device, mapping.arv_ident.c_str(), &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve bool: {}", err->message);
                g_clear_error(&err);
                break;
            }

            if (b < 0 || b > 1)
            {
                SPDLOG_ERROR(
                    "Bool has undefined internal value {} {}", mapping.arv_ident.c_str(), b);
            }
            auto struc = p->get_struct();

            struc.value.b.value = b;

            p->set_struct(struc);
            break;
        }
        case Property::VALUE_TYPE::STRING:
        case Property::VALUE_TYPE::ENUM:
        {
            p->set_value(
                arv_device_get_string_feature_value(_device, mapping.arv_ident.c_str(), &err));
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve enum/string: {}", err->message);
                g_clear_error(&err);
            }
            break;
        }
        case Property::VALUE_TYPE::INTSWISSKNIFE:
        case Property::VALUE_TYPE::INTEGER:
        {
            int i = arv_device_get_integer_feature_value(_device, mapping.arv_ident.c_str(), &err);
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve integer: {}", err->message);
                g_clear_error(&err);
                break;
            }
            auto struc = p->get_struct();

            struc.value.i.value = i;

            p->set_struct(struc);
            break;
        }
        case Property::VALUE_TYPE::FLOAT:
        {
            if (p->get_type() == TCAM_PROPERTY_TYPE_DOUBLE)
            {
                double d =
                    arv_device_get_float_feature_value(_device, mapping.arv_ident.c_str(), &err);
                if (err)
                {
                    SPDLOG_ERROR("Unable to retrieve float: {}", err->message);
                    g_clear_error(&err);
                    break;
                }

                auto struc = p->get_struct();

                struc.value.d.value = d;

                p->set_struct(struc);
            }
            else
            {

                double d =
                    arv_device_get_float_feature_value(_device, mapping.arv_ident.c_str(), &err);
                if (err)
                {
                    SPDLOG_ERROR("Unable to retrieve float: {}", err->message);
                    g_clear_error(&err);
                    break;
                }
                auto struc = p->get_struct();

                struc.value.i.value = d;

                p->set_struct(struc);
            }
            break;
        }
        case Property::VALUE_TYPE::COMMAND:
        case Property::VALUE_TYPE::BUTTON:
        case Property::VALUE_TYPE::UNDEFINED:
        default:
        {
            break;
        }
    }
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
    const char* trig_mode =
        arv_device_get_string_feature_value(arv_camera_get_device(arv_camera), "TriggerMode", &err);

    arv_camera_set_frame_rate(this->arv_camera, new_format.get_framerate(), &err);

    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera), "TriggerSelector", trig_selector, &err);
    arv_device_set_string_feature_value(
        arv_camera_get_device(arv_camera), "TriggerMode", trig_mode, &err);

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
}


std::vector<VideoFormatDescription> AravisDevice::get_available_video_formats()
{
    if (this->available_videoformats.empty())
    {
        this->index_genicam();
    }

    return available_videoformats;
}


bool AravisDevice::set_sink(std::shared_ptr<SinkInterface> s)
{
    this->external_sink = s;

    return true;
}


bool AravisDevice::initialize_buffers(std::vector<std::shared_ptr<ImageBuffer>> b)
{
    GError* err = nullptr;

    this->buffers.clear();

    this->buffers.reserve(b.size());
    int payload = arv_camera_get_payload(this->arv_camera, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve payload: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    for (unsigned int i = 0; i < b.size(); ++i)
    {
        ArvBuffer* ab = arv_buffer_new_full(payload, b.at(i)->get_data(), b.at(i).get(), nullptr);
        buffer_info info = {};
        info.buffer = b.at(i);
        info.arv_buffer = ab;
        info.is_queued = false;
        this->buffers.push_back(info);
    }
    return true;
}


bool AravisDevice::release_buffers()
{
    buffers.clear();

    return true;
}


void AravisDevice::requeue_buffer(std::shared_ptr<ImageBuffer> buffer)
{

    for (auto& b : buffers)
    {
        if (b.buffer == buffer)
        {
            //SPDLOG_DEBUG("Returning buffer to aravis.");
            arv_stream_push_buffer(this->stream, b.arv_buffer);
            b.is_queued = true;
        }
    }
}


bool AravisDevice::start_stream()
{
    if (arv_camera == nullptr)
    {
        SPDLOG_ERROR("ArvCamera missing!");
        return false;
    }

    if (buffers.size() < 2)
    {
        SPDLOG_ERROR("Need at least two buffers.");
        return false;
    }

    if (this->stream != nullptr)
    {
        g_object_unref(this->stream);
    }

    // install callback to initialize the capture thread as real time
    auto stream_cb = [](void* /*user_data*/, ArvStreamCallbackType type, ArvBuffer* /*buffer*/) {
        if (type == ARV_STREAM_CALLBACK_TYPE_INIT)
        {
            if (!arv_make_thread_realtime(10))
            {
                if (!arv_make_thread_high_priority(-10))
                {
                    SPDLOG_WARN("Unable to make aravis capture thread real time or high priority");
                }
                else
                {
                    SPDLOG_WARN("Aravis capture thread is running in high priority mode");
                }
            }
            else
            {
                SPDLOG_INFO("Aravis capture thread is running as a real time thread");
            }
        }
    };

    GError* err = nullptr;

    this->stream = arv_camera_create_stream(this->arv_camera, stream_cb, NULL, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to create stream: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    if (this->stream == nullptr)
    {
        SPDLOG_ERROR("Unable to create ArvStream.");
        return false;
    }

    if (ARV_IS_GV_STREAM(this->stream))
    {
        if (this->arv_options.auto_socket_buffer)
        {
            g_object_set(this->stream,
                         "socket-buffer",
                         ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                         "socket-buffer-size",
                         0,
                         NULL);
        }

        g_object_set(this->stream,
                     "packet-timeout",
                     (unsigned)this->arv_options.packet_timeout * 1000,
                     "frame-retention",
                     (unsigned)this->arv_options.frame_retention * 1000,
                     NULL);
        g_object_set(
            this->stream, "packet-request-ratio", this->arv_options.packet_request_ratio, NULL);
    }

    for (std::size_t i = 0; i < buffers.size(); ++i)
    {
        arv_stream_push_buffer(this->stream, buffers.at(i).arv_buffer);
    }

    arv_stream_set_emit_signals(this->stream, TRUE);

    arv_camera_set_acquisition_mode(this->arv_camera, ARV_ACQUISITION_MODE_CONTINUOUS, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set acquisition mode: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    // a work thread is not required as aravis already pushes the images asynchroniously

    g_signal_connect(stream, "new-buffer", G_CALLBACK(callback), this);

    SPDLOG_INFO("Starting actual stream...");

    arv_camera_start_acquisition(this->arv_camera, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to start stream: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    statistics = {};

    return true;
}


bool AravisDevice::stop_stream()
{
    if (arv_camera == NULL)
    {
        return false;
    }
    GError* err = nullptr;

    arv_camera_stop_acquisition(arv_camera, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to stop stream: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    if (this->stream != nullptr)
    {
        arv_stream_set_emit_signals(this->stream, FALSE);
        g_object_unref(this->stream);
        this->stream = NULL;
    }

    return true;
}


void AravisDevice::determine_packet_request_ratio()
{
    std::string arv_prr = tcam::get_environment_variable("TCAM_ARV_PACKET_REQUEST_RATIO", "0.1");

    double eps = 0.0;

    try
    {
        eps = std::stof(arv_prr);
    }
    catch (...)
    {
        SPDLOG_WARN("Unable to interpret the value for TCAM_ARV_PACKET_REQUEST_RATIO. Falling back "
                    "to default value.");
    }

    if (eps <= 0.0 || eps > 1.0)
    {
        this->arv_options.packet_request_ratio = 0.1;
    }
    else
    {
        this->arv_options.packet_request_ratio = eps;
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


void AravisDevice::callback(ArvStream* stream __attribute__((unused)), void* user_data)
{
    AravisDevice* self = static_cast<AravisDevice*>(user_data);
    if (self == NULL)
    {
        SPDLOG_ERROR("Callback camera instance is NULL.");
        return;
    }
    if (self->stream == NULL)
    {
        return;
    }

    ArvBuffer* buffer = arv_stream_pop_buffer(self->stream);

    if (buffer != NULL)
    {
        ArvBufferStatus status = arv_buffer_get_status(buffer);

        if (status == ARV_BUFFER_STATUS_SUCCESS)
        {
            SPDLOG_TRACE("Received new buffer.");

            self->statistics.capture_time_ns = arv_buffer_get_system_timestamp(buffer);
            self->statistics.camera_time_ns = arv_buffer_get_timestamp(buffer);
            self->statistics.frame_count++;
            self->statistics.is_damaged = false;
            // only way to retrieve actual image size
            size_t image_size = 0;
            arv_buffer_get_data(buffer, &image_size);

            for (auto& b : self->buffers)
            {
                const void* arv_user_data = arv_buffer_get_user_data(buffer);
                if (b.buffer.get() != arv_user_data)
                {
                    continue;
                }

                b.buffer->set_statistics(self->statistics);
                if (auto ptr = self->external_sink.lock())
                {
                    b.is_queued = false;
                    auto desc = b.buffer->getImageBuffer();
                    desc.length = image_size;
                    b.buffer->set_image_buffer(desc);
                    ptr->push_image(b.buffer);
                }
                else
                {
                    SPDLOG_ERROR("ImageSink expired. Unable to deliver images.");
                    arv_stream_push_buffer(self->stream, buffer);
                    return;
                }
            }
        }
        else
        {
            std::string msg;

            switch (status)
            {
                case ARV_BUFFER_STATUS_SUCCESS:
                {
                    msg = "the buffer is cleared";
                    break;
                }
                case ARV_BUFFER_STATUS_TIMEOUT:
                {
                    msg = "Timeout has been reached before all packets were received";
                    break;
                }
                case ARV_BUFFER_STATUS_MISSING_PACKETS:
                {
                    msg = "Stream has missing packets";

                    if (auto ptr = self->external_sink.lock())
                    {

                        if (ptr->should_incomplete_frames_be_dropped())
                        {
                            break;
                        }
                        SPDLOG_WARN(
                            "Image has missing packets. Sending incomplete buffer as requested.");
                        self->statistics.capture_time_ns = arv_buffer_get_timestamp(buffer);
                        self->statistics.camera_time_ns = arv_buffer_get_timestamp(buffer);
                        self->statistics.frame_count++;
                        self->statistics.is_damaged = true;

                        // only way to retrieve actual image size
                        size_t image_size = 0;
                        arv_buffer_get_data(buffer, &image_size);

                        for (auto& b : self->buffers)
                        {
                            const void* arv_user_data = arv_buffer_get_user_data(buffer);
                            if (b.buffer.get() != arv_user_data)
                            {
                                continue;
                            }

                            b.buffer->set_statistics(self->statistics);
                            b.is_queued = false;
                            auto desc = b.buffer->getImageBuffer();
                            desc.length = image_size;
                            b.buffer->set_image_buffer(desc);
                            ptr->push_image(b.buffer);
                        }
                    }

                    goto no_back_push;
                }
                case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
                {
                    msg = "Stream has packet with wrong id";
                    break;
                }
                case ARV_BUFFER_STATUS_SIZE_MISMATCH:
                {
                    msg = "The received image did not fit in the buffer data space";
                    break;
                }
                case ARV_BUFFER_STATUS_FILLING:
                {
                    msg = "The image is currently being filled";
                    break;
                }
                case ARV_BUFFER_STATUS_ABORTED:
                {
                    msg = "The filling was aborted before completion";
                    break;
                }
                case ARV_BUFFER_STATUS_CLEARED:
                {
                    msg = "Buffer cleared";
                    break;
                }
                case ARV_BUFFER_STATUS_UNKNOWN:
                {
                    msg = "This should not happen";
                    break;
                }
            }
            arv_stream_push_buffer(self->stream, buffer);
no_back_push:
            SPDLOG_WARN(msg.c_str());
        }
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
    index_genicam_format(NULL);
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

        // TODO: how to handle private settings
        static std::vector<std::string> private_settings = {
            "TLParamsLocked",
            "GevSCPSDoNotFragment",
            "GevTimestampTickFrequency",
            "GevTimeSCPD",
            "GevSCPD",
            "PayloadSize",
            "PayloadPerFrame",
            "PayloadPerPacket",
            "TotalPacketSize",
            "PacketsPerFrame",
            "PacketTimeUS",
            "GevSCPSPacketSize",
            "GevSCPSFireTestPacket",
            "DeviceVendorName",
            "DeviceType",
            "DeviceModelType",
            "DeviceVersion",
            "DeviceSerialNumber",
            "DeviceUserID",
            "DeviceSFNCVersionMajor",
            "DeviceSFNCVersionMinor",
            "DeviceTLType",
            "DeviceTLTypeMajor",
            "DeviceTLTypeMinor",
            "DeviceTLTypeSubMinor",
            "DeviceLinkSelector",
            "WidthMax",
            "HeightMax",
            "ChunkModeActive",
            "ActionDeviceKey",
            "ActionSelector",
            "ActionGroupMask",
            "ActionGroupKey",
            "UserSetSelector",
            "UserSetLoad",
            "UserSetSave",
            "UserSetDefault",
            "DeviceScanType",
            "StringReg",
            "DeviceModelName",
            "DeviceSFNCVersionSubMinor",
            "MaskedIntReg",
            "DeviceTLVersionMajor",
            "MaskedIntReg",
            "DeviceTLVersionMinor",
            "DeviceTLVersionSubMinor",
            "DeviceLinkHeartbeatTimeout",
            "DeviceStreamChannelCount",
            "DeviceStreamChannelSelector",
            "DeviceStreamChannelType",
            "DeviceStreamChannelLink",
            "DeviceStreamChannelEndianness",
            "DeviceStreamChannelPacketSize",
            "DeviceEventChannelCount",
            "IMX174HardwareWDRShutterMode",
            "IMX174HardwareWDREnable",
            "IMX174WDRShutter2",
            "ChunkIMX174FrameSet",
            "ChunkIMX174FrameId",
            "SensorPixelHeight",
            "SensorPixelWidth",
        };


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


        if (std::find(private_settings.begin(), private_settings.end(), feature)
            != private_settings.end())
        {
            // TODO: implement handling
        }
        // is part of the format description
        else if (std::find(format_member.begin(), format_member.end(), feature)
                 != format_member.end())
        {
            // index_genicam_format(camera, node, frmt_mapping);
            this->format_nodes.push_back(node);
        }
        else
        {
            property_mapping m;

            m.arv_ident = feature;
            m.prop = create_property(arv_camera, node, handler);

            if (m.prop == nullptr)
            {
                SPDLOG_ERROR("Property '{}' is null", m.arv_ident.c_str());
                return;
            }

            handler->properties.push_back(m);
        }
    }
}


void AravisDevice::index_genicam_format(ArvGcNode* /* node */)
{
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

    node_to_use = "Binning";
    auto binning_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);

    std::vector<int> binning;

    if (binning_node != format_nodes.end())
    {

        if (ARV_IS_GC_ENUMERATION(*binning_node))
        {
            const GSList* childs;
            const GSList* iter;

            childs = arv_gc_enumeration_get_entries(ARV_GC_ENUMERATION(*binning_node));
            if (err)
            {
                SPDLOG_ERROR("Unable to retrieve enum entries: {}", err->message);
                g_clear_error(&err);
            }
            for (iter = childs; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented((ArvGcFeatureNode*)iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name((ArvDomNode*)iter->data), "EnumEntry")
                        == 0)
                    {
                        GError* error = NULL;

                        // this is the denominator of our framerate
                        int64_t val =
                            arv_gc_enum_entry_get_value(ARV_GC_ENUM_ENTRY(iter->data), &error);

                        // std::cout << "Binning entry: "
                        // << arv_gc_feature_node_get_name ((ArvGcFeatureNode*)iter->data)
                        // << " - "
                        // << val << std::endl;

                        binning.push_back(val);
                    }
                }
            }
        }
        else
        {
            // int range
            // TODO implement
        }
    }
    else
    {
        // default value
        binning.push_back(0);
    }

    // TODO are there cameras that only have fixed resolutions?

    int width_min = 0;
    int width_max = 0;
    int width_step = 0;

    int height_min = 0;
    int height_max = 0;
    int height_step = 0;

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
