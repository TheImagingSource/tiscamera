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

#include "AFU420Device.h"

#include "../logging.h"
#include "../public_utils.h"
#include "../utils.h"
#include "AFU420DeviceBackend.h"
#include "UsbHandler.h"
#include "UsbSession.h"

#include <algorithm>
#include <cstring>
#include <dutils_img/fcc_to_string.h>
#include <dutils_img/image_fourcc_func.h>
#include <fstream>
#include <ostream>
#include <unistd.h>

using namespace tcam;

tcam::AFU420Device::AFU420Device(const DeviceInfo& info)
{
    device = info;


    usb_device_ = UsbHandler::get_instance().open_device_(info.get_serial());

    if (usb_device_ == nullptr)
    {
        SPDLOG_ERROR("Failed to open device.");
    }

    if (!usb_device_->open_interface(0))
    {
        SPDLOG_ERROR("Failed to open camera interface - {}. \n"
                     "Please check device permissions!",
                     0);
    }

    // ensure defined state
    usb_device_->halt_endpoint(USB_EP_BULK_VIDEO);

    // read_firmware_version();

    // properties rely on this information
    check_for_optics();

    m_backend = std::make_shared<tcam::property::AFU420DeviceBackend>(this);

    // set_hdr(16);
    set_ois_pos(0, 0);
    set_ois_mode(6);

    create_properties();
    create_formats();

    query_active_format();
}


tcam::AFU420Device::~AFU420Device()
{
    stop_stream();

    transfer_items.clear();
}

void AFU420Device::query_active_format()
{
    struct tcam_video_format format = {};

    uint16_t bpp = 0;
    int ret = control_read(bpp, BASIC_USB_TO_PC_GET_BIT_DEPTH, 1);
    if (ret < 0)
    {
        SPDLOG_ERROR("Could not query bit depth.");
        return;
    }

    if (bpp == 8)
    {
        format.fourcc = FOURCC_GBRG8;
    }
    else if (bpp == 12)
    {
        format.fourcc = FOURCC_GBRG12_MIPI_PACKED;
    }
    else
    {
        SPDLOG_ERROR("Received bogus bit depth of '{}'", bpp);
        //return;
    }

    sResolutionConf conf = {};
    ret = read_resolution_config_from_device(conf);
    if (ret <= 0)
    {
        SPDLOG_ERROR("Could not read resolution config. LibUsb returned: {}", ret);
        return;
    }

    //print_resolution_conf(conf);

    format.width = conf.x_output_size;
    format.height = conf.y_output_size;

    active_resolution_conf_ = conf;

    active_video_format = VideoFormat(format);

    SPDLOG_DEBUG("Active format is: {}", active_video_format.to_string().c_str());
}


int AFU420Device::read_resolution_config_from_device(sResolutionConf& conf)
{
    std::vector<uint8_t> tmp(sizeof(sResolutionConf), 0);

    int ret = control_read(tmp, BASIC_USB_TO_PC_RES_FPS, 0, 0);
    if (ret <= 0)
    {
        SPDLOG_ERROR("Could not read resolution config from device. LibUsb returned: {}", ret);
        return ret;
    }

    conf = deserialize_resolution_config(tmp);

    return ret;
}


std::vector<uint8_t> AFU420Device::serialize_resolution_config(
    const struct AFU420Device::sResolutionConf& cfg)
{
    std::vector<uint8_t> rval(sizeof(struct AFU420Device::sResolutionConf), 0);

    // serializes the resolution conf struct into a block of memory

    unsigned int i = 0;

    rval[i++] = cfg.x_addr_start >> 8;
    rval[i++] = cfg.x_addr_start & 0xFF;
    rval[i++] = cfg.y_addr_start >> 8;
    rval[i++] = cfg.y_addr_start & 0xFF;
    rval[i++] = cfg.x_addr_end >> 8;
    rval[i++] = cfg.x_addr_end & 0xFF;
    rval[i++] = cfg.y_addr_end >> 8;
    rval[i++] = cfg.y_addr_end & 0xFF;

    rval[i++] = cfg.x_output_size >> 8;
    rval[i++] = cfg.x_output_size & 0xFF;
    rval[i++] = cfg.y_output_size >> 8;
    rval[i++] = cfg.y_output_size & 0xFF;

    rval[i++] = cfg.digital_crop_x_offset >> 8;
    rval[i++] = cfg.digital_crop_x_offset & 0xFF;
    rval[i++] = cfg.digital_crop_y_offset >> 8;
    rval[i++] = cfg.digital_crop_y_offset & 0xFF;
    rval[i++] = cfg.digital_crop_image_width >> 8;
    rval[i++] = cfg.digital_crop_image_width & 0xFF;
    rval[i++] = cfg.digital_crop_image_height >> 8;
    rval[i++] = cfg.digital_crop_image_height & 0xFF;

    rval[i++] = cfg.hor_binning;
    rval[i++] = cfg.ver_binning;

    rval[i++] = cfg.defaultFramerate >> 8;
    rval[i++] = cfg.defaultFramerate & 0xFF;

    return rval;
}


