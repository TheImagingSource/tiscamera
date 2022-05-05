/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "AFU050Device.h"

#include "../logging.h"
#include "AFU050DeviceBackend.h"
#include "AFU050PropertyImpl.h"
#include "UsbHandler.h"
#include "UsbSession.h"
#include "afu050_definitions.h"

#include <algorithm>
#include <cstring>
#include <dutils_img/fcc_to_string.h>
#include <fstream>
#include <unistd.h>

using namespace tcam;


tcam::AFU050Device::AFU050Device(const DeviceInfo& info)
{
    device = info;

    usb_device_ = UsbHandler::get_instance().open_device_(info.get_serial());

    // LibusbDevice tracks open interfaces and closes them automatically
    if (!usb_device_->open_interface(0))
    {
        SPDLOG_ERROR("Failed to open camera interface - {}. \n"
                     "Please check device permissions!",
                     0);
    }

    if (!usb_device_->open_interface(1))
    {
        SPDLOG_ERROR("Failed to open camera interface - {}. \n"
                     "Please check device permissions!",
                     1);
    }

    backend_ = std::make_shared<tcam::property::AFU050DeviceBackend>(this);

    create_properties();
    create_formats();
}

tcam::AFU050Device::~AFU050Device()
{
    stop_stream();
}

void tcam::AFU050Device::lost_device()
{
    if (device_is_lost_)
    {
        return;
    }
    device_is_lost_ = true;

    stop_stream();

    notify_device_lost();
}


DeviceInfo tcam::AFU050Device::get_device_description() const
{
    return device;
}


void AFU050Device::create_formats()
{

    // typedef enum
    // {
    // AFU050_FMT_2592X1944 = 1,
    // AFU050_FMT_1920X1080 = 2,
    // AFU050_FMT_1280X960 = 3
    // }afu050_video_format_t;

    struct tcam_video_format_description desc = {};

    desc.fourcc = FOURCC_MJPG;
    memcpy(desc.description, img::fcc_to_string(desc.fourcc).c_str(), sizeof(desc.description));

    std::vector<struct framerate_mapping> rf;

    auto add_res = [&rf](int width, int height, double fps)
    {
        struct tcam_resolution_description res = {};
        res.type = TCAM_RESOLUTION_TYPE_FIXED;
        res.min_size.width = width;
        res.min_size.height = height;
        res.max_size.width = width;
        res.max_size.height = height;

        std::vector<double> f;
        f.push_back(fps);

        framerate_mapping r = { res, f };
        rf.push_back(r);
    };

    add_res(2592, 1944, 15.0);
    add_res(1920, 1080, 30.0);
    add_res(1280, 960, 60.0);

    VideoFormatDescription format(nullptr, desc, rf);
    available_videoformats_.push_back(format);
}


void AFU050Device::add_dependency_tracking()
{
    for (auto& p : properties_)
    {
        auto ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(p);
        if (!ptr)
            continue;

        auto dep_names = ptr->get_dependent_names();
        if (dep_names.empty())
        {
            continue;
        }

        std::vector<std::weak_ptr<tcam::property::PropertyLock>> can_be_locked;

        for (auto& possible_dependency : properties_)
        {
            if (std::find(dep_names.begin(), dep_names.end(), possible_dependency->get_name())
                != dep_names.end())
            {
                can_be_locked.push_back(
                    std::dynamic_pointer_cast<tcam::property::PropertyLock>(possible_dependency));
            }
        }

        if (!can_be_locked.empty())
        {
            ptr->set_dependent_properties(std::move(can_be_locked));
        }
    }
}


