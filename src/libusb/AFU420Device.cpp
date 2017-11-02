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

#include "UsbHandler.h"
#include "UsbSession.h"

#include "logging.h"
#include "standard_properties.h"
#include "format.h"
//#include "afu050_definitions.h"
#include "utils.h"
#include "public_utils.h"
#include "image_transform_base.h"

#include <algorithm>
#include <cstring>

#include <fstream>
#include <ostream>
#include <unistd.h>

using namespace tcam;



tcam::AFU420Device::AFU420Device (const DeviceInfo& info,
                                  std::shared_ptr<UsbSession> s)
    :session(s), is_stream_on(false), stop_all(false)
{
    device = info;


    device_handle = UsbHandler::get_instance().open_device(info.get_serial());

    if (!device_handle)
    {
        tcam_log(TCAM_LOG_ERROR, "Failed to open device.");
    }

    // int ret = libusb_get_configuration(device_handle, &end_point);

    // if (ret != 0)
    // {
    //     tcam_warning("Could not retrieve configuration.");
    // }


    int ret = libusb_claim_interface(device_handle, 0);
    if (ret < 0)
    {
        tcam_error("Failed to open camera interface - %d (%d). \n"
                   "Please check device permissions!", 0, ret);
        //libusb_close(device_handle);
    }

    if (libusb_clear_halt(device_handle, USB_EP_BULK_VIDEO) != 0)
    {
        tcam_error("Could not halt endpoint");
    }

    read_firmware_version();

    property_handler = std::make_shared<AFU420PropertyHandler>(this);

    create_properties();
    create_formats();

    query_active_format();

    //set_exposure(500000);
}

tcam::AFU420Device::~AFU420Device ()
{
    stop_stream();

    if (work_thread.joinable())
    {
        work_thread.join();
    }

    libusb_release_interface(device_handle, 0);

    libusb_close(device_handle);
}

void print_resolution_conf (AFU420Device::sResolutionConf conf)
{
    tcam_info("\nxaddrstart: %d\n \
yaddrstart: %d\n \
xaddrstop: %d\n \
yaddrstop: %d\n \
xoutputsize: %d\n \
youtputsize: %d\n \
crop x offset: %d\n \
crop y offset: %d\n \
crop img width: %d\n \
crop img height: %d\n \
hor binning: %d\n \
ver binning: %d\n \
framerate: %d\n",
              conf.x_addr_start,
              conf.y_addr_start,
              conf.x_addr_end,
              conf.y_addr_end,
              conf.x_output_size,
              conf.y_output_size,
              conf.digital_crop_x_offset,
              conf.digital_crop_y_offset,
              conf.digital_crop_image_width,
              conf.digital_crop_image_height,
              conf.hor_binning,
              conf.ver_binning,
              conf.defaultFramerate);
}


void AFU420Device::query_active_format ()
{
    struct tcam_video_format format = {};

    uint16_t bpp = 0;
    int ret = control_read(bpp, BASIC_USB_TO_PC_GET_BIT_DEPTH, 1);
    if (ret < 0)
    {
        tcam_error("Could not query bit depth.");
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
        tcam_error("Received bogus bit depth of '%d' %d", bpp);
        //return;
    }

    sResolutionConf conf = {};
    ret = read_resolution_config_from_device(conf);
    if (ret <= 0)
    {
        tcam_error("Could not read resolution config. LibUsb returned: %d", ret);
        return;
    }

    //print_resolution_conf(conf);

    format.width = conf.x_output_size;
    format.height = conf.y_output_size;

    active_video_format = VideoFormat(format);

    tcam_debug("Active format is: %s", active_video_format.to_string().c_str());
}


int AFU420Device::read_resolution_config_from_device (sResolutionConf& conf)
{
    std::vector<uint8_t> tmp(sizeof(sResolutionConf), 0);

    int ret = control_read(tmp, BASIC_USB_TO_PC_RES_FPS, 0, 0);
    if (ret <= 0)
    {
        tcam_error("Could not read resolution config from device. LibUsb returned: %d", ret);
        return ret;
    }

    conf = deserialize_resolution_config(tmp);

    return ret;
}