AFU420Device::sResolutionConf AFU420Device::deserialize_resolution_config(
    std::vector<uint8_t> serialized_data) noexcept(true)
{
    struct AFU420Device::sResolutionConf sResConf = {};

    int i = 0;

    // de-serializes a memory block to generate the resolution struct

    //auto u16_be_to_le = [pucSerialData]( int byte_offset ) {
    //    return uint16_t(pucSerialData[byte_offset]) << 8 | pucSerialData[byte_offset + 1];
    //};
    //sResConf.x_addr_start = u16_be_to_le( offsetof( sResolutionConf, x_addr_start ) );

    sResConf.x_addr_start = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.x_addr_start |= (unsigned short)serialized_data[i++];
    sResConf.y_addr_start = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.y_addr_start |= (unsigned short)serialized_data[i++];
    sResConf.x_addr_end = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.x_addr_end |= (unsigned short)serialized_data[i++];
    sResConf.y_addr_end = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.y_addr_end |= (unsigned short)serialized_data[i++];

    sResConf.x_output_size = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.x_output_size |= (unsigned short)serialized_data[i++];
    sResConf.y_output_size = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.y_output_size |= (unsigned short)serialized_data[i++];

    sResConf.digital_crop_x_offset = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.digital_crop_x_offset |= (unsigned short)serialized_data[i++];
    sResConf.digital_crop_y_offset = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.digital_crop_y_offset |= (unsigned short)serialized_data[i++];
    sResConf.digital_crop_image_width = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.digital_crop_image_width |= (unsigned short)serialized_data[i++];
    sResConf.digital_crop_image_height = ((unsigned short)serialized_data[i++]) << 8;
    sResConf.digital_crop_image_height |= (unsigned short)serialized_data[i++];

    sResConf.hor_binning = serialized_data[i++];
    sResConf.ver_binning = serialized_data[i++];

    sResConf.defaultFramerate = ((unsigned int)serialized_data[i++]) << 8;
    sResConf.defaultFramerate |= ((unsigned int)serialized_data[i++]);
    return sResConf;
}