void AFU050Device::create_properties()
{
    add_double("ExposureTime", VC_UNIT_INPUT_TERMINAL, CT_EXPOSURE_TIME_ABSOLUTE_CONTROL, 1. / 100. );
    add_enum("ExposureAuto",
             VC_UNIT_EXTENSION_UNIT,
             XU_AUTO_EXPOSURE,
             { { { 0, "Off" }, { 1, "Continuous" } } });

    add_double("Gain", VC_UNIT_PROCESSING_UNIT, PU_GAIN_CONTROL);
    add_enum("GainAuto",
             VC_UNIT_EXTENSION_UNIT,
             XU_AUTO_GAIN,
             { { { 0, "Off" }, { 1, "Continuous" } } });

    add_enum("FocusAuto",
             VC_UNIT_EXTENSION_UNIT,
             XU_FOCUS_ONE_PUSH,
             { { { 0, "Off" }, { 1, "Once" } } });

    static const double wb_mod = 1023.75;

    add_double("BalanceWhiteRed", VC_UNIT_EXTENSION_UNIT, XU_GAIN_R_CONTROL, wb_mod);
    add_double("BalanceWhiteGreen", VC_UNIT_EXTENSION_UNIT, XU_GAIN_G_CONTROL, wb_mod);
    add_double("BalanceWhiteBlue", VC_UNIT_EXTENSION_UNIT, XU_GAIN_B_CONTROL, wb_mod);
    add_enum("BalanceWhiteAuto",
             VC_UNIT_EXTENSION_UNIT,
             XU_AUTO_WHITE_BALANCE,
             { { { 0, "Off" }, { 1, "Continuous" } } });

    add_double("Saturation", VC_UNIT_PROCESSING_UNIT, PU_SATURATION_CONTROL, 32. / 100. );
    add_int("Contrast", VC_UNIT_PROCESSING_UNIT, PU_CONTRAST_CONTROL);
    add_int("Sharpness", VC_UNIT_PROCESSING_UNIT, PU_SHARPNESS_CONTROL);

    // read only properties
    // no backend etc needed

    tcam_value_int width;
    width.min = 2592;
    width.max = 2592;
    width.value = 2592;
    width.default_value = 2592;
    width.step = 0;

    properties_.push_back(
        std::make_shared<tcam::property::AFU050PropertyIntegerImpl>("SensorWidth", width));

    tcam_value_int height;
    height.min = 1944;
    height.max = 1944;
    height.value = 1944;
    height.default_value = 1944;
    height.step = 0;

    properties_.push_back(
        std::make_shared<tcam::property::AFU050PropertyIntegerImpl>("SensorHeight", height));

    // add_int(TCAM_PROPERTY_FOCUS_X, VC_UNIT_EXTENSION_UNIT, PU_);
    // add_int(TCAM_PROPERTY_FOCUS_Y, VC_UNIT_EXTENSION_UNIT, PU_);

    add_dependency_tracking();
}


bool tcam::AFU050Device::set_video_format(const VideoFormat& format)
{
    if (is_stream_on_)
    {
        SPDLOG_ERROR("Unable to set format. Stream is running.");
        return false;
    }
    // {
    // AFU050_FMT_2592X1944 = 1,
    // AFU050_FMT_1920X1080 = 2,
    // AFU050_FMT_1280X960 = 3
    // }afu050_video_format_t;

    auto get_format_index = [](const VideoFormat& _format)
    {
        auto s = _format.get_struct();
        if (s.width == 2592 && s.height == 1944 && s.framerate == 15.0)
        {
            return 1;
        }
        else if (s.width == 1920 && s.height == 1080 && s.framerate == 30.0)
        {
            return 2;
        }
        else if (s.width == 1280 && s.height == 960 && s.framerate == 60.0)
        {
            return 3;
        }
        else
        {
            return -1;
        }
    };

    int index = get_format_index(format);

    if (index == -1)
    {
        SPDLOG_ERROR("Format is not supported. {}", format.to_string().c_str());
        return false;
    }
    active_video_format_ = format;
    if (set_video_format(0, index, 0) > 0)
        return true;
    else
        return false;
}


bool tcam::AFU050Device::validate_video_format(const VideoFormat&) const
{
    return false;
}


VideoFormat tcam::AFU050Device::get_active_video_format() const
{
    // TODO: set when initializing device;
    return active_video_format_;
}


std::vector<VideoFormatDescription> tcam::AFU050Device::get_available_video_formats()
{
    return available_videoformats_;
}


bool tcam::AFU050Device::set_framerate(double framerate __attribute__((unused)))
{
    return false;
}

double tcam::AFU050Device::get_framerate()
{
    return active_video_format_.get_framerate();
}

bool tcam::AFU050Device::initialize_buffers(std::shared_ptr<BufferPool> pool)
{
    auto buffs = pool->get_buffer();
    std::scoped_lock lck { buffer_list_mtx_ };

    buffer_list_.clear();
    buffer_list_.reserve(buffs.size());

    for (auto& b : buffs) { buffer_list_.push_back({ b.lock(), true }); }
    return true;
}