std::vector<uint8_t> AFU420Device::serialize_resolution_config ( const struct AFU420Device::sResolutionConf& cfg )
{
    std::vector<uint8_t> rval( sizeof( struct AFU420Device::sResolutionConf ), 0 );

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


AFU420Device::sResolutionConf AFU420Device::deserialize_resolution_config (std::vector<uint8_t> serialized_data) noexcept( true )
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



struct AFU420Device::sResolutionConf AFU420Device::CreateResolutionConf (const tcam_image_size start,
                                                                         const tcam_image_size stream_dim,
                                                                         tcam_image_size binning)
{
    sResolutionConf res_conf = {};

    // change possible binning.v == 1 to = 0, because the firmware likes it
    // if( binning.width == 1 )
    // {
    //     binning.width = 0;
    // }
    // if( binning.height == 1 )
    // {
    //     binning.height = 0;
    // }

    // // check if the binning factor is valid
    // if( (binning.width != 0) && (binning.height != 0) &&
    //     (binning.width != 2) && (binning.height != 2) &&
    //     (binning.width != 4) && (binning.height != 4) &&
    //     (binning.width != 8) && (binning.height != 8) )
    // {
    //     tcam_error("Invalid binning factor for videoformat.");
    //     return res_conf;
    // }

    // this contains the actual dimensions in pixels on the sensor
    tcam_image_size res_on_sensor_dim = stream_dim;

    // adjust dim to reference the sensor dimensions this format uses
    if( binning.width > 0 )
        res_on_sensor_dim.width = stream_dim.width * binning.width;
    if( binning.height > 0 )
        res_on_sensor_dim.height = stream_dim.height * binning.height;

    tcam_image_size roi_start = transform_roi_start( start, res_on_sensor_dim );
    if( roi_start.width < 0 || roi_start.height < 0 )
    {
        tcam_error("Invalid roi start.");
        return res_conf;
    }

    int bin_horz = binning.width;
    int bin_vert = binning.height;

    // check if the ROI is inside the visible area
    if( (roi_start.width > (m_uPixelMaxX - m_uPixelMinX)) || (roi_start.height > (m_uPixelMaxY - m_uPixelMinY)) )
    {
        tcam_error("Invalid roi start.");
        return res_conf;
    }

    // both start values have to be a multiple of 4 (startX maybe %12?)
    if( roi_start.width % 4 || roi_start.height % 4 )
    {
        tcam_error("Invalid roi start.");
        return res_conf;
    }

    if( (res_on_sensor_dim.width > m_uPixelMaxX) || (res_on_sensor_dim.height > m_uPixelMaxY) )
    {
        tcam_error("Invalid dimensions (too large) for videoformat.");
        return res_conf;
    }

    if( res_on_sensor_dim.width % 4 || res_on_sensor_dim.width % 12 || res_on_sensor_dim.height % 4 )
    {
        tcam_error("Invalid dimensions (step) for videoformat.");
        return res_conf;
    }

    // set new resolution
    int m_uPixelX = res_on_sensor_dim.width;
    int m_uPixelY = res_on_sensor_dim.height;
    int m_uPixelNoBinningX = res_on_sensor_dim.width;
    int m_uPixelNoBinningY = res_on_sensor_dim.height;

    // horizontal binning?
    if( bin_horz )
    {
        // use binning factor on horizontal pixel
        m_uPixelX = m_uPixelNoBinningX / bin_horz;
    }
    else
    {
        m_uPixelX = m_uPixelNoBinningX;
    }

    // vertical binning?
    if( bin_vert )
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
    if( (res_conf.x_addr_end > (unsigned short)m_uPixelMaxX) || (res_conf.y_addr_end > (unsigned short)m_uPixelMaxY) )
    {
        tcam_error("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUHUHUHUHUHU");
        tcam_error("%d > %d   %d > %d", res_conf.x_addr_end, (unsigned short)m_uPixelMaxX,
                   res_conf.y_addr_end, (unsigned short)m_uPixelMaxY);
        return sResolutionConf{};
    }
    return res_conf;
}


void AFU420Device::read_firmware_version ()
{
    int a = 0, b = 0, c = 0, d = 0;

    uint64_t ullVersion = 0;

    int hr = control_read(ullVersion, BASIC_USB_TO_PC_VERSION_FIRMWARE);

    if(hr > 0)
    {
        a = (unsigned int)((ullVersion / 1000000000ULL));
        ullVersion -= ((unsigned long long) a * 1000000000ULL);

        b = (unsigned int)((ullVersion / 1000000ULL));
        ullVersion -= ((unsigned long long) b * 1000000ULL);

        c = (unsigned int)((ullVersion / 1000ULL));
        ullVersion -= ((unsigned long long) c * 1000ULL);

        d = (unsigned int)((ullVersion / 1ULL));
    }
    else
    {
        tcam_error("Could not read firmware version");
    }
    //firmware_version_ = win32::version_info( (uint16_t)a, (uint16_t)b, (uint16_t)c, (uint16_t)d );

    tcam_info("Firmware version is %d.%d.%d.%d \n", a, b, c, d);
}


tcam_image_size AFU420Device::transform_roi_start (tcam_image_size pos, tcam_image_size pixels_on_sensor_dim)
{
    return {max_sensor_dim_.width - pos.width - pixels_on_sensor_dim.width,
            max_sensor_dim_.height - pos.height - pixels_on_sensor_dim.height};
}


DeviceInfo tcam::AFU420Device::get_device_description () const
{
    return device;
}


void AFU420Device::create_formats ()
{
    tcam_image_size max_sensor_dim_by12 = { 5424, 5360 };
    tcam_image_size step = { 12, 4 };

    std::vector<stream_fmt_data> fmt_list{
        stream_fmt_data{0, 8, FOURCC_GBRG8, FOURCC_RGB32, min_sensor_dim_, max_sensor_dim_, step, 2.0, 30.0},
        stream_fmt_data{1, 8, FOURCC_GBRG8, FOURCC_Y800, min_sensor_dim_, max_sensor_dim_, step, 2.0, 30.0},
        stream_fmt_data{2, 12, FOURCC_GBRG12_MIPI_PACKED, FOURCC_RGB64, min_sensor_dim_, max_sensor_dim_by12, step, 2.0, 30.0},
        stream_fmt_data{3, 12, FOURCC_GBRG12_MIPI_PACKED, FOURCC_Y16, min_sensor_dim_, max_sensor_dim_by12, step, 2.0, 30.0},
    };

    // assign to member for frame rate checks
    stream_format_list_ = fmt_list;

    for (auto& fmt : fmt_list)
    {
        struct tcam_video_format_description desc = {};

        desc.fourcc = fmt.fmt_in;
        memcpy(desc.description, fourcc2description(desc.fourcc), sizeof(desc.description));

        std::vector<struct framerate_mapping> rf;
        auto add_res = [&rf] (stream_fmt_data& fmt, tcam_image_size& size)
            {
                struct tcam_resolution_description res = {};
                res.type = TCAM_RESOLUTION_TYPE_FIXED;
                res.min_size.width = size.width;
                res.min_size.height = size.height;
                res.max_size.width = size.width;
                res.max_size.height = size.height;

                std::vector<double> f = create_steps_for_range(fmt.fps_min, fmt.fps_max);

                framerate_mapping r = { res, f };
                rf.push_back(r);
            };


        for (auto& f : get_standard_resolutions(fmt.dim_min, fmt.dim_max))
        {

            if( f.width % 4 || f.width % 12 || f.height % 4 )
            {
                continue;
            }
            get_frame_rate_range(fmt.id, 0, f, fmt.fps_min, fmt.fps_max);

            add_res(fmt, f);
        }

        //tcam_info("Adding fmt to fmt_list");
        VideoFormatDescription format(nullptr, desc, rf);
        available_videoformats.push_back(format);
    }
    // set list with actual framerates
    stream_format_list_ = fmt_list;
}


std::vector<std::shared_ptr<Property>> tcam::AFU420Device::getProperties ()
{
    return property_handler->create_property_vector();
}


bool tcam::AFU420Device::set_property (const Property& p)
{
    return property_handler->set_property(p);
}


bool tcam::AFU420Device::get_property (Property& p)
{
    return property_handler->get_property(p);
}


AFU420Device::sResolutionConf AFU420Device::videoformat_to_resolution_conf (const VideoFormat& format)
//, const Property& binning)
{
    auto conf = CreateResolutionConf({0, 0}, format.get_size(), {0, 0});

    return conf;
}


bool tcam::AFU420Device::set_video_format (const VideoFormat& format)
{
    if (is_stream_on)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set format. Stream is running.");
        return false;
    }

    tcam_info("Attempting to set format to: '%s'", format.to_string().c_str());

    // TODO check if format is valid

    // TODO binning

    int ret = setup_bit_depth(img::get_bits_per_pixel(format.get_fourcc()));

    if (ret < 0)
    {
        tcam_error("Could not set bit depth. Aborting. %d", ret);
        return false;
    }
    else
    {
        tcam_debug("Set bit depth. %d", ret);
    }

    auto conf = videoformat_to_resolution_conf(format);

    ret = set_resolution_config(conf, resolution_config_mode::set);

    if (ret <= 0)
    {
        tcam_error("Could not set bit depth. Aborting.");
        return false;
    }

    // TODO set binning mode

    if (!set_framerate(format.get_framerate()))
    {
        return false;
    }

    active_video_format = format;

    tcam_info("Set format to: %s", active_video_format.to_string().c_str());

    return true;
}