struct AFU420Device::sResolutionConf AFU420Device::CreateResolutionConf(
    const tcam_image_size start,
    const tcam_image_size stream_dim,
    tcam_image_size binning)
{

    // SPDLOG_DEBUG("Creating resolutionconf with input:\nstart: {}x{}\nstream_dim {}x{}\nbinning {}{}",
    //            start.width, start.height,
    //            stream_dim.width, stream_dim.height,
    //            binning.width, binning.height);

    sResolutionConf res_conf = {};

    // change possible binning.v == 1 to = 0, because the firmware likes it
    if (binning.width == 1)
    {
        binning.width = 0;
    }
    if (binning.height == 1)
    {
        binning.height = 0;
    }

    // check if the binning factor is valid
    if ((binning.width != 0) && (binning.height != 0) && (binning.width != 2)
        && (binning.height != 2) && (binning.width != 4) && (binning.height != 4)
        && (binning.width != 8) && (binning.height != 8))
    {
        SPDLOG_ERROR("Invalid binning factor for videoformat.");
        return res_conf;
    }

    // this contains the actual dimensions in pixels on the sensor
    tcam_image_size res_on_sensor_dim = stream_dim;

    // adjust dim to reference the sensor dimensions this format uses
    if (binning.width > 0)
    {
        res_on_sensor_dim.width = stream_dim.width * binning.width;
    }
    if (binning.height > 0)
    {
        res_on_sensor_dim.height = stream_dim.height * binning.height;
    }

    tcam_image_size roi_start = transform_roi_start(start, res_on_sensor_dim);

    int bin_horz = binning.width;
    int bin_vert = binning.height;

    // check if the ROI is inside the visible area
    if ((roi_start.width > (m_uPixelMaxX - m_uPixelMinX))
        || (roi_start.height > (m_uPixelMaxY - m_uPixelMinY)))
    {
        SPDLOG_ERROR("Invalid roi start. {}x{}", roi_start.width, roi_start.height);
        return res_conf;
    }

    // both start values have to be a multiple of 4 (startX maybe %12?)
    if (roi_start.width % 4 || roi_start.height % 4)
    {
        SPDLOG_ERROR("Invalid roi start. {} {}", roi_start.width, roi_start.height);
        return res_conf;
    }

    if ((res_on_sensor_dim.width > m_uPixelMaxX) || (res_on_sensor_dim.height > m_uPixelMaxY))
    {
        SPDLOG_ERROR("Invalid dimensions (too large) for videoformat.");
        return res_conf;
    }

    if (res_on_sensor_dim.width % 4 || res_on_sensor_dim.width % 12 || res_on_sensor_dim.height % 4)
    {
        SPDLOG_ERROR("Invalid dimensions (step) for videoformat.");
        return res_conf;
    }

    // set new resolution
    int m_uPixelX = res_on_sensor_dim.width;
    int m_uPixelY = res_on_sensor_dim.height;
    int m_uPixelNoBinningX = res_on_sensor_dim.width;
    int m_uPixelNoBinningY = res_on_sensor_dim.height;

    // horizontal binning?
    if (bin_horz)
    {
        // use binning factor on horizontal pixel
        m_uPixelX = m_uPixelNoBinningX / bin_horz;
    }
    else
    {
        m_uPixelX = m_uPixelNoBinningX;
    }

    // vertical binning?
    if (bin_vert)
    {
        // use binning factor on vertical pixel
        m_uPixelY = m_uPixelNoBinningY / bin_vert;
    }
    else
    {
        m_uPixelY = m_uPixelNoBinningY;
    }

    // copy binning factor
    res_conf.hor_binning = (unsigned char)bin_horz;
    res_conf.ver_binning = (unsigned char)bin_vert;

    // add 4 pixel offset to the start
    res_conf.x_addr_start = (unsigned short)(4 + roi_start.width);
    res_conf.y_addr_start = (unsigned short)(4 + roi_start.height);

    // fill up the end pixel position (no binning takes effect here)
    res_conf.x_addr_end = (unsigned short)(res_conf.x_addr_start + m_uPixelNoBinningX - 1);
    res_conf.y_addr_end = (unsigned short)(res_conf.y_addr_start + m_uPixelNoBinningY - 1);

    // get the output size (effected by binning factor)
    res_conf.x_output_size = (unsigned short)(m_uPixelX);
    res_conf.y_output_size = (unsigned short)(m_uPixelY);

    // select the crop for the image
    res_conf.digital_crop_image_width = (unsigned short)m_uPixelX;
    res_conf.digital_crop_image_height = (unsigned short)m_uPixelY;

    // restart the previous stream if the end address pixel are not correct
    if ((res_conf.x_addr_end > (unsigned short)m_uPixelMaxX)
        || (res_conf.y_addr_end > (unsigned short)m_uPixelMaxY))
    {
        SPDLOG_ERROR(
            "ResolutionConfig could not be created. end pixel address does not make sense.");
        SPDLOG_ERROR("{} > {}   {} > {}",
                     res_conf.x_addr_end,
                     (unsigned short)m_uPixelMaxX,
                     res_conf.y_addr_end,
                     (unsigned short)m_uPixelMaxY);
        return sResolutionConf {};
    }
    return res_conf;
}


void AFU420Device::read_firmware_version()
{
    int a = 0, b = 0, c = 0, d = 0;

    uint64_t ullVersion = 0;

    int hr = control_read(ullVersion, BASIC_USB_TO_PC_VERSION_FIRMWARE);

    if (hr > 0)
    {
        a = (unsigned int)((ullVersion / 1000000000ULL));
        ullVersion -= ((unsigned long long)a * 1000000000ULL);

        b = (unsigned int)((ullVersion / 1000000ULL));
        ullVersion -= ((unsigned long long)b * 1000000ULL);

        c = (unsigned int)((ullVersion / 1000ULL));
        ullVersion -= ((unsigned long long)c * 1000ULL);

        d = (unsigned int)((ullVersion / 1ULL));
    }
    else
    {
        SPDLOG_ERROR("Could not read firmware version");
    }

    SPDLOG_INFO("Firmware version is {}.{}.{}.{} \n", a, b, c, d);
}