void tcam::AFU050Device::init_buffers()
{
    std::scoped_lock lck { buffer_list_mtx_ };

    static const int num_buffers = 10;

    buffer_list_.clear();
    buffer_list_.reserve(num_buffers);

    const auto fmt = get_active_video_format();

    size_t max_size = AFU050Device::JPEGBUF_SIZE;

    for (unsigned int i = 0; i < num_buffers; ++i)
    {
        buffer_list_.push_back({ std::make_shared<ImageBuffer>(fmt, max_size), false });
    }
}

bool tcam::AFU050Device::release_buffers()
{
    std::scoped_lock lck { buffer_list_mtx_ };

    buffer_list_.clear();

    return true;
}

std::shared_ptr<tcam::ImageBuffer> AFU050Device::get_free_buffer()
{
    std::scoped_lock lck { buffer_list_mtx_ };

    for (auto& buf : buffer_list_)
    {
        if (buf.is_queued)
        {
            buf.is_queued = false;
            return buf.buffer;
        }
    }
    return nullptr;
}

void tcam::AFU050Device::requeue_buffer(const std::shared_ptr<ImageBuffer>& buf)
{
    std::scoped_lock lck { buffer_list_mtx_ };

    for (auto& b : buffer_list_)
    {
        if (b.buffer->get_image_buffer_ptr() == buf->get_image_buffer_ptr())
        {
            b.is_queued = true;
            return;
        }
    }
}


void tcam::AFU050Device::transfer_callback(libusb_transfer* transfer)
{
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
        if (transfer->status == LIBUSB_TRANSFER_CANCELLED)
        {
            libusb_free_transfer(transfer);
            return;
        }
        else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
        {
            lost_device();
            libusb_free_transfer(transfer);
            return;
        }
        SPDLOG_ERROR("libusb transfer returned with: {}", transfer->status);
    }

    if (!is_stream_on_)
    {
        libusb_free_transfer(transfer);
        return;
    }

    int processed = 0;

    unsigned char* ptr = transfer->buffer;

    while (processed < transfer->actual_length)
    {
        if (!current_jpegbuf_ptr_)
        {
            unsigned char* start_of_image = (unsigned char*)memmem(
                ptr + processed, (transfer->actual_length - processed), "\xff\xd8", 2);
            if (start_of_image)
            {
                if (!current_jpegbuf_ptr_)
                {
                    current_jpegbuf_ptr_ = current_jpegbuf_data_.data();
                    //current_jpegbuf_ptr_ = (unsigned char*)malloc(JPEGBUF_SIZE);
                }

                processed = start_of_image - transfer->buffer;
                memcpy(current_jpegbuf_ptr_, start_of_image, 2);
                processed += 2;
                current_jpegsize_ += 2;
                current_jpegbuf_index_ += 2;
            }
        }

        if (current_jpegbuf_ptr_)
        {
            unsigned char* end_of_image = (unsigned char*)memmem(
                ptr + processed, transfer->actual_length - processed, "\xff\xd9", 2);
            int cplen = transfer->actual_length - processed;

            if (end_of_image)
            {
                cplen = (((end_of_image)-ptr) - processed) + 2;
            }

            if ((current_jpegsize_ + cplen) > JPEGBUF_SIZE)
            {
                SPDLOG_ERROR("Image is too big. Dropping...");
                //JPEG too big, dropping
                current_jpegbuf_index_ = 0;
                current_jpegsize_ = 0;
                current_jpegbuf_ptr_ = 0;
                break;
            }

            memcpy(current_jpegbuf_ptr_ + current_jpegbuf_index_, ptr + processed, cplen);
            processed += cplen;
            current_jpegbuf_index_ += cplen;
            current_jpegsize_ += cplen;

            if (end_of_image)
            {
                std::shared_ptr<tcam::ImageBuffer> buffer = get_free_buffer();
                if (buffer == nullptr)
                {
                    ++frames_dropped_;
                    SPDLOG_TRACE("Failed to fetch free buffer");
                }
                else
                {
                    assert(buffer->get_image_buffer_size() >= current_jpegsize_);
                    memcpy(buffer->get_image_buffer_ptr(), current_jpegbuf_ptr_, current_jpegsize_);

                    tcam_stream_statistics stats = {};
                    stats.frame_count = frames_delivered_;
                    stats.frames_dropped = frames_dropped_;

                    auto since_epoch = std::chrono::system_clock::now().time_since_epoch();

                    stats.capture_time_ns =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch).count();

                    buffer->set_valid_data_length(current_jpegsize_);
                    buffer->set_statistics(stats);

                    if (auto sink_ptr = listener_.lock())
                    {
                        ++frames_delivered_;
                        sink_ptr->push_image(buffer);
                    }
                    else
                    {
                        ++frames_dropped_;
                        requeue_buffer(buffer);
                        SPDLOG_ERROR("ImageSink expired. Unable to deliver images.");
                    }
                }
                current_jpegbuf_index_ = 0;
                current_jpegsize_ = 0;

                current_jpegbuf_ptr_ = nullptr;
            }
        }
        else
        {
            processed += transfer->actual_length;
        }
    }

    if (is_stream_on_)
    {
        //submit the next transfer
        int ret = libusb_submit_transfer(transfer);
        if (ret == LIBUSB_ERROR_NO_DEVICE)
        {
            lost_device();
        }
    }
    else
    {
        libusb_free_transfer(transfer);
    }
}

