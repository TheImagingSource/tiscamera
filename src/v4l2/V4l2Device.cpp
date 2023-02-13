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

#include "V4l2Device.h"

#include "../logging.h"
#include "../utils.h"
#include "v4l2_utils.h"

#include <algorithm>
#include <cstring> /* memcpy*/
#include <dutils_img/fcc_to_string.h>
#include <errno.h>
#include <fcntl.h> /* O_RDWR O_NONBLOCK */
#include <libudev.h>
#include <linux/videodev2.h>
#include <unistd.h>

using namespace tcam;

static const int lost_countdown_default = 5;


V4l2Device::V4l2Device(const DeviceInfo& device_desc)
{
    device = device_desc;

    if ((m_fd = open(device.get_info().identifier, O_RDWR /* required */ | O_NONBLOCK, 0)) == -1)
    {
        SPDLOG_ERROR("Unable to open device \'{}\'. Reported error: {}({})",
                     device.get_info().identifier,
                     strerror(errno),
                     errno);

        throw std::runtime_error("Failed opening device.");
    }

    m_monitor_v4l2_thread = std::thread(&V4l2Device::monitor_v4l2_thread_func, this);

    p_property_backend = std::make_shared<tcam::v4l2::V4L2PropertyBackend>(m_fd);

    allocator_ = std::make_shared<V4L2Allocator>(m_fd);

    this->create_properties();
    this->index_formats();

    determine_active_video_format();

    create_videoformat_dependent_properties();
}


V4l2Device::~V4l2Device()
{
    if (m_is_stream_on)
    {
        stop_stream();
    }

    this->m_stop_monitor_v4l2_thread = true;

    if (this->m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }

    if (m_monitor_v4l2_thread.joinable())
    {
        m_monitor_v4l2_thread.join();
    }
}


DeviceInfo V4l2Device::get_device_description() const
{
    return device;
}


bool V4l2Device::set_video_format(const VideoFormat& new_format)
{
    if (m_is_stream_on == true)
    {
        SPDLOG_ERROR("Device is streaming.");
        return false;
    }

    SPDLOG_DEBUG("Requested format change to '{}' {:x}",
                 new_format.to_string(),
                 new_format.get_fourcc());

    uint32_t fourcc = new_format.get_fourcc();

    // use greyscale for camera interaction
    if (m_emulate_bayer)
    {
        fourcc = V4L2_PIX_FMT_GREY;
    }

    if (fourcc == FOURCC_Y800)
    {
        fourcc = V4L2_PIX_FMT_GREY;
    }

    // set format in camera

    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    fmt.fmt.pix.width = new_format.get_size().width;
    fmt.fmt.pix.height = new_format.get_size().height;

    fmt.fmt.pix.pixelformat = fourcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    int ret = tcam_xioctl(this->m_fd, VIDIOC_S_FMT, &fmt);

    if (ret < 0)
    {
        SPDLOG_ERROR("Error while setting format '{}'", strerror(errno));
        return false;
    }

    /* framerate */

    if (!set_framerate(new_format.get_framerate()))
    {
        SPDLOG_ERROR("Unable to set framerate to {}", new_format.get_framerate());
        return false;
    }

    set_scaling(new_format.get_scaling());

    /* validation */

    determine_active_video_format();
    SPDLOG_DEBUG("Active format is: '{}'", m_active_video_format.to_string().c_str());

    update_properties(new_format);

    return true;
}

VideoFormat V4l2Device::get_active_video_format() const
{
    return m_active_video_format;
}


std::vector<VideoFormatDescription> V4l2Device::get_available_video_formats()
{
    //SPDLOG_DEBUG("Returning {} formats.", m_available_videoformats.size());
    return m_available_videoformats;
}


bool V4l2Device::set_framerate(double framerate)
{
    if (m_is_stream_on == true)
    {
        SPDLOG_ERROR("Device is streaming.");
        return false;
    }

    std::vector<double> vec;

    for (auto& d : framerate_conversions) { vec.push_back(d.fps); }

    std::sort(vec.begin(), vec.end());

    auto iter_low = std::lower_bound(vec.begin(), vec.end(), framerate);

    if (iter_low == vec.end())
    {
        SPDLOG_ERROR("No framerates available");
        return false;
    }

    framerate = *iter_low;

    auto f = [framerate](const framerate_conv& _f)
    {
        return framerate == _f.fps;
    };

    auto fps = std::find_if(framerate_conversions.begin(), framerate_conversions.end(), f);


    if (fps == framerate_conversions.end())
    {
        SPDLOG_ERROR("unable to find corresponding framerate settings.");
        return false;
    }

    struct v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    parm.parm.capture.timeperframe.numerator = fps->numerator;
    parm.parm.capture.timeperframe.denominator = fps->denominator;
    SPDLOG_TRACE("Setting framerate to '{}' =  {} / {}",
                 framerate,
                 parm.parm.capture.timeperframe.denominator,
                 parm.parm.capture.timeperframe.numerator);

    int ret = tcam_xioctl(m_fd, VIDIOC_S_PARM, &parm);

    if (ret < 0)
    {
        SPDLOG_ERROR("Failed to set frame rate\n");
        return false;
    }

    return true;
}