tcam_image_size AFU420Device::transform_roi_start(tcam_image_size pos,
                                                  tcam_image_size pixels_on_sensor_dim)
{
    return { max_sensor_dim_.width - pos.width - pixels_on_sensor_dim.width,
             max_sensor_dim_.height - pos.height - pixels_on_sensor_dim.height };
}


DeviceInfo tcam::AFU420Device::get_device_description() const
{
    return device;
}


void AFU420Device::create_formats()
{
    std::vector<stream_fmt_data> fmt_list {
        stream_fmt_data { 0, 8, FOURCC_GBRG8, min_sensor_dim_, max_sensor_dim_, step, 2.0, 30.0 },
        stream_fmt_data { 2,
                          12,
                          FOURCC_GBRG12_MIPI_PACKED,
                          min_sensor_dim_,
                          max_sensor_dim_by12,
                          step,
                          2.0,
                          30.0 },
    };

    // assign to member for frame rate checks
    stream_format_list_ = fmt_list;

    for (auto& fmt : fmt_list)
    {
        struct tcam_video_format_description desc = {};

        desc.fourcc = fmt.fmt;
        memcpy(desc.description, img::fcc_to_string(desc.fourcc).c_str(), sizeof(desc.description));

        std::vector<struct framerate_mapping> rf;

        auto add_res =
            [&rf, this](stream_fmt_data& _fmt, tcam_image_size& size, const image_scaling& scale)
        {
            struct tcam_resolution_description res = {};
            res.type = TCAM_RESOLUTION_TYPE_FIXED;
            res.min_size.width = size.width;
            res.min_size.height = size.height;
            res.max_size.width = size.width;
            res.max_size.height = size.height;

            res.scaling = scale;

            double fps_min = 0;
            double fps_max = 0;

            this->get_frame_rate_range(_fmt.fmt, 1, size, fps_min, fps_max);

            std::vector<double> f = create_steps_for_range(fps_min, fps_max);

            framerate_mapping r = { res, f };
            rf.push_back(r);
        };

        // max resolution only with binning 1x1
        add_res(fmt, fmt.dim_max, { 1, 1, 1, 1 });

        for (auto& f : get_standard_resolutions(fmt.dim_min, fmt.dim_max))
        {
            if (f.width % 4 || f.width % 12 || f.height % 4)
            {
                continue;
            }

            for (const auto& s : m_available_scaling)
            {
                if (s.legal_resolution(fmt.dim_max, f))
                {
                    add_res(fmt, f, s);
                }
            }
        }

        // smallest resolution works with all scalings
        for (const auto& s : m_available_scaling) { add_res(fmt, fmt.dim_min, s); }

        VideoFormatDescription format(nullptr, desc, rf);
        available_videoformats.push_back(format);
    }
    // set list with actual framerates
    stream_format_list_ = fmt_list;
}


tcam_image_size tcam::AFU420Device::calculate_auto_offset(uint32_t fourcc,
                                                          const tcam_image_size& size,
                                                          const tcam_image_size& binning) const
{
    tcam_image_size max = {};

    int bpp = img::get_bits_per_pixel(fourcc);
    if (bpp == 8)
    {
        max = max_sensor_dim_;
    }
    else
    {
        max = max_sensor_dim_by12;
    }

    return calculate_auto_center(
        max,
        step,
        size,
        { static_cast<int>(binning.width), static_cast<int>(binning.height), 1, 1 });
}


AFU420Device::sResolutionConf tcam::AFU420Device::videoformat_to_resolution_conf(
    const VideoFormat& format)
{

    tcam_image_size binning = { static_cast<unsigned int>(format.get_scaling().binning_h),
                                static_cast<unsigned int>(format.get_scaling().binning_v) };

    tcam_image_size offset = { 0, 0 };

    if (m_offset_auto)
    {
        if (format.get_size() == max_sensor_dim_ || format.get_size() == max_sensor_dim_by12)
        {
            offset = { 0, 0 };
        }
        else
        {
            offset = calculate_auto_offset(format.get_fourcc(), format.get_size(), binning);
        }
    }
    else
    {
        offset = m_offset;
    }

    auto conf = CreateResolutionConf(offset, format.get_size(), binning);

    return conf;
}