void LIBUSB_CALL tcam::AFU050Device::libusb_bulk_callback(struct libusb_transfer* trans)
{
    AFU050Device* self = reinterpret_cast<AFU050Device*>(trans->user_data);
    self->transfer_callback(trans);
}


bool tcam::AFU050Device::start_stream(const std::shared_ptr<IImageBufferSink>& sink)
{
#define USB_ENDPOINT_IN (LIBUSB_ENDPOINT_IN | 2)


    if (is_stream_on_)
    {
        SPDLOG_ERROR("Unable to start stream. Stream is already running.");
        return false;
    }

    SPDLOG_TRACE("Starting stream...");
    is_stream_on_ = true;
    current_jpegbuf_ptr_ = nullptr;
    current_jpegsize_ = 0;
    current_jpegbuf_index_ = 0;

    listener_ = sink;

    frames_delivered_ = 0;
    frames_dropped_ = 0;

    current_jpegbuf_data_.clear();
    current_jpegbuf_data_.resize(JPEGBUF_SIZE);

    for (int cnt = 0; cnt < TRANSFER_COUNT; cnt++)
    {
        uint8_t* buf = (uint8_t*)malloc(LEN_IN_BUFFER);

        struct libusb_transfer* transfer_in = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(transfer_in,
                                  usb_device_->get_handle(),
                                  USB_ENDPOINT_IN,
                                  buf,
                                  LEN_IN_BUFFER,
                                  AFU050Device::libusb_bulk_callback,
                                  this,
                                  0);

        //submit the transfer, all following transfers are initiated from the CB
        int ret = libusb_submit_transfer(transfer_in);
        if (ret < 0)
        {
            if (ret == LIBUSB_ERROR_NO_DEVICE)
            {
                lost_device();
            }

            SPDLOG_DEBUG("ret < 0");
            break;
        }

        transfers_.push_back(transfer_in);
    }

    return true;
}

void tcam::AFU050Device::stop_stream()
{
    is_stream_on_ = false;

    listener_.reset();

    release_buffers();

    transfers_.clear();
}

void AFU050Device::add_int(const std::string& name, const VC_UNIT unit, const unsigned char prop)
{
    if (unit == VC_UNIT_HEADER || prop == 0)
    {
        return;
    }

    //SPDLOG_DEBUG("adding int {} {} {}", name, unit, prop);

    control_definition cd = { unit, prop };

    properties_.push_back(
        std::make_shared<tcam::property::AFU050PropertyIntegerImpl>(name, cd, backend_));
}


void AFU050Device::add_double(const std::string& name,
                              const VC_UNIT unit,
                              const unsigned char prop,
                              double modifier)
{
    if (unit == VC_UNIT_HEADER || prop == 0)
    {
        return;
    }

    SPDLOG_DEBUG("adding double {} {} {}", name, unit, prop);

    control_definition cd = { unit, prop };

    properties_.push_back(
        std::make_shared<tcam::property::AFU050PropertyDoubleImpl>(name, cd, backend_, modifier));
}