double V4l2Device::get_framerate()
{
    // what about range framerates?
    // - ranges are not supported by uvc
    struct v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = tcam_xioctl(m_fd, VIDIOC_G_PARM, &parm);

    if (ret < 0)
    {
        SPDLOG_ERROR("Failed to get frame rate\n");
        return 0.0;
    }

    SPDLOG_INFO("Current framerate is {} / {} fps",
                parm.parm.capture.timeperframe.denominator,
                parm.parm.capture.timeperframe.numerator);

    return (double)parm.parm.capture.timeperframe.denominator
           / (double)parm.parm.capture.timeperframe.numerator;
}

bool V4l2Device::initialize_buffers(std::shared_ptr<BufferPool> pool)
{
    if (m_is_stream_on)
    {
        SPDLOG_ERROR("Stream running.");
        return false;
    }

    pool_ = pool;

    auto b = pool_->get_buffer();

    this->m_buffers.clear();
    this->m_buffers.reserve(b.size());

    for (unsigned int i = 0; i < b.size(); ++i)
    {
        buffer_info info = { b.at(i), false };

        this->m_buffers.push_back(info);
    }

    return true;
}


bool V4l2Device::release_buffers()
{
    if (m_is_stream_on)
    {
        return false;
    }

    // dequeue all buffers
    struct v4l2_requestbuffers req = {};

    req.count = 0; // free all buffers
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == tcam_xioctl(m_fd, VIDIOC_REQBUFS, &req))
    {
        SPDLOG_ERROR("Error while calling VIDIOC_REQBUFS to empty buffer queue. {}",
                     strerror(errno));
    }

    m_buffers.clear();
    return true;
}


bool V4l2Device::queue_mmap(int i, std::shared_ptr<ImageBuffer> b)
{
    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    buf.index = i;

    int ret = tcam_xioctl(m_fd, VIDIOC_QBUF, &buf);
    if (ret == -1)
    {
        SPDLOG_ERROR("Unable to queue mmap buffer({}): {} {}", errno, strerror(errno), fmt::ptr(b->get_image_buffer_ptr()));
        return false;
    }

    return true;
}


bool V4l2Device::queue_userptr(int i, std::shared_ptr<ImageBuffer> b)
{

    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.index = i;
    buf.m.userptr = (unsigned long)b->get_image_buffer_ptr();
    buf.length = b->get_image_buffer_size();

    // requeue buffer
    int ret = tcam_xioctl(m_fd, VIDIOC_QBUF, &buf);
    if (ret == -1)
    {
        SPDLOG_ERROR("Could not requeue buffer");
        return false;
    }
    return true;
}


void V4l2Device::requeue_buffer(const std::shared_ptr<ImageBuffer>& buffer)
{

    for (unsigned int i = 0; i < m_buffers.size(); ++i)
    {

        auto& b = m_buffers.at(i);

        auto buf_ptr = b.buffer.lock();

        if (!b.is_queued && buf_ptr == buffer)
        {
            switch (pool_->get_memory_type())
            {
                case TCAM_MEMORY_TYPE_USERPTR:
                {
                    if (queue_userptr(i, buf_ptr))
                    {
                        b.is_queued = true;
                    }
                    break;
                }
                case TCAM_MEMORY_TYPE_MMAP:
                {
                    if (queue_mmap(i, buf_ptr))
                    {
                        b.is_queued = true;
                    }
                    break;
                }
                case TCAM_MEMORY_TYPE_DMA:
                case TCAM_MEMORY_TYPE_DMA_IMPORT:
                {
                    SPDLOG_ERROR("Queueing of DMA not implemented");
                    break;
                }
            }
        }
    }
}


void V4l2Device::update_stream_timeout()
{
    for (const auto& p : m_properties)
    {
        if (p->get_name() == "ExposureTime")
        {
            auto val = std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(p)->get_value();

            if (val)
            {
                m_stream_timeout_sec = (val.value() / 1000000) + 2;
            }
            else
            {
                // TODO: implement error handling
            }
            break;
        }
    }
    SPDLOG_DEBUG("Setting stream timeout to {}", m_stream_timeout_sec.load());
}


bool V4l2Device::start_stream(const std::shared_ptr<IImageBufferSink>& sink)
{
    switch (pool_->get_memory_type())
    {
        case TCAM_MEMORY_TYPE_USERPTR:
        {
            init_userptr_buffers();
            break;
        }
        case TCAM_MEMORY_TYPE_MMAP:
        {
            SPDLOG_DEBUG("init mmap");
            init_mmap_buffers();
            break;
        }
        case TCAM_MEMORY_TYPE_DMA:
        case TCAM_MEMORY_TYPE_DMA_IMPORT:
        {
            SPDLOG_ERROR("MEMORY type not implemented");
            return false;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == tcam_xioctl(m_fd, VIDIOC_STREAMON, &type))
    {
        SPDLOG_ERROR("Unable to set ioctl VIDIOC_STREAMON {} {}", errno, strerror(errno));
        return false;
    }

    m_statistics = {};

    m_listener = sink;

    m_is_stream_on = true;

    update_stream_timeout();

    SPDLOG_INFO("Starting stream in work thread.");

    this->m_work_thread = std::thread(&V4l2Device::stream, this);

    return true;
}


void V4l2Device::stop_stream()
{
    if (!m_is_stream_on)
    {
        return;
    }

    SPDLOG_TRACE("Stopping stream...");

    if (m_is_stream_on)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        int ret = tcam_xioctl(m_fd, VIDIOC_STREAMOFF, &type);

        if (ret < 0)
        {
            SPDLOG_ERROR("Unable to set ioctl VIDIOC_STREAMOFF {}", errno);
        }
    }

    m_is_stream_on = false;

    if (m_work_thread.joinable())
    {
        m_work_thread.join();
    }

    m_listener.reset();

    SPDLOG_DEBUG("Stopped stream");
}