bool tcam::AFU420Device::set_video_format(const VideoFormat& format)
{
    if (is_stream_on_)
    {
        SPDLOG_ERROR("Unable to set format. Stream is running.");
        return false;
    }

    SPDLOG_INFO("Attempting to set format to: '{}'", format.to_string().c_str());

    int ret = setup_bit_depth(img::get_bits_per_pixel(format.get_fourcc()));

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not set bit depth. Aborting. {}", ret);
        return false;
    }
    else
    {
        SPDLOG_DEBUG("Set bit depth to {}", image_bit_depth_);
    }

    auto conf = videoformat_to_resolution_conf(format);

    ret = set_resolution_config(conf, resolution_config_mode::set);

    if (ret <= 0)
    {
        SPDLOG_ERROR("Could not set bit depth. Aborting.");
        return false;
    }

    if (!set_framerate(format.get_framerate()))
    {
        return false;
    }

    active_video_format = format;

    SPDLOG_INFO("Set format to: {}", active_video_format.to_string().c_str());

    return true;
}

VideoFormat tcam::AFU420Device::get_active_video_format() const
{
    return active_video_format;
}


std::vector<VideoFormatDescription> tcam::AFU420Device::get_available_video_formats()
{
    return available_videoformats;
}


bool tcam::AFU420Device::set_framerate(double framerate)
{

    // set the frame rate in the camera (frame rate will be sent as 7.7FPS = 770)
    unsigned short val = ((unsigned short)(framerate * 100.0));

    SPDLOG_DEBUG("Attempting to set framerate value {}", val);

    int ret = control_write(BASIC_PC_TO_USB_FPS, val, 1);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not set framerate. LibUsb returned: {}", ret);
        return false;
    }

    return true;
}


double tcam::AFU420Device::get_framerate()
{
    return active_video_format.get_framerate();
}


bool tcam::AFU420Device::initialize_buffers(std::shared_ptr<BufferPool> pool)
{
    auto buffs = pool->get_buffer();
    SPDLOG_TRACE("Received {} buffer from external allocator.", buffs.size());

    buffer_list_.reserve(buffs.size());

    for (auto& b : buffs) { buffer_list_.push_back({ b.lock(), true }); }
    return true;
}

bool tcam::AFU420Device::release_buffers()
{
    std::scoped_lock lck { buffers_mutex_ };

    buffer_list_.clear();
    current_buffer_ = nullptr;

    return true;
}

void AFU420Device::requeue_buffer(const std::shared_ptr<ImageBuffer>& buffer)
{
    buffer->set_valid_data_length(0);

    std::scoped_lock lck { buffers_mutex_ };
    for (auto& b : buffer_list_)
    {
        if (buffer == b.buffer)
        {
            b.is_queued = true;
            break;
        }
    }
}

void AFU420Device::push_buffer(std::shared_ptr<tcam::ImageBuffer>&& cur_buf)
{
    assert(cur_buf != nullptr);

    if (drop_incomplete_frames_ && (usbbulk_image_size_ - cur_buf->get_valid_data_length() != 0))
    {
        SPDLOG_TRACE("Image buffer does not contain enough data. Dropping frame...");

        frames_dropped_++;
        requeue_buffer(cur_buf);
        return;
    }

    tcam_stream_statistics stats = {};
    stats.frame_count = frames_delivered_;
    stats.frames_dropped = frames_dropped_;

    auto since_epoch = std::chrono::system_clock::now().time_since_epoch();
    stats.capture_time_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch).count();

    cur_buf->set_statistics(stats);

    frames_delivered_++;

    if (!deliver_thread_.push(std::move(cur_buf)))
    {
        requeue_buffer(cur_buf);
    }
}


static uint16_t bytes_to_uint16(uint8_t lo, uint8_t hi)
{
    return uint16_t(lo) | (uint16_t(hi) << 8);
}