VideoFormat tcam::AFU420Device::get_active_video_format () const
{
    return active_video_format;
}


std::vector<VideoFormatDescription> tcam::AFU420Device::get_available_video_formats ()
{
    return available_videoformats;
}


bool tcam::AFU420Device::set_framerate (double framerate)
{

    // set the frame rate in the camera (frame rate will be sent as 7.7FPS = 770)
    unsigned short val = ((unsigned short)(framerate * 100.0));

    tcam_debug("Attempting to set framerate value %d", val);

    int ret = control_write(BASIC_PC_TO_USB_FPS, val, 1);

    if (ret < 0)
    {
        tcam_error("Could not set framerate. LibUsb returned: %d", ret);
        return false;
    }

    return true;
}


double tcam::AFU420Device::get_framerate ()
{
    return active_video_format.get_framerate();
}


bool tcam::AFU420Device::set_sink (std::shared_ptr<SinkInterface> s)
{
    listener = s;
}


bool tcam::AFU420Device::initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>> buffs)
{
    tcam_log(TCAM_LOG_INFO, "Received %d buffer from external allocator.", buffs.size());

    buffers.reserve(buffs.size());

    for (auto& b : buffs)
    {
        buffers.push_back({b, true});
    }
    return true;
}


bool tcam::AFU420Device::release_buffers ()
{
    buffers.clear();
}


