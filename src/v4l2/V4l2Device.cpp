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

#include "dfk73.h"
#include "format.h"
#include "img/fcc_to_string.h"
#include "logging.h"
#include "utils.h"
#include "v4l2_property_impl.h"
#include "v4l2_utils.h"

#include <algorithm>
#include <cstring> /* memcpy*/
#include <errno.h>
#include <fcntl.h> /* O_RDWR O_NONBLOCK */
#include <libudev.h>
#include <linux/videodev2.h>
#include <unistd.h>

using namespace tcam;


V4l2Device::V4L2FormatHandler::V4L2FormatHandler(V4l2Device* dev) : device(dev) {}


std::vector<double> V4l2Device::V4L2FormatHandler::get_framerates(const struct tcam_image_size& s
                                                                  __attribute__((unused)),
                                                                  int pixelformat
                                                                  __attribute__((unused)))
{
    std::vector<double> ret;

    return ret;
}


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

    if (pipe(udev_monitor_pipe) != 0)
    {
        SPDLOG_ERROR("Unable to create udev monitor pipe");
        throw std::runtime_error("Failed opening device.");
    }
    m_monitor_v4l2_thread = std::thread(&V4l2Device::monitor_v4l2_thread_func, this);

    m_format_handler = std::make_shared<V4L2FormatHandler>(this);

    determine_active_video_format();

    p_property_backend = std::make_shared<tcam::property::V4L2PropertyBackend>(m_fd);
    //this->index_all_controls(property_handler);
    this->index_controls();
    this->index_formats();
}


V4l2Device::~V4l2Device()
{
    if (m_is_stream_on)
    {
        stop_stream();
    }

    this->m_is_stream_on = false;

    this->m_stop_monitor_v4l2_thread = true;

    // signal the udev monitor to exit it's poll/select
    ssize_t write_ret = write(udev_monitor_pipe[1], "q", 1);
    if (write_ret != 1)
    {
        SPDLOG_ERROR("Error closing udev monitoring pipe. write returned '{}' errno: {}",
                     write_ret,
                     strerror(errno));
    }

    // close write pipe fd
    close(udev_monitor_pipe[1]);

    if (m_monitor_v4l2_thread.joinable())
    {
        m_monitor_v4l2_thread.join();
    }

    if (this->m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }

    if (m_work_thread.joinable())
    {
        m_work_thread.join();
    }

    if (m_notification_thread.joinable()) // join this just in case stop_stream was not called
    {
        m_notification_thread.join();
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
                 new_format.to_string().c_str(),
                 new_format.get_fourcc());

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

    uint32_t fourcc = new_format.get_fourcc();

    // use greyscale for camera interaction
    if (emulate_bayer)
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

    /* validation */

    determine_active_video_format();
    SPDLOG_DEBUG("Active format is: '{}'", m_active_video_format.to_string().c_str());

    return true;
}


bool V4l2Device::validate_video_format(const VideoFormat& format __attribute__((unused))) const
{
    return false;
}


VideoFormat V4l2Device::get_active_video_format() const
{
    return m_active_video_format;
}