struct AFU420Device::header_res AFU420Device::check_and_eat_img_header(unsigned char* data,
                                                                       size_t data_size)
{
    struct header_res res = {};
    res.frame_id = -1;
    res.buffer = data;
    res.size = data_size;
    size_t actual_header_size = get_packet_header_size();

    if (data_size < actual_header_size)
    {
        return res;
    }

    const int bpp = get_stream_bitdepth();
    auto get_hdr_field_at = [bpp, data](int offset)
    {
        return data[offset * bpp / 8];
    };

    if (image_bit_depth_ == 12)
    {
        uint8_t hdr_12bit[4] = { 0x0a, 0xaa, 0x55, 0x00 };
        int d = memcmp(data, hdr_12bit, 4);
        if (d != 0)
        {
            return res;
        }
    }
    else // this should work for 8 and 10 bit
    {
        uint8_t hdr_8bit[4] = { 0x0a, 0xaa, 0x00, 0xa5 };
        int d = memcmp(data, hdr_8bit, 4);
        if (d != 0)
        {
            return res;
        }
    }

    uint32_t uWidth =
        bytes_to_uint16(get_hdr_field_at(0x4E), get_hdr_field_at(0x4C)); // note that we skip 0x4D
    uint32_t uHeight = bytes_to_uint16(get_hdr_field_at(0x5E), get_hdr_field_at(0x5C));

    tcam_image_size dim = get_stream_dim();
    if (uWidth != dim.width || uHeight != dim.height)
    {
        SPDLOG_ERROR("Dimensions do not fit.");
        return res;
    }
    res.frame_id = get_hdr_field_at(0x10);

    res.buffer = data + actual_header_size;
    res.size = data_size - actual_header_size;

    //int hdr_mode_on = get_hdr_field_at( uWidth + 0x26 );

    return res;
}


void tcam::AFU420Device::transfer_callback(struct libusb_transfer* xfr)
{
    if (!is_stream_on_)
    {
        // do not free transfers
        //libusb_free_transfer(xfr);

        //SPDLOG_DEBUG("stream is off");
        return;
    }

    auto submit_transfer = [](struct libusb_transfer* _xfr)
    {
        if (libusb_submit_transfer(_xfr) < 0)
        {
            SPDLOG_ERROR("error re-submitting URB\n");
        }
    };


    if (xfr->status != LIBUSB_TRANSFER_COMPLETED)
    {
        if (xfr->status == LIBUSB_TRANSFER_CANCELLED)
        {
            return;
        }
        SPDLOG_WARN("transfer status {}", xfr->status);
        submit_transfer(xfr);

        if (lost_countdown_ == 0)
        {
            notify_device_lost();
        }

        lost_countdown_--;
        return;
    }

    auto header = check_and_eat_img_header(xfr->buffer, xfr->length);

    bool is_header = header.frame_id >= 0;
    bool is_trailer = (!is_header) && header.size < (size_t)usbbulk_chunk_size_;

    if (is_header)
    {
        if (current_buffer_)
        {
            push_buffer(std::move(current_buffer_));
        }

        current_buffer_ = get_next_buffer();

        if (current_buffer_ == nullptr)
        {
            SPDLOG_ERROR("No buffer to work with. Dropping image"); // Buffer starvation
            frames_dropped_++;
            submit_transfer(xfr);
            return;
        }

        have_header_ = true;
        transfer_offset_ = 0;
    }

    if (current_buffer_ == nullptr)
    {
        submit_transfer(xfr); // just requeue und wait for the next header to arrive
        return;
    }

    int bytes_available = usbbulk_image_size_ - transfer_offset_;

    int bytes_to_copy = std::min(bytes_available, int(header.size));

    current_buffer_->copy_block(header.buffer, bytes_to_copy, transfer_offset_);

    transfer_offset_ += bytes_to_copy;

    current_buffer_->set_valid_data_length(transfer_offset_);

    bool is_complete_image = transfer_offset_ >= usbbulk_image_size_;
    if (is_complete_image || is_trailer)
    {
        if (current_buffer_)
        {
            push_buffer(std::move(current_buffer_));
            current_buffer_.reset();
        }
        have_header_ = false;
        transfer_offset_ = 0;
    }
    lost_countdown_ = 20;
    submit_transfer(xfr);
}


std::shared_ptr<tcam::ImageBuffer> tcam::AFU420Device::get_next_buffer()
{
    if (buffer_list_.empty())
    {
        SPDLOG_ERROR("No buffers to work with.");
        return nullptr;
    }

    for (auto& b : buffer_list_)
    {
        if (b.is_queued)
        {
            //SPDLOG_ERROR("returning buffer %p", b.buffer->get_data());
            b.is_queued = false;
            return b.buffer;
        }
    }

    SPDLOG_ERROR("No free buffers available! {}", buffer_list_.size());
    return nullptr;
}


void LIBUSB_CALL tcam::AFU420Device::libusb_bulk_callback(struct libusb_transfer* trans)
{
    AFU420Device* self = static_cast<AFU420Device*>(trans->user_data);
    self->transfer_callback(trans);
}