void V4l2Device::notify_device_lost_func()
{
    SPDLOG_INFO("notifying callbacks about lost device");
    if (this->m_is_stream_on)
    {
        stop_stream();
    }
    this->lost_device();
}


void V4l2Device::lost_device()
{
    this->notify_device_lost();
}


/*
 * in kernel versions < 3.15 uvcvideo does not correctly interpret bayer 8-bit
 * this function detects those cases and corrects all settings
 */
static bool checkForBayer(const v4l2_fmtdesc& fmtdesc, v4l2_fmtdesc& new_desc)
{

    new_desc = fmtdesc;
    // when v4l2 does not recognize a format fourcc it will
    // set the fourcc to 0 and pass the description string.
    // we compare the possible strings and correct the fourcc
    // for later emulation of the correct pattern

    if (strcmp((const char*)fmtdesc.description, "47425247-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_GBRG8;
        new_desc.pixelformat = FOURCC_GRBG8;
        strncpy((char*)new_desc.description, "BayerGR8", sizeof(new_desc.description) - 1);
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "42474752-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_BGGR8;
        new_desc.pixelformat = FOURCC_RGGB8;
        strncpy((char*)new_desc.description, "BayerRG8", sizeof(new_desc.description) - 1);
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "31384142-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_RGGB8;
        new_desc.pixelformat = FOURCC_BGGR8;
        strncpy((char*)new_desc.description, "BayerBG8", sizeof(new_desc.description) - 1);
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "47524247-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_GRBG8;
        new_desc.pixelformat = FOURCC_GBRG8;
        strncpy((char*)new_desc.description, "BayerGB8", sizeof(new_desc.description) - 1);
        return true;
    }

    return false;
}


void V4l2Device::determine_scaling()
{
    m_scale.scale_type = ImageScalingType::Unknown;

    auto check_prop = [this] (const std::string& name, ImageScalingType flag)
    {
        auto prop = tcam::property::find_property(m_internal_properties, name);
        if (prop)
        {
            m_scale.scale_type = flag;
            m_scale.properties.push_back(prop);
            return true;
        }
        return false;
    };

    if (check_prop("Override Scanning Mode", ImageScalingType::Override))
    {
        static const char* scanning_mode_entries[] =
            {
                "Scanning Mode Selector",
                "Scanning Mode Identifier",
                "Scanning Mode Scale Horizontal",
                "Scanning Mode Scale Vertical",
                "Scanning Mode Binning H",
                "Scanning Mode Binning V",
                "Scanning Mode Skipping H",
                "Scanning Mode Skipping V",
                "Scanning Mode Flags",
            };

        for (const auto& entry : scanning_mode_entries)
        {
            if (auto p = tcam::property::find_property(m_internal_properties, entry))
            {
                m_scale.properties.push_back(p);
            }
            else
            {
                SPDLOG_ERROR("Unable to find Scanning Mode property \"{}\". Disabling Binning/Skipping", entry);
                m_scale.scale_type = ImageScalingType::None;
                m_scale.properties.clear();
            }
        }
    }
    else
    {
        check_prop("Binning", ImageScalingType::Binning);
        check_prop("BinningHorizontal", ImageScalingType::Binning);
        check_prop("BinningVertical", ImageScalingType::Binning);

        ImageScalingType to_set = ImageScalingType::Skipping;

        // we already have binning
        // if something skipping related is found we have both
        if (m_scale.scale_type == ImageScalingType::Binning)
        {
            to_set = ImageScalingType::BinningSkipping;
        }

        check_prop("Skipping", to_set);
        check_prop("SkippingHorizontal", to_set);
        check_prop("SkippingVertical", to_set);
    }

    if (m_scale.scale_type == ImageScalingType::Unknown)
    {
        m_scale.scale_type = ImageScalingType::None;
    }
}


image_scaling V4l2Device::get_current_scaling()
{
    if (m_scale.scale_type == Unknown)
    {
        determine_scaling();
    }

    if (m_scale.scale_type == ImageScalingType::None)
    {
        return {};
    }
    else if (m_scale.scale_type == ImageScalingType::Override)
    {
        auto override_scanning_mode = tcam::property::find_property(m_internal_properties, "Override Scanning Mode");
        if (!override_scanning_mode)
        {
            SPDLOG_ERROR("Unable to find 'Override Scanning Mode'");
            return {};
        }

        auto osm = dynamic_cast<tcam::property::IPropertyInteger*>(override_scanning_mode.get());
        auto ret = osm->get_value();

        if (!ret)
        {
            SPDLOG_ERROR("Unable to retrieve value for 'Override Scanning Mode': {}", ret.error().message());
            return {};
        }

        int current_value = ret.value();

        for (const auto i : m_scale.override_index)
        {
            if (i.override_value == current_value)
            {
                return m_scale.scales.at(i.scales_index);
            }
        }
    }

    image_scaling ret = {};

    if (m_scale.scale_type == ImageScalingType::Binning || m_scale.scale_type == ImageScalingType::BinningSkipping)
    {
        auto bin_total = tcam::property::find_property(m_scale.properties, "Binning");

        // only has total binning
        if (bin_total)
        {

            auto bin = dynamic_cast<tcam::property::IPropertyInteger*>(bin_total.get());

            auto res = bin->get_value();
            if (!res)
            {
                SPDLOG_ERROR("Unable to retrieve value for Binning: {}", res.as_failure().error().message());
                return {};
            }

            ret.binning_h = res.value();
            ret.binning_v = res.value();

        }
        else
        {
            auto bin_hb = tcam::property::find_property(m_scale.properties, "BinningHorizontal");
            auto bin_vb = tcam::property::find_property(m_scale.properties, "BinningVertical");

            auto bin_h = dynamic_cast<tcam::property::IPropertyInteger*>(bin_hb.get());
            auto bin_v = dynamic_cast<tcam::property::IPropertyInteger*>(bin_vb.get());

            auto res = bin_h->get_value();

            if (!res)
            {
                SPDLOG_ERROR("Unable to retrieve value for BinningHorizontal: {}", res.as_failure().error().message());
                return {};
            }
            ret.binning_h = res.value();
            res = bin_v->get_value();
            if (!res)
            {
                SPDLOG_ERROR("Unable to retrieve value for BinningVertical: {}", res.as_failure().error().message());
                return {};
            }
            ret.binning_v = res.value();

        }
    }
    if (m_scale.scale_type == ImageScalingType::Skipping || m_scale.scale_type == ImageScalingType::BinningSkipping)
    {
        SPDLOG_ERROR("SKIPPING NOT IMPLEMENTED");

    }

    return ret;
}


bool V4l2Device::set_scaling(const image_scaling& scale)
{
    bool is_valid = false;

    if (scale.is_default())
    {
        is_valid = true;
    }
    else
    {
        for (const auto& s : m_scale.scales)
        {
            if (s == scale)
            {
                is_valid = true;
                break;
            }
        }
    }
    if (!is_valid)
    {
        SPDLOG_ERROR("Scaling description is not valid.");
        return false;
    }

    if (m_scale.scale_type == ImageScalingType::None)
    {
        return true;
    }

    if(m_scale.scale_type == ImageScalingType::Override)
    {
        if (scale.is_default())
        {
            auto over = tcam::property::find_property(m_scale.properties, "Override Scanning Mode");

            auto override_prop = dynamic_cast<tcam::property::IPropertyInteger*>(over.get());

            SPDLOG_INFO("Setting override mode to: {}", 1);

            auto ret = override_prop->set_value(1);

            if (!ret)
            {
                SPDLOG_ERROR("Unable to set 'Override Scanning Mode': {}", ret.as_failure().error().message());
                return false;
            }
            return true;
        }

        for (size_t i = 0; i < m_scale.scales.size(); i++)
        {
            if (m_scale.scales.at(i) == scale)
            {
                for (const auto& o : m_scale.override_index)
                {
                    if (o.scales_index == (int)i)
                    {
                        auto over = tcam::property::find_property(m_scale.properties, "Override Scanning Mode");

                        auto override_prop = dynamic_cast<tcam::property::IPropertyInteger*>(over.get());

                        SPDLOG_INFO("Setting override mode to: {}", o.override_value);

                        auto ret = override_prop->set_value(o.override_value);

                        if (!ret)
                        {
                            SPDLOG_ERROR("Unable to set 'Override Scanning Mode': {}", ret.as_failure().error().message());
                            return false;
                        }
                        return true;
                    }
                }
            }
        }
    }
    else
    {
        if (m_scale.scale_type == ImageScalingType::Binning || m_scale.scale_type == ImageScalingType::BinningSkipping)
        {

            auto bin_total = tcam::property::find_property(m_scale.properties, "Binning");

            // only has total binning
            if (bin_total)
            {
                auto bin = dynamic_cast<tcam::property::IPropertyInteger*>(bin_total.get());

                auto ret = bin->set_value(scale.binning_h);
                if (!ret)
                {
                    return false;
                }
            }
            else
            {
                auto bin_hb = tcam::property::find_property(m_scale.properties, "BinningHorizontal");
                auto bin_vb = tcam::property::find_property(m_scale.properties, "BinningVertical");

                auto bin_h = dynamic_cast<tcam::property::IPropertyInteger*>(bin_hb.get());
                auto bin_v = dynamic_cast<tcam::property::IPropertyInteger*>(bin_vb.get());

                auto ret = bin_h->set_value(scale.binning_h);

                if (!ret)
                {
                    return false;
                }

                ret = bin_v->set_value(scale.binning_v);
                if (!ret)
                {
                    return false;
                }
            }
        }
        if (m_scale.scale_type == ImageScalingType::Skipping || m_scale.scale_type == ImageScalingType::BinningSkipping)
        {
            SPDLOG_ERROR("SKIPPING NOT IMPLEMENTED");
            return false;
        }
    }

    return true;
}


void V4l2Device::generate_scales()
{
    if (m_scale.scale_type == Unknown)
    {
        determine_scaling();
    }

    if (m_scale.scale_type == ImageScalingType::None)
    {
        return;
    }
    else if (m_scale.scale_type == ImageScalingType::Override)
    {
        auto override_scanning_mode = tcam::property::find_property(m_scale.properties,"Override Scanning Mode");
        if (!override_scanning_mode)
        {
            return;
        }

        // to actually set, use 'Override Scanning Mode'
        // to test, use ScanningModeSelector
        // to see if stuff is a valid setting, use ScanningModeIdentifier == ScanningModeSelector

        auto p = tcam::property::find_property(m_scale.properties, "Scanning Mode Selector");
        auto identifier_b = tcam::property::find_property(m_scale.properties, "Scanning Mode Identifier");
        auto binning_hb = tcam::property::find_property(m_scale.properties, "Scanning Mode Binning H");
        auto binning_vb = tcam::property::find_property(m_scale.properties, "Scanning Mode Binning V");
        auto skipping_hb = tcam::property::find_property(m_scale.properties, "Scanning Mode Skipping H");
        auto skipping_vb = tcam::property::find_property(m_scale.properties, "Scanning Mode Skipping V");
        //auto flags_b = tcam::property::find_property(m_scale.properties, "Scanning Mode Flags");

        auto identifier = dynamic_cast<tcam::property::IPropertyInteger*>(identifier_b.get());
        auto binning_h = dynamic_cast<tcam::property::IPropertyInteger*>(binning_hb.get());
        auto binning_v = dynamic_cast<tcam::property::IPropertyInteger*>(binning_vb.get());
        auto skipping_h = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_hb.get());
        auto skipping_v = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_vb.get());
        //auto flags = dynamic_cast<tcam::property::IPropertyInteger*>(flags_b.get());

        auto mode = dynamic_cast<tcam::property::IPropertyInteger*>(p.get());
        int current_value = mode->get_value().value();

        for (int i = mode->get_range().min; i <= mode->get_range().max; i += mode->get_range().stp)
        {
            if (!mode->set_value(i))
            {
                SPDLOG_ERROR("mode could not be changed");
                continue;
            }

            // ScanningModeIdentifier has to be equal to ScanningModeSelector
            // if it is 1 it is the default; ignore that
            if (identifier->get_value().value() != i || i == 1)
            {
                continue;
            }

            // SPDLOG_ERROR("mode: {} ident: {} bin h: {} bin v: {} skip h: {} skip v: {} flags: {}",
            //              i, identifier->get_value().value(),
            //              binning_h->get_value().value(), binning_v->get_value().value(),
            //              skipping_h->get_value().value(), skipping_v->get_value().value(),
            //              flags->get_value().value());

            image_scaling new_scale = {};

            new_scale.binning_h = binning_h->get_value().value();
            new_scale.binning_v = binning_v->get_value().value();

            new_scale.skipping_h = skipping_h->get_value().value();
            new_scale.skipping_v = skipping_v->get_value().value();

            m_scale.scales.push_back(new_scale);

            m_scale.override_index.push_back({i, (int)m_scale.scales.size() - 1});

        }
        // restore previous setting
        auto ret = mode->set_value(current_value);
        if (!ret)
        {
            SPDLOG_ERROR("Probing override scanning mode ended with an error: {}", ret.as_failure().error().message());
        }
    }

    if (m_scale.scale_type == ImageScalingType::Binning || m_scale.scale_type == ImageScalingType::BinningSkipping)
    {
        auto binning = tcam::property::find_property(m_scale.properties, "Binning");
        // only has binning
        if (binning)
        {
            auto b = dynamic_cast<tcam::property::IPropertyInteger*>(binning.get());

            for (int i = b->get_range().min; i <= b->get_range().max; i++)
            {
                // only accept valid values
                if (i != 2 && i != 4 && i != 8)
                {
                    continue;
                }

                image_scaling new_scale = {};

                new_scale.binning_h = i;
                new_scale.binning_v = i;

                // SPDLOG_INFO("New binning: {}x{}", i, i);

                m_scale.scales.push_back(new_scale);
            }

        }


    }
    if (m_scale.scale_type == ImageScalingType::Skipping || m_scale.scale_type == ImageScalingType::BinningSkipping)
    {
        // TODO add skipping
        SPDLOG_ERROR("Skipping not implemented");
    }

}