void AFU420Device::requeue_buffer (std::shared_ptr<MemoryBuffer> buffer)
{
    for (auto& b : buffers)
    {
        if (buffer == b.buffer)
        {
            b.is_queued = true;
            break;
        }
    }

}


void tcam::AFU420Device::transfer_callback (struct libusb_transfer* xfr)
{
    static int num_packets;
    if (!is_stream_on)
    {
        //libusb_free_transfer(xfr);
        return;
    }

    auto submit_transfer = [](struct libusb_transfer* xfr)
        {
            if (libusb_submit_transfer(xfr) < 0)
            {
                tcam_error("error re-submitting URB\n");
                exit(1);
            }
        };

    if (current_buffer_ == nullptr)
    {
        current_buffer_ = get_next_buffer();
        current_buffer_->clear();
        transfered_size_ = 0;

        if (current_buffer_ == nullptr)
        {
            tcam_error("Can not get buffer to fill image data. Aborting libusb callback.");
            submit_transfer(xfr);
            return;
        }
    }


	if (xfr->status != LIBUSB_TRANSFER_COMPLETED)
    {
		tcam_error("transfer status %d\n", xfr->status);
		//libusb_free_transfer(xfr);
		// exit(3);
        return;
	}

    num_packets++;
    // if (xfr->actual_length != 15360)
    // {
    //     tcam_error("Only received %d bytes in packet %d", xfr->actual_length, num_packets);
    //     submit_transfer(xfr);
    //     return;
    // }

    std::lock_guard<std::mutex> guard(control_transfer_mtx_);

    unsigned int copy_size = 0;
    if (transfered_size_ + xfr->actual_length > current_buffer_->get_buffer_size())
    {
        copy_size = current_buffer_->get_buffer_size() - transfered_size_;
    }
    else
    {
        copy_size = xfr->actual_length ;
    }

    current_buffer_->set_data(libusb_control_transfer_get_data(xfr), copy_size, transfered_size_);

	transfered_size_ += xfr->actual_length;
	num_xfer++;

    auto buffer_complete = [&] ()
        {
            if (current_buffer_->get_image_size() >= active_video_format.get_required_buffer_size())
            {
                return true;
            }
            //tcam_error("%d /// %d", current_buffer_->get_image_size(), active_video_format.get_required_buffer_size());
            return false;
        };

    if (buffer_complete())
    {
        //auto buffer = get_next_buffer();

        if (auto ptr = listener.lock())
        {
            tcam_debug("Transferred data %d - buf size %d expected size %d ", transfered_size_,
                       current_buffer_->get_image_size(),
                       active_video_format.get_required_buffer_size());
            // tcam_debug("pushing buffer to sink");
            statistics.frame_count++;
            current_buffer_->set_statistics(statistics);
            ptr->push_image(current_buffer_);

            // std::ofstream myFile ("/home/edt/test.raw", std::ios::out | std::ios::binary);
            // myFile.write((char*)current_buffer_->get_data(), current_buffer_->get_image_size());

            // exit(1);
            current_buffer_ = nullptr;
            transfered_size_ = 0;
            num_packets = 0;
        }
        else
        {
            tcam_error("ImageSink expired. Unable to deliver images.");
        }
    }

    submit_transfer(xfr);
}