bool tcam::AFU420Device::start_stream(const std::shared_ptr<IImageBufferSink>& sink)
{
    const int USB2_STACKUP_SIZE = 512;
    const int USB3_STACKUP_SIZE = 32;


    // reset statistics
    frames_delivered_ = 0;
    frames_dropped_ = 0;
    transfer_offset_ = 0;
    have_header_ = false;

    static const int num_transfers = 12;
    int chunk_size = 0;

    if (usb_device_->is_superspeed())
    {
        chunk_size = usb_device_->get_max_packet_size(USB_EP_BULK_VIDEO) * USB3_STACKUP_SIZE;
    }
    else
    {
        chunk_size = 15 * 1024 * USB2_STACKUP_SIZE;
    }

    transfer_items.clear();
    transfer_items.reserve(num_transfers);

    size_t buffer_size = 1024 * 1024;

    usbbulk_chunk_size_ = chunk_size;

    usbbulk_image_size_ = active_video_format.get_required_buffer_size();

    for (int i = 0; i < num_transfers; ++i)
    {
        transfer_items.push_back({});
        transfer_items.at(i).transfer = libusb_alloc_transfer(0);
        transfer_items.at(i).buffer.reserve(buffer_size);

        struct libusb_transfer* xfr = (libusb_transfer*)transfer_items.at(i).transfer;

        libusb_fill_bulk_transfer(xfr,
                                  usb_device_->get_handle(),
                                  LIBUSB_ENDPOINT_IN | USB_EP_BULK_VIDEO,
                                  (uint8_t*)transfer_items.at(i).buffer.data(),
                                  transfer_items.at(i).buffer.capacity(),
                                  AFU420Device::libusb_bulk_callback,
                                  this,
                                  0);

        libusb_submit_transfer(xfr);
    }

    listener_ = sink;

    deliver_thread_.start(sink);

    unsigned char val = 0;
    int ret = control_write(BASIC_PC_TO_USB_START_STREAM, val);

    if (ret < 0)
    {
        SPDLOG_ERROR("Stream could not be started. Aborting");

        listener_.reset();

        return false;
    }

    have_header_ = false;
    is_stream_on_ = true;

    SPDLOG_INFO("Stream started");

    return true;
}


void tcam::AFU420Device::stop_stream()
{
    SPDLOG_DEBUG("stop_stream called");

    is_stream_on_ = false;

    deliver_thread_.stop();

    for (auto& item : transfer_items) { libusb_cancel_transfer((libusb_transfer*)item.transfer); }

    usb_device_->halt_endpoint(USB_EP_BULK_VIDEO);

    listener_.reset();

    release_buffers();
}


int AFU420Device::set_resolution_config(sResolutionConf conf, resolution_config_mode mode)
{
    auto serialized_conf = serialize_resolution_config(conf);

    uint16_t test_mode = mode == resolution_config_mode::test ? 1 : 0;

    int hr = control_write(ADVANCED_PC_TO_USB_RES_FPS, test_mode, 0, serialized_conf);

    uint32_t exp_max = 0, exp_min = 0;
    control_read(exp_min, BASIC_USB_TO_PC_MIN_EXP, test_mode, 0);
    control_read(exp_max, BASIC_USB_TO_PC_MAX_EXP, test_mode, 0);

    return hr;
}


int AFU420Device::setup_bit_depth(int bpp)
{
    if (bpp != 8 && bpp != 10 && bpp != 12)
    {
        return EINVAL;
    }
    int hr = control_write(BASIC_PC_TO_USB_SET_BIT_DEPTH, (uint16_t)bpp);

    if (hr < 0)
    {
        SPDLOG_ERROR("Failed to set a bit depth. This is most likely a too old firmware. {} {}",
                     hr,
                     libusb_strerror((libusb_error)hr));

        return hr;
    }

    image_bit_depth_ = bpp;
    return hr;
}


int AFU420Device::get_fps_max(double& max,
                              tcam_image_size pos,
                              tcam_image_size dim,
                              tcam_image_size binning,
                              int src_bpp)
{
    int hr = setup_bit_depth(src_bpp);
    if (hr < 0)
    {
        SPDLOG_ERROR("could not set bit depth");
    }

    auto conf = CreateResolutionConf(pos, dim, binning);

    if (conf.x_output_size == 0)
    {
        SPDLOG_ERROR("resolution size has output size 0");
        return EINVAL;
    }

    hr = set_resolution_config(conf, resolution_config_mode::test);

    if (hr <= 0)
    {
        SPDLOG_ERROR("Could not set resolution config ({})", hr);
        return hr;
    }

    const uint16_t conf_test_mode = 1;
    uint16_t ushMaxFPS = 0;

    hr = control_read(ushMaxFPS, BASIC_USB_TO_PC_MAX_FPS, conf_test_mode, 0);

    if (hr < 0)
    {
        return hr;
    }

    max = ((double)ushMaxFPS) / 100.0;

    return 0;
}