void V4l2Device::index_formats()
{
    generate_scales();

    struct v4l2_fmtdesc fmtdesc = {};
    struct v4l2_frmsizeenum frms = {};

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (fmtdesc.index = 0; !tcam_xioctl(m_fd, VIDIOC_ENUM_FMT, &fmtdesc); fmtdesc.index++)
    {
        struct tcam_video_format_description desc = {};

        struct v4l2_fmtdesc new_desc = {};
        m_emulate_bayer = checkForBayer(fmtdesc, new_desc);

        // internal fourcc definitions are identical with v4l2
        desc.fourcc = new_desc.pixelformat;
        memcpy(desc.description, new_desc.description, sizeof(new_desc.description));
        frms.pixel_format = fmtdesc.pixelformat;

        std::vector<struct framerate_mapping> rf;

        // needed for binning/skipping later on
        tcam_image_size sensor_size = {};

        // iterate all framesizes to find largest
        for (frms.index = 0; !tcam_xioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &frms); frms.index++)
        {
            if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                sensor_size.width = std::max(frms.discrete.width,sensor_size.width);
                sensor_size.height = std::max(frms.discrete.height,sensor_size.height);
            }
        }

        for (frms.index = 0; !tcam_xioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &frms); frms.index++)
        {
            if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                struct tcam_resolution_description res = {};

                res.min_size.width = frms.discrete.width;
                res.max_size.width = frms.discrete.width;
                res.min_size.height = frms.discrete.height;
                res.max_size.height = frms.discrete.height;

                std::vector<double> f = index_framerates(frms);

                res.type = TCAM_RESOLUTION_TYPE_FIXED;

                framerate_mapping r = { res, f };
                rf.push_back(r);

                for (auto s : m_scale.scales)
                {
                    if (s.legal_resolution(sensor_size, res.max_size))
                    {
                        // being here we have a valid resolution/scaling combo
                        // copy resolution desc and add scaling
                        auto scaled_res = res;
                        scaled_res.scaling = s;

                        rf.push_back({scaled_res, f});
                    }
                }
            }
            else
            {
                // TIS USB cameras do not have this kind of setting
                SPDLOG_ERROR("Encountered unknown V4L2_FRMSIZE_TYPE");
            }
        }

        // algorithms, etc. use Y800 as an identifier.
        // declare format as such.
        if (desc.fourcc == V4L2_PIX_FMT_GREY) // equals FOURCC_GREY
        {
            desc.fourcc = FOURCC_Y800;
        }

        // VideoFormatDescription format(m_format_handler, desc, rf);
        VideoFormatDescription format(nullptr, desc, rf);
        this->m_available_videoformats.push_back(format);

        SPDLOG_DEBUG("Found format: {}", img::fcc_to_string(format.get_fourcc()));
    }
}


