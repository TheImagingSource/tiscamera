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

#include "../ImageBuffer.h"
#include "../logging.h"
#include "../utils.h"
#include "AravisDevice.h"

#include <memory>
#include <vector>

using namespace tcam;

void tcam::AravisDevice::clear_buffer_info_arb_buffer(buffer_info& info)
{
    std::scoped_lock lck { info.parent->buffer_list_mtx_ };

    info.arv_buffer = nullptr;
}

bool AravisDevice::initialize_buffers(std::shared_ptr<BufferPool> pool)
{
    auto new_list = pool->get_buffer();

    std::scoped_lock lck { arv_camera_access_mutex_ };

    GError* err = nullptr;

    this->buffer_list_.clear();
    this->buffer_list_.reserve(new_list.size());

    size_t payload = arv_camera_get_payload(this->arv_camera_, &err);
    if (err)
    {
        SPDLOG_ERROR("Unable to retrieve payload: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    auto buffer_destroy_notfiy = [](void* buffer_info_ptr) {
        auto& info = *static_cast<buffer_info*>(buffer_info_ptr);
        clear_buffer_info_arb_buffer(info);
    };

    size_t buffer_size = new_list.front().lock()->get_image_buffer_size();
    if( buffer_size < payload)
    {
        SPDLOG_WARN("Aravis payload-size ({}) > image_buffer_size ({})", payload, buffer_size);
    }

    // we build the according items in a 2 step process, so we are able to pass &info to arv_buffer_new_full
    for (auto&& buffer : new_list) { this->buffer_list_.push_back(buffer_info { this, buffer.lock() }); }

    for (auto& info : buffer_list_)
    {
        info.arv_buffer = arv_buffer_new_full(
            buffer_size, info.buffer->get_image_buffer_ptr(), &info, buffer_destroy_notfiy);
    }
    return true;
}

bool AravisDevice::release_buffers()
{
    std::scoped_lock lck { arv_camera_access_mutex_ };

    // this should not be taken here, because the g_object_unref triggers the
    // destroy notify of the arv_buffer which in turn takes the mutex
    //std::scoped_lock lck { buffer_list_mtx_ };

    for (auto& b : buffer_list_)  // these get partially freed by the arv_stream, partially here
    {
        if (b.arv_buffer)
        {
            g_object_unref(b.arv_buffer);
        }
    }

    buffer_list_.clear();

    return true;
}

void AravisDevice::requeue_buffer(const std::shared_ptr<ImageBuffer>& buffer)
{
    std::scoped_lock lck0 { arv_camera_access_mutex_ };

    std::scoped_lock lck { buffer_list_mtx_ };
    for (auto& b : buffer_list_)
    {
        if (b.buffer == buffer && b.arv_buffer != nullptr)
        {
#if !defined NDEBUG
            b.is_queued = true;
#endif
            arv_stream_push_buffer(this->stream_, b.arv_buffer);
            return;
        }
    }

    // we can just drop it here
    SPDLOG_DEBUG("Buffer not requeued. Already flushed from buffer_list. ptr={}.",
                 static_cast<void*>(buffer.get()));
}

static bool set_stream_options(ArvStream* stream)
{

    // helper functions
    // these attempt to set a certain value type
    // if a conversion fails we return false and
    // the stream object will not be touched

    auto set_int = [&stream](const std::string& name, const std::string& value)
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
            g_object_set(stream, name.c_str(), res, nullptr);

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

    auto set_double = [&stream](const std::string& name, const std::string& value)
    {
        try
        {
            // there are no values that want more than two decimal position
            // ceilf seems to produce more reliable values than ceil
            double res = (double)ceilf(std::stof(value) * 100.0L) / 100.0L;
            g_object_set(stream, name.c_str(), res, nullptr);

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

    auto set_enum = [&stream](const std::string& name, const std::string& value)
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
                auto entry = g_enum_get_value_by_name(
                    (GEnumClass*)g_type_class_peek(props[i]->value_type), value.c_str());

                if (entry)
                {
                    g_object_set(stream, name.c_str(), entry->value, nullptr);

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
                            SPDLOG_ERROR("TCAM_ARV_STREAM_OPTIONS: value for '{}' could not be "
                                         "interpreted. Value is: '{}'",
                                         single_setting.at(0),
                                         single_setting.at(1));
                        }
                    }
                }
            }
        }
    }

    return true;
}