std::shared_ptr<tcam::MemoryBuffer> tcam::AFU420Device::get_next_buffer ()
{
    if (buffers.empty())
    {
        tcam_error("No buffers to work with.");
        return nullptr;
    }

    for (auto& b : buffers)
    {
        if (b.is_queued)
        {
            //tcam_error("returning buffer %p", b.buffer->get_data());
            b.is_queued = false;
            return b.buffer;
        }
    }

    tcam_error("No free buffers available! %d", buffers.size());
    return nullptr;
}


void LIBUSB_CALL tcam::AFU420Device::libusb_bulk_callback (struct libusb_transfer* trans)
{
    AFU420Device* self = static_cast<AFU420Device*>(trans->user_data);
    self->transfer_callback(trans);
}


bool tcam::AFU420Device::start_stream ()
{
    // TODO test trigger mode

    const int USB2_STACKUP_SIZE = 512;
    const int USB3_STACKUP_SIZE = 32;


    // reset statistics
    statistics = {};

    // TODO get packet num from interface
    int num_iso_packets = 16;

    static const int num_transfers = 40;

    transfer_items.clear();
    transfer_items.reserve(num_transfers);

    for (int i = 0; i < num_transfers; ++i)
    {
        transfer_items.push_back({});
        transfer_items.at(i).transfer = libusb_alloc_transfer(0);

        struct libusb_transfer* xfr = (libusb_transfer*)transfer_items.at(i).transfer;

        libusb_fill_bulk_transfer(xfr,
                                  device_handle,
                                  LIBUSB_ENDPOINT_IN | USB_EP_BULK_VIDEO,
                                  (uint8_t*)&transfer_items.at(i).buffer,
                                  sizeof(transfer_items.at(i).buffer),
                                  AFU420Device::libusb_bulk_callback,
                                  this,
                                  0);
        //libusb_set_iso_packet_lengths(xfr, sizeof(transfer_items.at(i).buffer)/num_transfers);

        libusb_submit_transfer(xfr);
    }
    unsigned char val = 0;
    int ret = control_write(BASIC_PC_TO_USB_START_STREAM, val);

    if (ret < 0)
    {
        tcam_error("Stream could not be started. Aborting");
        return false;
    }

    is_stream_on = true;
    stop_all = false;

    work_thread = std::thread(&AFU420Device::stream, this);

    tcam_info("Stream started");

    return true;
}


