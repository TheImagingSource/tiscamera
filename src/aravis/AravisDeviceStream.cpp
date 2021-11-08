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

#include "AravisDevice.h"

#include "ImageBuffer.h"
#include "logging.h"
#include "utils.h"

#include <vector>
#include <memory>

using namespace tcam;

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



bool set_stream_options(ArvStream* stream)
{

    // helper functions
    // these attempt to set a certain value type
    // if a conversion fails we return false and
    // the stream object will not be touched

    auto set_int = [&stream] (const std::string& name, const std::string& value)
    {
        try
        {
            // stoi likes to convert 0.8 to 0.
            // check if point is in value
            // -> is double, don't even try
            if (value.find(".") != std::string::npos)
            {
                return false;
            }

            auto res = std::stoi(value);
            g_object_set(stream,
                         name.c_str(),
                         res,
                         nullptr);

            int test;
            g_object_get(stream, name.c_str(), &test, nullptr);

            if (test != res)
            {
                SPDLOG_ERROR("Setting {} did not go as expected. {} != {}", name, test, res);
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
        return true;
    };

    auto set_double = [&stream] (const std::string& name, const std::string& value)
    {
        try
        {
            // there are no values that want more than two decimal position
            // ceilf seems to produce more reliable values than ceil
            double res = (double)ceilf(std::stof(value) * 100.0L) / 100.0L;
            g_object_set(stream,
                         name.c_str(),
                         res,
                         nullptr);

            double test;
            g_object_get(stream, name.c_str(), &test, nullptr);

            if (test != res)
            {
                SPDLOG_ERROR("Setting {} did not go as expected. {} != {}", name, test, res);
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
        return true;
    };

    auto set_enum = [&stream] (const std::string& name, const std::string& value)
    {
        GObjectClass* klass = G_OBJECT_GET_CLASS(stream);

        if (!klass)
        {
            SPDLOG_ERROR("No GObject klass for arv_stream");
            return false;
        }

        // what happens here:
        // we retrieve all properties of arvstream
        // search for a property with matching name
        // look up the GEnum entry in the hope that the value string matches
        // then set the actual value

        uint n_props = 0;
        GParamSpec** props = g_object_class_list_properties(klass, &n_props);

        for (unsigned int i = 0; i < n_props; ++i)
        {
            if (strcmp(props[i]->name, name.c_str()) == 0)
            {
                auto entry = g_enum_get_value_by_name((GEnumClass*)g_type_class_peek(props[i]->value_type), value.c_str());

                if (entry)
                {
                    g_object_set(stream,
                                 name.c_str(),
                                 entry->value,
                                 nullptr);

                    g_free(props);

                    // potential debug code
                    // leave it
                    // as it is a mess to find the correct functions to coll

                    // gint resend = -1;
                    // g_object_get(stream, "packet-resend", &resend, nullptr);
                    // auto val = g_enum_get_value((GEnumClass*)g_type_class_peek(g_type_from_name("ArvGvStreamPacketResend")),
                    //                             resend);
                    // SPDLOG_ERROR("==== packet resend = {}", val->value_name);

                    return true;
                }
                // we already have the valid entry
                // since no interpretation is possible
                // be can simply abort;
                break;
            }
        }

        g_free(props);

        return false;
    };

    // actual function

    std::string stream_options = tcam::get_environment_variable("TCAM_ARV_STREAM_OPTIONS", "");

    if (!stream_options.empty())
    {
        auto settings = tcam::split_string(stream_options, ",");

        for (auto s : settings)
        {
            auto single_setting = tcam::split_string(s, "=");

            if (single_setting.size() != 2)
            {
                SPDLOG_ERROR("Unable to interpret TCAM_ARV_STREAM_OPTIONS setting: {}", s);
            }
            else
            {
                // attempt setting int
                // if it fails the value is not an int
                // and we attempt double, and if that fails it has to be an enum
                // if that also fails hte user made an error and gave an invalid value
                if (!set_int(single_setting.at(0), single_setting.at(1)))
                {
                    if (!set_double(single_setting.at(0), single_setting.at(1)))
                    {
                        if (!set_enum(single_setting.at(0), single_setting.at(1)))
                        {
                            SPDLOG_ERROR("TCAM_ARV_STREAM_OPTIONS: value for '{}' could not be interpreted. Value is: '{}'",
                                         single_setting.at(0), single_setting.at(1));
                        }
                    }
                }
            }
        }
    }

    return true;
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
        set_stream_options(this->stream);
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