bool AravisDevice::start_stream(const std::shared_ptr<IImageBufferSink>& sink)
{
    std::scoped_lock lck0 { arv_camera_access_mutex_ };

    if (arv_camera_ == nullptr)
    {
        SPDLOG_ERROR("ArvCamera missing!");
        return false;
    }

    assert(stream_ == nullptr);
    if (stream_)
    {
        SPDLOG_ERROR("Stop was not called previously");
        return false;
    }

    if (buffer_list_.size() < 2)
    {
        SPDLOG_ERROR("Need at least two buffers.");
        return false;
    }


    // install callback to initialize the capture thread as real time
    auto stream_cb = [](void* /*user_data*/, ArvStreamCallbackType type, ArvBuffer* /*buffer*/)
    {
        if (type == ARV_STREAM_CALLBACK_TYPE_INIT)
        {
            if (!arv_make_thread_realtime(10))
            {
                if (!arv_make_thread_high_priority(-10))
                {
                    SPDLOG_INFO("Unable to make aravis capture thread real time or high priority");
                }
                else
                {
                    SPDLOG_INFO("Aravis capture thread is running in high priority mode");
                }
            }
            else
            {
                SPDLOG_INFO("Aravis capture thread is running as a real time thread");
            }
        }
    };

    disable_chunk_mode();

    GError* err = nullptr;

    this->stream_ = arv_camera_create_stream(this->arv_camera_, stream_cb, NULL, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to create stream: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    if (this->stream_ == nullptr)
    {
        SPDLOG_ERROR("Unable to create ArvStream.");
        return false;
    }

    if (ARV_IS_GV_STREAM(this->stream_))
    {
        set_stream_options(this->stream_);
    }

    for (auto& buf : buffer_list_) { arv_stream_push_buffer(this->stream_, buf.arv_buffer); }

    arv_stream_set_emit_signals(this->stream_, TRUE);

    arv_camera_set_acquisition_mode(this->arv_camera_, ARV_ACQUISITION_MODE_CONTINUOUS, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to set acquisition mode: {}", err->message);
        g_clear_error(&err);
        return false;
    }

    // a work thread is not required as aravis already pushes the images asynchronously

    g_signal_connect(stream_, "new-buffer", G_CALLBACK(aravis_new_buffer_callback), this);

    SPDLOG_INFO("Starting actual stream...");

    frames_delivered_ = 0;
    frames_dropped_ = 0;

    sink_ = sink;

    arv_camera_start_acquisition(this->arv_camera_, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to start stream: {}", err->message);
        g_clear_error(&err);

        stop_stream();

        return false;
    }

    return true;
}


void AravisDevice::stop_stream()
{
    std::scoped_lock lck0 { arv_camera_access_mutex_ };

    if (arv_camera_ == NULL)
    {
        return;
    }
    GError* err = nullptr;

    if (stream_)
    {
        // emit_signals has to be disabled to prevent
        // to prevent a warning
        // disable it before stop_acquisition
        // as under _some_ circumstances it may end
        // up locking up in the mutex
        arv_stream_set_emit_signals(this->stream_, FALSE);
    }

    arv_camera_stop_acquisition(arv_camera_, &err);

    if (err)
    {
        SPDLOG_ERROR("Unable to stop stream: {}", err->message);
        g_clear_error(&err);
        return;
    }

    if (this->stream_ != nullptr)
    {
        g_object_unref(this->stream_);
        this->stream_ = NULL;
    }

    // releasing the stream deletes all arv_buffer objects currently pending in the arv_stream, so we cannot re-use the actaul ImageBuffers here

    sink_.reset();

    release_buffers();
}

static auto translate_arv_buffer_status(ArvBufferStatus status) -> const char*
{
    switch (status)
    {
        case ARV_BUFFER_STATUS_SUCCESS:
        {
            return "the buffer is cleared";
        }
        case ARV_BUFFER_STATUS_TIMEOUT:
        {
            return "Timeout has been reached before all packets were received";
        }
        case ARV_BUFFER_STATUS_MISSING_PACKETS:
        {
            return "Stream has missing packets";
        }
        case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
        {
            return "Stream has packet with wrong id";
        }
        case ARV_BUFFER_STATUS_SIZE_MISMATCH:
        {
            return "The received image did not fit in the buffer data space";
        }
        case ARV_BUFFER_STATUS_FILLING:
        {
            return "The image is currently being filled";
        }
        case ARV_BUFFER_STATUS_ABORTED:
        {
            return "The filling was aborted before completion";
        }
        case ARV_BUFFER_STATUS_CLEARED:
        {
            return "Buffer cleared";
        }
        case ARV_BUFFER_STATUS_UNKNOWN:
        {
            return "This should not happen";
        }
    }
    return nullptr;
}


void AravisDevice::aravis_new_buffer_callback(ArvStream* stream __attribute__((unused)),
                                              void* user_data)
{
    AravisDevice* self = static_cast<AravisDevice*>(user_data);
    if (self == NULL)
    {
        SPDLOG_ERROR("Callback camera instance is NULL.");
        return;
    }
    // This should not be a problem, because stop stream guards against this by killing all callbacks before removing the stream variable
    // std::scoped_lock lck0 { self->arv_camera_access_mutex_ }; // disable this becase we call into SoftwareProperties which may lock this mutex and its own mutex too
    if (self->stream_ == NULL)
    {
        return;
    }

    ArvBuffer* buffer = arv_stream_pop_buffer(stream);
    if (buffer == NULL)
    {
        return;
    }

    ArvBufferStatus status = arv_buffer_get_status(buffer);

    if (status == ARV_BUFFER_STATUS_SUCCESS)
    {
        self->complete_aravis_stream_buffer(buffer, false);
    }
    else if (status == ARV_BUFFER_STATUS_MISSING_PACKETS)
    {
        if (self->drop_incomplete_frames_)
        {
            SPDLOG_DEBUG("Image has missing packets. Dropping incomplete frame as requested.");

            ++self->frames_dropped_;

            arv_stream_push_buffer(stream, buffer);
        }
        else
        {
            SPDLOG_DEBUG("Image has missing packets. Sending incomplete buffer as requested.");

            self->complete_aravis_stream_buffer(buffer, true);
        }
    }
    else
    {
        ++self->frames_dropped_;

        arv_stream_push_buffer(self->stream_, buffer);
        auto ptr = translate_arv_buffer_status(status);
        if (ptr)
        {
            SPDLOG_DEBUG("arvBufferStatus: {}", ptr);
        }
    }
}

void AravisDevice::complete_aravis_stream_buffer(ArvBuffer* buffer, bool is_incomplete)
{
    // receives the actual ImageBuffer from the ArvBuffer
    std::shared_ptr<ImageBuffer> completed_buffer;
    {
        //std::scoped_lock lck { buffer_list_mtx_ };

        buffer_info& arv_user_data = *const_cast<buffer_info*>(
            static_cast<const buffer_info*>(arv_buffer_get_user_data(buffer)));

#if !defined NDEBUG
        arv_user_data.is_queued = false;
#endif
        completed_buffer = arv_user_data.buffer;
    }

    if (completed_buffer == nullptr) // this should never happen
    {
        ++frames_dropped_;

        SPDLOG_ERROR("Failed to find the associated ImageBuffer for the completed arv buffer.");
        arv_stream_push_buffer(stream_, buffer);
        return;
    }

    if (auto ptr = sink_.lock())
    {
        ++frames_delivered_;

        // only way to retrieve actual image size
        size_t image_size = 0;
        arv_buffer_get_data(buffer, &image_size);

        tcam_stream_statistics stats = {};
        stats.capture_time_ns = arv_buffer_get_system_timestamp(buffer);
        stats.camera_time_ns = arv_buffer_get_timestamp(buffer);
        stats.frame_count = frames_delivered_;
        stats.frames_dropped = frames_dropped_;
        stats.is_damaged = is_incomplete;

        completed_buffer->set_statistics(stats);
        completed_buffer->set_valid_data_length(image_size);
        ptr->push_image(completed_buffer);
    }
    else
    {
        ++frames_dropped_;

        SPDLOG_ERROR("ImageSink expired. Unable to deliver images.");

        requeue_buffer(completed_buffer);
    }
}