bool tcam::AFU420Device::stop_stream ()
{
    tcam_info("stop_stream called");
    stop_all = true;
    is_stream_on = false;

    // unsigned char val = 0;
    // int ret = control_write(BASIC_PC_TO_USB_STOP_STREAM,0);
    // if (ret < 0)
    // {
    //     tcam_error("ggggggggggggggggggggggggggggggggggggggg%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%g");
    // }

    for (auto& item : transfer_items)
    {
        libusb_cancel_transfer((libusb_transfer*)item.transfer);
    }

    sleep(1);
    if (work_thread.joinable())
        work_thread.join();

    if (libusb_clear_halt(device_handle, USB_EP_BULK_VIDEO) != 0)
    {
        tcam_error("Could not halt stream");
    }
    release_buffers();

    return true;
}


void tcam::AFU420Device::stream ()
{
    /* libusb_handle_events has to be called continuously to keep the stream alive */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200;
    while (!stop_all)
    {
        int ret = libusb_handle_events_timeout_completed(session->get_session(), &tv, NULL);
        if (ret < 0)
        {
            tcam_error("LIBUSB TIMEOUT!!!");
            break;
        }
    }
}


int AFU420Device::set_resolution_config (sResolutionConf conf, resolution_config_mode mode)
{
    auto serialized_conf = serialize_resolution_config(conf);

    uint16_t test_mode = mode == resolution_config_mode::test ? 1 : 0;

    int hr = control_write(ADVANCED_PC_TO_USB_RES_FPS, test_mode, 0, serialized_conf);

	uint32_t exp_max = 0, exp_min = 0;
	control_read( exp_min, BASIC_USB_TO_PC_MIN_EXP, test_mode, 0 );
	control_read( exp_max, BASIC_USB_TO_PC_MAX_EXP, test_mode, 0 );
    //print_resolution_conf(conf);
    //tcam_debug("exp min: %d exp max: %d", exp_min, exp_max);

    return hr;
}


int AFU420Device::setup_bit_depth (int bpp)
{
    //PROFILE_STREAM_SETUP_FUNC();
    if( bpp != 8 && bpp != 10 && bpp != 12 )
    {
        return EINVAL;
    }
    int hr = control_write( BASIC_PC_TO_USB_SET_BIT_DEPTH, (uint16_t) bpp );

    if (hr < 0)
    {
        tcam_error("Failed to set a bit depth. This is most likely a too old firmware. %d %s", hr, libusb_strerror((libusb_error)hr));

        uint32_t b = 0;
        int r = control_read(b, BASIC_USB_TO_PC_GET_BIT_DEPTH, 1 );
        if (r < 0)
        {
            tcam_error("Unable to read bit depth %d", r);
        }
        else
        {
            tcam_info("Current bit depth is %u", b);
        }

        return hr;
    }

    image_bit_depth_ = bpp;
    return hr;
}


int AFU420Device::get_fps_max (double& max,
                               tcam_image_size pos,
                               tcam_image_size dim,
                               tcam_image_size binning,
                               int src_bpp)
{
    //tcam_info("in get_fps_max");
    int hr = setup_bit_depth(src_bpp);
    if (hr < 0)
    {
        tcam_error("could not set bit depth");
    }

    auto conf = CreateResolutionConf(pos, dim, binning);

    if (conf.x_output_size == 0)
    {
        tcam_error("resolution size has output size 0");
        return EINVAL;
    }

    hr = set_resolution_config(conf, resolution_config_mode::test);


    if (hr <= 0)
    {
        tcam_error("Could not set resolution config (%d)", hr);
        return hr;
    }

    const uint16_t conf_test_mode = 1;
    uint16_t ushMaxFPS = 0;

    hr = control_read(ushMaxFPS, BASIC_USB_TO_PC_MAX_FPS, conf_test_mode, 0);

    //tcam_debug("get_max_fps returned %d", ushMaxFPS);

    if (hr < 0)
    {
        return hr;
    }

    max = ((double) ushMaxFPS) / 100.0;

    return 0;
}