std::vector<VideoFormatDescription> V4l2Device::get_available_video_formats()
{
    SPDLOG_DEBUG("Returning {} formats.", m_available_videoformats.size());
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

    auto f = [&framerate](const framerate_conv& _f) {
        return compare_double(framerate, _f.fps);
    };

    auto fps = std::find_if(framerate_conversions.begin(), framerate_conversions.end(), f);


    if (fps == framerate_conversions.end())
    {
        SPDLOG_ERROR("unable to find corresponding framerate settings.");
        return false;
    }

    struct v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (strcmp(this->device.get_info().additional_identifier, "8221") == 0)
    {
        int ret;
        int fi_index = 0;
        struct v4l2_frmivalenum frmival = {};

        VideoFormat fmt = this->get_active_video_format();

        frmival.pixel_format = fmt.get_fourcc();
        frmival.width = fmt.get_size().width;
        frmival.height = fmt.get_size().height;

        for (frmival.index = 0; tcam_xioctl(m_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0;
             frmival.index++)
        {
            SPDLOG_DEBUG("F: {} {}x{} @ {}/{}",
                         img::fcc_to_string(frmival.pixel_format),
                         frmival.width,
                         frmival.height,
                         frmival.discrete.numerator,
                         frmival.discrete.denominator);
            // Workaround for erratic frame rate handling of dfk73uc cameras:
            // Always set the highest possible frame rate for the current
            // video format via the UVC control. Then set the sensor clock
            // divider register to achieve the actual frame rate.
            if (frmival.type != V4L2_FRMIVAL_TYPE_DISCRETE)
            {
                SPDLOG_ERROR("Got invalid frame interval from camera");
                return false;
            }
            if (frmival.index == 0)
            {
                // Always set the first frame rate in the list. The
                // actual framerate is set by
                // dfk73_v4l2_set_framerate_index(...)
                parm.parm.capture.timeperframe.numerator = frmival.discrete.numerator;
                parm.parm.capture.timeperframe.denominator = frmival.discrete.denominator;
            }
            if ((frmival.discrete.numerator == fps->numerator)
                && (frmival.discrete.denominator == fps->denominator))
            {
                fi_index = frmival.index;
                // fi_index = 1;
                break;
            }
        }
        SPDLOG_DEBUG("Setting framerate to '{}' =  {} / {} and frame rate divisor to {}",
                     framerate,
                     parm.parm.capture.timeperframe.denominator,
                     parm.parm.capture.timeperframe.numerator,
                     fi_index);

        ret = tcam_xioctl(m_fd, VIDIOC_S_PARM, &parm);
        if (ret < 0)
        {
            SPDLOG_ERROR("Failed to set frame rate");
            return false;
        }

        errno = 0;
        ret = dfk73_v4l2_set_framerate_index(this->m_fd, fi_index);

        if (ret != 0)
        {
            SPDLOG_ERROR("dfk73: Failed to set frame rate index, {}", strerror(errno));
            return false;
        }
    }
    else
    {
        parm.parm.capture.timeperframe.numerator = fps->numerator;
        parm.parm.capture.timeperframe.denominator = fps->denominator;
        SPDLOG_DEBUG("Setting framerate to '{}' =  {} / {}",
                     framerate,
                     parm.parm.capture.timeperframe.denominator,
                     parm.parm.capture.timeperframe.numerator);

        int ret = tcam_xioctl(m_fd, VIDIOC_S_PARM, &parm);

        if (ret < 0)
        {
            SPDLOG_ERROR("Failed to set frame rate\n");
            return false;
        }
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


bool V4l2Device::set_sink(std::shared_ptr<SinkInterface> sink)
{
    if (m_is_stream_on)
    {
        return false;
    }

    this->m_listener = sink;

    return true;
}


bool V4l2Device::initialize_buffers(std::vector<std::shared_ptr<ImageBuffer>> b)
{
    if (m_is_stream_on)
    {
        SPDLOG_ERROR("Stream running.");
        return false;
    }

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

    m_buffers.clear();
    return true;
}


void V4l2Device::requeue_buffer(std::shared_ptr<ImageBuffer> buffer)
{
    for (unsigned int i = 0; i < m_buffers.size(); ++i)
    {

        auto& b = m_buffers.at(i);
        if (!b.is_queued && b.buffer == buffer)
        {
            struct v4l2_buffer buf = {};

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)b.buffer->get_data();
            buf.length = b.buffer->get_buffer_size();

            // requeue buffer
            int ret = tcam_xioctl(m_fd, VIDIOC_QBUF, &buf);
            if (ret == -1)
            {
                SPDLOG_ERROR("Could not requeue buffer");
                return;
            }
            b.is_queued = true;
        }
    }
}


void V4l2Device::update_stream_timeout()
{
    for (const auto& p : m_properties)
    {
        if (p->get_name() == "ExposureTime")
        {
            auto val =std::dynamic_pointer_cast<tcam::property::IPropertyFloat>(p)->get_value();

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


bool V4l2Device::start_stream()
{
    init_userptr_buffers();

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == tcam_xioctl(m_fd, VIDIOC_STREAMON, &type))
    {
        SPDLOG_ERROR("Unable to set ioctl VIDIOC_STREAMON {}", errno);
        return false;
    }

    m_statistics = {};

    m_is_stream_on = true;

    update_stream_timeout();

    SPDLOG_INFO("Starting stream in work thread.");

    this->m_work_thread = std::thread(&V4l2Device::stream, this);

    return true;
}


bool V4l2Device::stop_stream()
{
    SPDLOG_DEBUG("Stopping stream");
    int ret = 0;

    if (m_is_stream_on)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = tcam_xioctl(m_fd, VIDIOC_STREAMOFF, &type);

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

    if (m_notification_thread.joinable())
    { // wait for possible device lost notification to end
        m_notification_thread.join();
    }

    SPDLOG_DEBUG("Stopped stream");

    if (ret < 0)
    {
        return false;
    }

    return true;
}


void V4l2Device::notify_device_lost_func()
{
    SPDLOG_INFO("notifying callbacks about lost device");
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
static bool checkForBayer(const struct v4l2_fmtdesc& fmtdesc, struct v4l2_fmtdesc& new_desc)
{

    new_desc = fmtdesc;
    // when v4l2 does not recognize a format fourcc it will
    // set the fourcc to 0 and pass the description string.
    // we compare the possible strings and correct the fourcc
    // for loater emulation of the correct pattern

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


void V4l2Device::index_formats()
{
    struct v4l2_fmtdesc fmtdesc = {};
    struct v4l2_frmsizeenum frms = {};

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (fmtdesc.index = 0; !tcam_xioctl(m_fd, VIDIOC_ENUM_FMT, &fmtdesc); fmtdesc.index++)
    {
        struct tcam_video_format_description desc = {};

        struct v4l2_fmtdesc new_desc = {};
        emulate_bayer = checkForBayer(fmtdesc, new_desc);

        // internal fourcc definitions are identical with v4l2
        desc.fourcc = new_desc.pixelformat;
        memcpy(desc.description, new_desc.description, sizeof(new_desc.description));
        frms.pixel_format = fmtdesc.pixelformat;

        std::vector<struct framerate_mapping> rf;

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

                res.framerate_count = f.size();
                res.type = TCAM_RESOLUTION_TYPE_FIXED;

                framerate_mapping r = { res, f };
                rf.push_back(r);
            }
            else
            {
                // TIS USB cameras do not have this kind of setting
                SPDLOG_ERROR("Encountered unknown V4L2_FRMSIZE_TYPE");
            }
        }

        desc.resolution_count = rf.size();

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

    struct v4l2_streamparm parm = {};

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
            }
            else
            {
                lost_countdown--;
            }
            waiting_period = m_stream_timeout_sec;
        }
        if (lost_countdown <= 0)
        {
            SPDLOG_WARN("Did not receive image for long time.");
            lost_countdown = lost_countdown_default;
        }
    }
}


bool V4l2Device::is_trigger_mode_enabled()
{
    for (auto& p : m_properties)
    {
        if (p->get_name() == "TriggerMode")
        {
            auto val =std::dynamic_pointer_cast<tcam::property::IPropertyEnum>(p)->get_value();
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
    buf.memory = V4L2_MEMORY_USERPTR;

    int ret = tcam_xioctl(m_fd, VIDIOC_DQBUF, &buf);

    if (ret == -1)
    {
        SPDLOG_TRACE("Unable to dequeue buffer.");
        return false;
    }

    m_buffers.at(buf.index).is_queued = false;

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

    // on initial startup all buffers are received once, but empty
    // this causes unneccessary error messages
    // filter those messages until we receive one valid image
    static bool already_received_valid_image;

    if (m_active_video_format.get_fourcc() != FOURCC_MJPG)
    {
        if (buf.bytesused != (this->m_active_video_format.get_required_buffer_size()))
        {
            if (already_received_valid_image)
            {
                SPDLOG_ERROR("Buffer has wrong size. Got: {} Expected: {} Dropping...",
                             buf.bytesused,
                             this->m_active_video_format.get_required_buffer_size());
            }
            requeue_buffer(m_buffers.at(buf.index).buffer);
            return true;
        }
    }
    already_received_valid_image = true;
    // v4l2 timestamps contain seconds and microseconds
    // here they are converted to nanoseconds
    m_statistics.capture_time_ns =
        ((long long)buf.timestamp.tv_sec * 1000 * 1000 * 1000) + (buf.timestamp.tv_usec * 1000);
    m_statistics.frame_count++;
    m_buffers.at(buf.index).buffer->set_statistics(m_statistics);

    auto desc = m_buffers.at(buf.index).buffer->getImageBuffer();
    desc.length = buf.bytesused;
    m_buffers.at(buf.index).buffer->set_image_buffer(desc);

    SPDLOG_TRACE("pushing new buffer");

    if (auto ptr = m_listener.lock())
    {
        ptr->push_image(m_buffers.at(buf.index).buffer);
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

    SPDLOG_DEBUG("Will use {} buffers", m_buffers.size());

    struct v4l2_requestbuffers req = {};

    req.count = m_buffers.size();
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == tcam_xioctl(m_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            SPDLOG_ERROR("{} does not support user pointer i/o", device.get_serial().c_str());
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
        buf.m.userptr = (unsigned long)m_buffers.at(i).buffer->get_data();
        buf.length = m_buffers.at(i).buffer->get_buffer_size();

        SPDLOG_TRACE("Queueing buffer({:x}) with length {}", m_buffers.at(i).buffer->get_data(), buf.length);

        if (-1 == tcam_xioctl(m_fd, VIDIOC_QBUF, &buf))
        {
            SPDLOG_ERROR("Unable to queue v4l2_buffer 'VIDIOC_QBUF' {}", strerror(errno));
            return;
        }
        else
        {
            SPDLOG_TRACE("Successfully queued v4l2_buffer");
            m_buffers.at(i).is_queued = true;
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
            if (r.max_size.width > size.width || r.max_size.height > size.width)
            {
                size = r.max_size;
            }
        }
    }

    return size;
}


void V4l2Device::monitor_v4l2_thread_func()
{
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
        int select_fd = (udev_fd > udev_monitor_pipe[0] ? udev_fd : udev_monitor_pipe[0]);
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(udev_fd, &fds);
        FD_SET(udev_monitor_pipe[1], &fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        ret = select(select_fd, &fds, NULL, NULL, &tv);

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

    /* close read pipe fd */
    close(udev_monitor_pipe[0]);

    udev_monitor_unref(mon);
    udev_unref(udev);
}