std::vector<double> V4l2Device::index_framerates(const struct v4l2_frmsizeenum& frms)
{
    struct v4l2_frmivalenum frmival = {};

    frmival.pixel_format = frms.pixel_format;
    frmival.width = frms.discrete.width;
    frmival.height = frms.discrete.height;

    std::vector<double> f;

    for (frmival.index = 0; tcam_xioctl(m_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0;
         frmival.index++)
    {
        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {

            // v4l2 lists frame rates as fractions (number of seconds / frames (e.g. 1/30))
            // we however use framerates as fps (e.g. 30/1)
            // therefor we have to switch numerator and denominator

            double frac = (double)frmival.discrete.denominator / frmival.discrete.numerator;
            f.push_back(frac);

            framerate_conv c = { frac, frmival.discrete.numerator, frmival.discrete.denominator };
            framerate_conversions.push_back(c);
        }
        else
        {
            // not used

            continue;
            // double fps_min = ((double)frmival.stepwise.min.numerator / (double)frmival.stepwise.min.denominator);
            // double fps_max = ((double)frmival.stepwise.max.numerator / (double)frmival.stepwise.max.denominator);
        }
    }

    return f;
}

void V4l2Device::determine_active_video_format()
{

    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = tcam_xioctl(this->m_fd, VIDIOC_G_FMT, &fmt);

    if (ret < 0)
    {
        SPDLOG_ERROR("Error while querying video format");

        return;
    }

    v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = tcam_xioctl(m_fd, VIDIOC_G_PARM, &parm);

    if (ret < 0)
    {

        SPDLOG_ERROR("Failed to set frame rate");
        return;
    }

    tcam_video_format format = {};
    format.fourcc = fmt.fmt.pix.pixelformat;

    if (format.fourcc == V4L2_PIX_FMT_GREY)
    {
        format.fourcc = FOURCC_Y800;
    }

    format.width = fmt.fmt.pix.width;
    format.height = fmt.fmt.pix.height;

    format.framerate = get_framerate();

    format.scaling = get_current_scaling();

    this->m_active_video_format = VideoFormat(format);
}


bool V4l2Device::extension_unit_is_loaded()
{
    /*
      This function checks if any custom properties
      have been loaded. The used identifier is 0x199e.
      It is used as a prefix for all TIS property IDs.
     */
    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    while (tcam_xioctl(this->m_fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        if (((qctrl.id >> 12) ^ 0x199e) == 0)
        {
            return true;
        }
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
    return false;
}


void V4l2Device::stream()
{
    tcam::set_thread_name("tcam_v4l2_strm");

    m_already_received_valid_image = false;
    static const int log_repetition = 10;
    int log_repetition_counter = 0;
    int lost_countdown = lost_countdown_default;
    // period elapsed for current image
    int waited_seconds = 0;
    // maximum_waiting period
    // do not compare waited_seconds with m_stream_timeout_sec directly
    // m_stream_timeout_sec may be set to low values while we are
    // still waiting for a long exposure image
    // still 'step in between' prevents such errors
    int waiting_period = m_stream_timeout_sec;

    while (this->m_is_stream_on)
    {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);

        int select_timeout = 2;

        /* Timeout. */
        struct timeval tv;
        tv.tv_sec = select_timeout;
        tv.tv_usec = 0;

        /* Wait until device gives go */
        int ret = select(m_fd + 1, &fds, NULL, NULL, &tv);
        if (ret == -1)
        {
            if (errno == EINTR)
            {
                continue; // intermittent wake, continue
            }
            else
            {
                /* error during select */
                SPDLOG_ERROR("Error during select. errno: %d (%s)", errno, strerror(errno));
                return;
            }
        }

        // before we recheck any variables,
        // just quit the loop because stop was requested
        if (!m_is_stream_on)
        {
            return;
        }

        if (ret == 0) // timeout encountered
        {
            if (is_trigger_mode_enabled())
            {
                continue; // timeout while trigger is enabled, just continue
            }

            if (waited_seconds < waiting_period)
            {
                waited_seconds += select_timeout;
            }
            else
            {
                SPDLOG_ERROR("Timeout while waiting for new image buffer.");
                m_statistics.frames_dropped++;
                waited_seconds = 0;
                lost_countdown--;
            }
        }
        else
        {
            bool ret_value = get_frame();
            if (ret_value)
            {
                lost_countdown = lost_countdown_default; // reset lost countdown variable
                log_repetition_counter = 0;
            }
            else
            {
                lost_countdown--;
            }
            waiting_period = m_stream_timeout_sec;
        }
        if (lost_countdown <= 0 && log_repetition_counter < log_repetition)
        {
            SPDLOG_WARN("Did not receive image for long time.");
            lost_countdown = lost_countdown_default;
            if (log_repetition_counter < log_repetition)
            {
                log_repetition_counter++;
            }
            if (log_repetition_counter >= log_repetition)
            {
                SPDLOG_WARN("Stopping messages \"Did not receive image for long time.\".");
            }
        }
    }
}


bool V4l2Device::is_trigger_mode_enabled()
{
    for (auto& p : m_properties)
    {
        if (p->get_name() == "TriggerMode")
        {
            auto val = std::dynamic_pointer_cast<tcam::property::IPropertyEnum>(p)->get_value();
            if (val)
            {
                if (val.value() == "On")
                {
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}


bool V4l2Device::get_frame()
{
    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (pool_->get_memory_type() == TCAM_MEMORY_TYPE_USERPTR)
    {
        buf.memory = V4L2_MEMORY_USERPTR;
    }
    else
    {
        buf.memory = V4L2_MEMORY_MMAP;
    }
    int ret = tcam_xioctl(m_fd, VIDIOC_DQBUF, &buf);

    if (ret == -1)
    {
        SPDLOG_TRACE("Unable to dequeue buffer.");
        return false;
    }

    auto& image_buffer = m_buffers.at(buf.index);

    image_buffer.is_queued = false;

    // buf.bytesused
    /* The number of bytes occupied by the data in the buffer. It depends on
       the negotiated data format and may change with each buffer for compressed
       variable size data like JPEG images. Drivers must set this field when
       type refers to a capture stream, applications when it refers to an output
       stream. If the application sets this to 0 for an output stream,
       then bytesused will be set to the size of the buffer (see the length
       field of this struct) by the driver. For multiplanar formats this
       field is ignored and the planes pointer is used instead.
     */

    if (m_active_video_format.get_fourcc() != FOURCC_MJPG)
    {
        if (buf.bytesused != (this->m_active_video_format.get_required_buffer_size()))
        {
            if (m_already_received_valid_image)
            {
                SPDLOG_ERROR("Buffer has wrong size. Got: {} Expected: {} Dropping...",
                             buf.bytesused,
                             this->m_active_video_format.get_required_buffer_size());
            }
            //SPDLOG_ERROR("error requeue");
            requeue_buffer(image_buffer.buffer.lock());
            return true;
        }
    }
    m_already_received_valid_image = true;
    // v4l2 timestamps contain seconds and microseconds
    // here they are converted to nanoseconds
    m_statistics.capture_time_ns =
        ((long long)buf.timestamp.tv_sec * 1000 * 1000 * 1000) + (buf.timestamp.tv_usec * 1000);
    m_statistics.frame_count++;
    auto b = image_buffer.buffer.lock();
    b->set_statistics(m_statistics);
    b->set_valid_data_length(buf.bytesused);

    //SPDLOG_INFO("pushing new buffer");

    if (auto ptr = m_listener.lock())
    {
        ptr->push_image(b);
    }
    else
    {
        SPDLOG_ERROR("ImageSink expired. Unable to deliver images.");
        return false;
    }

    return true;
}


void V4l2Device::init_userptr_buffers()
{
    struct v4l2_requestbuffers req = {};

    req.count = m_buffers.size();
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == tcam_xioctl(m_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            SPDLOG_ERROR("{} does not support user pointer i/o", device.get_serial());
            return;
        }
        else
        {
            SPDLOG_ERROR("VIDIOC_REQBUFS {}", strerror(errno));
        }
    }

    for (unsigned int i = 0; i < m_buffers.size(); ++i)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;

        auto b = m_buffers.at(i).buffer.lock();

        buf.m.userptr = (unsigned long)b->get_image_buffer_ptr();
        buf.length = b->get_image_buffer_size();

        SPDLOG_DEBUG("Queueing buffer({}) with length {}", fmt::ptr(b->get_image_buffer_ptr()), buf.length);

        if (-1 == tcam_xioctl(m_fd, VIDIOC_QBUF, &buf))
        {
            SPDLOG_ERROR("Unable to queue v4l2_buffer 'VIDIOC_QBUF' {}", strerror(errno));
            return;
        }
        else
        {
            //SPDLOG_TRACE("Successfully queued v4l2_buffer");
            m_buffers.at(i).is_queued = true;
        }
    }
}


void V4l2Device::init_mmap_buffers()
{
    // this request _MUST_ be done in the allocator
    // without it the mmap requests will fail
    //
    // struct v4l2_requestbuffers req = {};
    // req.count = m_buffers.size();
    // req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // req.memory = V4L2_MEMORY_MMAP;
    // if (tcam_xioctl(m_fd, VIDIOC_REQBUFS, &req) == -1) {
    //     return;
    // }
    // if (req.count < 2) {
    //     SPDLOG_ERROR("Insufficient memory for memory mapping{}", req.count);
    //     return;
    // }

    for (unsigned int n_buffers = 0; n_buffers < m_buffers.size(); ++n_buffers)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (tcam_xioctl(m_fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            SPDLOG_ERROR("WHAT index: {} {} {}", buf.index, errno, strerror(errno));
            //return;
        }
    }

    for (unsigned int i = 0; i < m_buffers.size(); ++i)
    {
        if (queue_mmap(i, m_buffers.at(i).buffer.lock()))
        {
            m_buffers.at(i).is_queued = true;
        }
    }
}


void V4l2Device::init_dma_buffers()
{
    for (unsigned int i = 0; i < m_buffers.size(); ++i)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;
        // TODO: pass dma fd
        // buf.m.fd = dmafd;

        if (tcam_xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
        {
            perror("VIDIOC_QBUF");
            return;
        }

    }
}


tcam_image_size V4l2Device::get_sensor_size() const
{
    tcam_image_size size = {};
    for (const auto& f : m_available_videoformats)
    {
        for (const auto& r : f.get_resolutions())
        {
            size.width = std::max( r.max_size.width, size.width );
            size.height = std::max( r.max_size.height, size.height );
        }
    }
    return size;
}


void V4l2Device::monitor_v4l2_thread_func()
{
    tcam::set_thread_name( "tcam_v4l2_mon" );
    auto udev = udev_new();
    if (!udev)
    {
        SPDLOG_ERROR("Failed to create udev context");
        return;
    }

    /* Set up a monitor to monitor hidraw devices */
    auto mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon)
    {
        SPDLOG_ERROR("Failed to create udev monitor");
        udev_unref(udev);
        return;
    }
    udev_monitor_filter_add_match_subsystem_devtype(mon, "video4linux", NULL);
    udev_monitor_enable_receiving(mon);
    /* Get the file descriptor (fd) for the monitor.
       This fd will get passed to select() */
    int udev_fd = udev_monitor_get_fd(mon);

    /* This section will run continuously, calling usleep() at
       the end of each pass. This is to demonstrate how to use
       a udev_monitor in a non-blocking way. */
    while (!m_stop_monitor_v4l2_thread)
    {
        /* Set up the call to select(). In this case, select() will
           only operate on a single file descriptor, the one
           associated with our udev_monitor. Note that the timeval
           object is set to 0, which will cause select() to not
           block. */
        fd_set fds;
        int select_fd = udev_fd + 1;

        FD_ZERO(&fds);
        FD_SET(udev_fd, &fds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(select_fd, &fds, NULL, NULL, &tv);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(udev_fd, &fds))
        {
            /* Make the call to receive the device.
               select() ensured that this will not block. */
            auto dev = udev_monitor_receive_device(mon);
            if (dev)
            {
                if (strcmp(udev_device_get_devnode(dev), device.get_identifier().c_str()) == 0)
                {
                    if (strcmp(udev_device_get_action(dev), "remove") == 0)
                    {
                        SPDLOG_ERROR("Lost device! {}", device.get_name().c_str());
                        this->lost_device();
                        break;
                    }
                    else
                    {
                        SPDLOG_WARN("Received an event for device: '{}' This should not happen.",
                                    udev_device_get_action(dev));
                    }
                }

                udev_device_unref(dev);
            }
            else
            {
                SPDLOG_ERROR("No Device from udev_monitor_receive_device. An error occured.");
            }
        }
    }

    udev_monitor_unref(mon);
    udev_unref(udev);
}