int AFU420Device::get_frame_rate_range (uint32_t strm_fmt_id,
                                        int scaling_factor_id,
                                        tcam_image_size dim,
                                        double& min_fps,
                                        double& max_fps)
{

    // for( auto&& e : frame_rate_cache_ )
    // {
    //     if (e.strm_fmt_id != strm_fmt_id)
    //     {
    //         continue;
    //     }
    //     if (e.scaling_factor_id != scaling_factor_id)
    //     {
    //         continue;
    //     }
    //     if (e.dim.width != dim.width || e.dim.height != dim.height)
    //     {
    //         continue;
    //     }
    //     min_fps = e.min_fps;
    //     max_fps = e.max_fps;
    //     return 0;
    // }

    const auto& stream_fmt_list = get_stream_format_descs ();

    auto f = std::find_if(stream_fmt_list.begin(), stream_fmt_list.end(),
                          [strm_fmt_id] (const stream_fmt_data& e)
                          {
                              return e.id == (int)strm_fmt_id;
                          });

    if (f == stream_fmt_list.end())
    {
        tcam_debug("???");
        // return EINVAL;
    }
    // auto func = [this, src_bpp = f->src_bpp, scaling_factor_id, dim, &min_fps, &max_fps]()
        // {                       //

            int binning = scaling_factor_id;
            if (binning <= 1)
            {
                binning = 0;
            }

            min_fps = 2.f;
            max_fps = 30.f;

            int hr = get_fps_max( max_fps, { 0, 0 }, dim, { binning, binning }, f->src_bpp );

            //tcam_debug("afu420::cam_c42_device::get_frame_rate_range" ", for dim={%d,%d}, bin=%d => max_fps=%f.", dim.width, dim.height, binning, max_fps );

            // return hr;
        // };

    // int hr = func();
    if (hr == 0)
    {
        frame_rate_cache_.push_back( { strm_fmt_id, scaling_factor_id, dim, min_fps, max_fps } );
        if( frame_rate_cache_.size() > 128 )
        {
            frame_rate_cache_.erase( frame_rate_cache_.begin(), frame_rate_cache_.begin() + 64 );
        }
    }

    return hr;
}


bool AFU420Device::property_write (property_description desc)
{
    // if (desc.property == nullptr)
    // {
    //     return false;
    // }

    // int ret = control_write(desc.prop_out, desc.index, desc.value,
    //                         (uint32_t) desc.property->get_struct().value.i.value);

    // if (ret < 0)
    // {
    //     tcam_error("Unable to write property '%s'. LibUsb returned %d",
    //                desc.property->get_name().c_str(), ret);
    //     return false;
    // }

    // return true;
    return false;
}


uint32_t AFU420Device::property_read (property_description desc)
{

    uint32_t value = 0;

    // int ret = control_read(value, desc.prop_out);

    // if (ret < 0)
    // {
    //     //tcam_error("Unable to read property '%s'. LibUsb returned %d",
    //     //         desc.property->get_name().c_str(), ret);

    //     tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    // }
    // else
    // {
    //     tcam_debug("read '%s' value: %u", value);
    // }

    return value;
}


int AFU420Device::control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex)
{
//    tcam_debug("control_write ucRequest %d, ushValue %d, ushIndex %d", ucRequest, ushValue, ushIndex);
    // std::vector<unsigned char> d;
    // d.reserve(sizeof(ushValue));
    // memcpy(d.data(), &ushValue, sizeof(ushValue));

    // return control_write(ucRequest, 0, ushIndex, d);

    unsigned int timeout = 500;
    int rval = libusb_control_transfer(device_handle,
                                       LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushValue,
                                       ushIndex,
                                       0, 0,
                                       timeout);
    return rval;

}