void AFU050Device::add_enum(const std::string& name,
                            const VC_UNIT unit,
                            const unsigned char prop,
                            std::map<int, std::string> entries)
{
    if (unit == VC_UNIT_HEADER || prop == 0)
    {
        return;
    }

    SPDLOG_DEBUG("adding enum {} {} {}", name, unit, prop);

    control_definition cd = { unit, prop };

    properties_.push_back(
        std::make_shared<tcam::property::AFU050PropertyEnumImpl>(name, cd, entries, backend_));
}


bool AFU050Device::set_control(int unit, int ctrl, int len, unsigned char* value)
{
    int ret = libusb_control_transfer(usb_device_->get_handle(),
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_RECIPIENT_DEVICE,
                                      UVC_SET_CUR,
                                      ctrl << 8,
                                      unit << 8,
                                      value,
                                      len,
                                      10000);

    if (ret == LIBUSB_ERROR_NO_DEVICE)
    {
        lost_device();
    }

    return (ret == len ? true : false);
}


bool AFU050Device::get_control(int unit,
                               int ctrl,
                               int len,
                               unsigned char* value,
                               enum CONTROL_CMD cmd)
{
    int ret = libusb_control_transfer(usb_device_->get_handle(),
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_RECIPIENT_DEVICE,
                                      cmd,
                                      ctrl << 8,
                                      unit << 8,
                                      value,
                                      len,
                                      10000);

    if (ret == LIBUSB_ERROR_NO_DEVICE)
    {
        lost_device();
    }

    if (len == ret)
    {
        return true;
    }
    return false;
}


int AFU050Device::set_video_format(uint8_t format_index,
                                   uint8_t frame_index,
                                   uint32_t frame_interval)
{
    uint8_t buf[34] = { 0 };

    buf[2] = format_index;
    buf[3] = frame_index;
    buf[4] = frame_interval & 0xff;
    buf[5] = (frame_interval >> 8) & 0xff;
    buf[6] = (frame_interval >> 16) & 0xff;
    buf[7] = (frame_interval >> 24) & 0xff;

    int ret = libusb_control_transfer(usb_device_->get_handle(),
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS
                                          | LIBUSB_RECIPIENT_INTERFACE,
                                      UVC_SET_CUR,
                                      0x2 << 8, // VS_COMMIT
                                      1,
                                      buf,
                                      sizeof(buf),
                                      10000);
    // frame_interval = buf[4] | ((uint32_t)buf[5]<<8) | ((uint32_t)buf[6]<<16) | ((uint32_t)buf[7]<<24);

    if (ret == LIBUSB_ERROR_NO_DEVICE)
    {
        lost_device();
    }

    SPDLOG_DEBUG("set_video_format transfer ended with {}", ret);

    return ret;
}


bool AFU050Device::get_bool_value(enum VC_UNIT unit, unsigned char property, enum CONTROL_CMD cmd)
{
    int value = 0;
    bool ret = get_control(unit, property, 4, (unsigned char*)&value, cmd);
    if (ret)
        SPDLOG_ERROR("get_control returned with: {}", ret);
    return (value ? 1 : 0);
}


bool AFU050Device::set_bool_value(enum VC_UNIT unit, unsigned char property, bool value)
{
    char val = (value ? 1 : 0);
    bool ret = set_control(unit, property, 4, (unsigned char*)&val);
    if (!ret)
        SPDLOG_ERROR("set_control returned with: {}", ret);
    return ret;
}


int AFU050Device::get_int_value(const enum VC_UNIT unit,
                                const unsigned char property,
                                const enum CONTROL_CMD cmd)
{
    int value = 0;
    bool ret = get_control(unit, property, 4, (unsigned char*)&value, cmd);
    if (!ret)
        SPDLOG_ERROR("get_control returned with: {}", ret);
    return value;
}


bool AFU050Device::set_int_value(enum VC_UNIT unit, unsigned char property, int value)
{
    bool ret = set_control(unit, property, 4, (unsigned char*)&value);
    if (!ret)
        SPDLOG_ERROR("set_control returned with: {}", ret);
    return ret;
}