int AFU420Device::get_frame_rate_range(uint32_t strm_fmt_id,
                                       int scaling_factor_id,
                                       tcam_image_size dim,
                                       double& min_fps,
                                       double& max_fps)
{
    const auto& stream_fmt_list = get_stream_format_descs();

    auto f =
        std::find_if(stream_fmt_list.begin(),
                     stream_fmt_list.end(),
                     [strm_fmt_id](const stream_fmt_data& e) { return e.id == (int)strm_fmt_id; });

    if (f == stream_fmt_list.end())
    {
        SPDLOG_DEBUG("???");
        // return EINVAL;
    }

    uint32_t binning = scaling_factor_id;
    if (binning <= 1)
    {
        binning = 0;
    }

    min_fps = 2.f;
    max_fps = 30.f;

    int hr = get_fps_max(max_fps, { 0, 0 }, dim, { binning, binning }, f->src_bpp);

    if (hr == 0)
    {
        frame_rate_cache_.push_back({ strm_fmt_id, scaling_factor_id, dim, min_fps, max_fps });
        if (frame_rate_cache_.size() > 128)
        {
            frame_rate_cache_.erase(frame_rate_cache_.begin(), frame_rate_cache_.begin() + 64);
        }
    }

    return hr;
}


void AFU420Device::check_for_optics()
{
    uint8_t has_optics = 1;

    int ret = control_read(has_optics, ADVANCED_USB_TO_PC_HAS_OPTICS);
    if (ret < 0)
    {
        has_optics_ = true;
    }
    else
    {
        has_optics_ = has_optics != 0;
    }
}


int AFU420Device::control_write(unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex)
{
    uint8_t data = 0;
    return usb_device_->control_transfer(
        HOST_TO_DEVICE, (uint8_t)ucRequest, ushValue, ushIndex, data, 0);
}


int AFU420Device::control_write(unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex,
                                uint8_t data)
{
    return usb_device_->control_transfer(
        HOST_TO_DEVICE, (uint8_t)ucRequest, ushValue, ushIndex, data);
}


int AFU420Device::control_write(unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex,
                                uint16_t data)
{
    return usb_device_->control_transfer(
        HOST_TO_DEVICE, (uint8_t)ucRequest, ushValue, ushIndex, data);
}


int AFU420Device::control_write(unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex,
                                uint32_t data)
{

    return usb_device_->control_transfer(
        HOST_TO_DEVICE, (uint8_t)ucRequest, ushValue, ushIndex, data);
}


int AFU420Device::control_write(unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex,
                                std::vector<unsigned char>& data)
{
    unsigned int timeout = 500;
    int rval = libusb_control_transfer(usb_device_->get_handle(),
                                       LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR
                                           | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushValue,
                                       ushIndex,
                                       data.data(),
                                       data.size(),
                                       timeout);
    return rval;
}


template<class T>
int AFU420Device::read_reg(T& value, unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex)
{

    unsigned int timeout = 500;
    int rval = libusb_control_transfer(usb_device_->get_handle(),
                                       LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR
                                           | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushValue,
                                       ushIndex,
                                       (unsigned char*)&value,
                                       sizeof(T),
                                       timeout);
    return rval;
}


int AFU420Device::control_read(uint8_t& value,
                               unsigned char ucRequest,
                               uint16_t ushValue,
                               uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read(uint16_t& value,
                               unsigned char ucRequest,
                               uint16_t ushValue,
                               uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read(uint32_t& value,
                               unsigned char ucRequest,
                               uint16_t ushValue,
                               uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read(uint64_t& value,
                               unsigned char ucRequest,
                               uint16_t ushValue,
                               uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read(std::vector<uint8_t>& value,
                               unsigned char ucRequest,
                               uint16_t ushValue,
                               uint16_t ushIndex)
{
    unsigned int timeout = 500;

    int rval = libusb_control_transfer(usb_device_->get_handle(),
                                       LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR
                                           | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushIndex,
                                       ushValue,
                                       (unsigned char*)value.data(),
                                       value.size(),
                                       timeout);
    return rval;
}