int AFU420Device::control_write (unsigned char ucRequest,
                                 uint16_t ushValue,
                                 uint16_t ushIndex,
                                 uint8_t data)
{
    // std::vector<unsigned char> d;
    // d.reserve(sizeof(data));
    // memcpy(d.data(), &data, sizeof(data));

    // return control_write(ucRequest, ushValue, ushIndex, d);

//    tcam_debug("control_write ucRequest %x, ushValue %d, ushIndex %d data %d", ucRequest, ushValue, ushIndex, data);

    unsigned int timeout = 700;

   int rval = libusb_control_transfer(device_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      (uint8_t)ucRequest,
                                      ushValue,
                                      ushIndex,

                                      (unsigned char*)&data, sizeof(data),
                                      timeout);
    return rval;
}


int AFU420Device::control_write (unsigned char ucRequest,
                                 uint16_t ushValue,
                                 uint16_t ushIndex,
                                 uint16_t data)
{

    //tcam_debug("control_write ucRequest %x, ushValue %d, ushIndex %d data %d", ucRequest, ushValue, ushIndex, data);

    unsigned int timeout = 700;

   int rval = libusb_control_transfer(device_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      (uint8_t)ucRequest,
                                      ushValue,
                                      ushIndex,

                                      (unsigned char*)&data, sizeof(data),
                                      timeout);
    return rval;
}


int AFU420Device::control_write (unsigned char ucRequest,
                                 uint16_t ushValue,
                                 uint16_t ushIndex,
                                 uint32_t data)
{
    // std::vector<unsigned char> d;
    // d.reserve(sizeof(data));
    // memcpy(d.data(), &data, sizeof(data));

    // return control_write(ucRequest, ushValue, ushIndex, d);

    // ushValue = 1;
    // ushIndex = 1;

    //tcam_debug("control_write ucRequest %x, ushValue %d, ushIndex %d data %d", ucRequest, ushValue, ushIndex, data);

    unsigned int timeout = 500;

   int rval = libusb_control_transfer(device_handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      (uint8_t)ucRequest,
                                      ushValue,
                                      ushIndex,

                                      (unsigned char*)&data, sizeof(data),
                                      timeout);
    return rval;
}


int AFU420Device::control_write (unsigned char ucRequest,
                                 uint16_t ushValue,
                                 uint16_t ushIndex,
                                 std::vector<unsigned char> data)
{

    //tcam_debug("control_write ucRequest %x, ushValue %d, ushIndex %d datasize %d", ucRequest, ushValue, ushIndex, sizeof(data));

    unsigned int timeout = 500;
    int rval = libusb_control_transfer(device_handle,
                                       LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushValue,
                                       ushIndex,
                                       data.data(), data.size(),
                                       timeout);
    return rval;
}


template<class T>
int AFU420Device::read_reg (T& value, unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex)
{
    unsigned int timeout = 500;

    //tcam_debug("read_reg: %x %d %d", ucRequest, ushValue, ushIndex);


    int rval = libusb_control_transfer(device_handle,
                                       DEVICE_TO_HOST,
                                       (uint8_t)ucRequest,
                                       ushIndex,
                                       ushValue,
                                       (unsigned char*)&value, sizeof(value),
                                       timeout);
    return rval;
}


int AFU420Device::control_read (uint8_t& value,
                                unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read (uint16_t& value,
                                unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read (uint32_t& value,
                                unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read (uint64_t& value,
                                unsigned char ucRequest,
                                uint16_t ushValue,
                                uint16_t ushIndex)
{
    return read_reg(value, ucRequest, ushValue, ushIndex);
}


int AFU420Device::control_read (std::vector<uint8_t>& value, unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex)
{
    unsigned int timeout = 500;

    //tcam_debug("transfer_read: %x %d %d", ucRequest, ushValue, ushIndex);

    int rval = libusb_control_transfer(device_handle,
                                       LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                       (uint8_t)ucRequest,
                                       ushIndex,
                                       ushValue,
                                       (unsigned char*)value.data(), value.size(),
                                       timeout);
    return rval;
}
