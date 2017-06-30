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
#include "format.h"
#include "logging.h"
#include "utils.h"
#include "v4l2_utils.h"
#include "PropertyGeneration.h"
#include <errno.h>
#include "v4l2_uvc_identifier.h"
#include "dfk73.h"

#include <algorithm>
#include <unistd.h>
#include <fcntl.h>              /* O_RDWR O_NONBLOCK */
#include <sys/mman.h>           /* mmap PROT_READ*/
#include <linux/videodev2.h>
#include <cstring>              /* memcpy*/
#include <libudev.h>

using namespace tcam;


V4l2Device::V4L2PropertyHandler::V4L2PropertyHandler (V4l2Device* dev)
    : device(dev)
{}


std::vector<std::shared_ptr<Property>> V4l2Device::V4L2PropertyHandler::create_property_vector ()
{
    std::vector<std::shared_ptr<Property>> props;

    for ( const auto& p : properties )
    {
        props.push_back(p.prop);
    }

    return props;
}


bool V4l2Device::V4L2PropertyHandler::set_property (const Property& new_property)
{
    auto f = [&new_property] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(new_property.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to find Property \"%s\"", new_property.get_name().c_str());
        return false;
    }

    if (desc->id == EMULATED_PROPERTY)
    {
        if (new_property.get_ID() == TCAM_PROPERTY_OFFSET_AUTO)
        {
            auto props = create_property_vector();
            return handle_auto_center(new_property,
                                      props,
                                      device->get_sensor_size(),
                                      device->active_video_format.get_size());
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "Emulated property not implemented \"%s\"", new_property.get_name().c_str());
            return false;
        }
    }
    else
    {
        desc->prop->set_struct(new_property.get_struct());

        if (device->changeV4L2Control(*desc))
        {
            return true;
        }
    }
    return false;
}


bool V4l2Device::V4L2PropertyHandler::get_property (Property& p)
{
    auto f = [&p] (const property_description& d)
        {
            return ((*d.prop).get_name().compare(p.get_name()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        std::string s = "Unable to find Property \"" + p.get_name() + "\"";
        tcam_log(TCAM_LOG_ERROR, "%s", s.c_str());
        return false;
    }

    device->updateV4L2Property(*desc);

    p.set_struct(desc->prop->get_struct());

    // TODO: ask device for current value
    return false;
}


V4l2Device::V4L2FormatHandler::V4L2FormatHandler (V4l2Device* dev)
    : device(dev)
{}


std::vector<double> V4l2Device::V4L2FormatHandler::get_framerates(const struct tcam_image_size& s, int pixelformat)
{
    std::vector<double> ret;

    return ret;
}


static const unsigned char lost_countdown_default = 5;


V4l2Device::V4l2Device (const DeviceInfo& device_desc)
    : emulate_bayer(false), emulated_fourcc(0),
      property_handler(nullptr),
      is_stream_on(false),
      lost_countdown(lost_countdown_default),
      stop_all(false),
      abort_all(false),
      device_is_lost(false)
{
    device = device_desc;

    udev_monitor = std::thread(&V4l2Device::monitor_v4l2_device, this);
    udev_monitor.detach();

    if ((fd = open(device.get_info().identifier, O_RDWR /* required */ | O_NONBLOCK, 0)) == -1)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to open device \'%s\'.", device.get_info().identifier);
        throw std::runtime_error("Failed opening device.");
    }

    property_handler = std::make_shared<V4L2PropertyHandler>(this);
    format_handler = std::make_shared<V4L2FormatHandler>(this);

    determine_active_video_format();

    this->index_all_controls(property_handler);
    this->index_formats();
}


V4l2Device::~V4l2Device ()
{
    if (is_stream_on)
        stop_stream();

    this->stop_all = true;
    this->abort_all = true;

    this->cv.notify_all();

    if (this->fd != -1)
    {
        close(fd);
        fd = -1;
    }

    if (work_thread.joinable())
    {
        work_thread.join();
    }

    if (udev_monitor.joinable())
    {
        udev_monitor.join();
    }

    if (notification_thread.joinable())
    {
        notification_thread.join();
    }

}


DeviceInfo V4l2Device::get_device_description () const
{
    return device;
}


std::vector<std::shared_ptr<Property>> V4l2Device::getProperties ()
{
    return property_handler->create_property_vector();
}


bool V4l2Device::set_property (const Property& new_property)
{
    tcam_log(TCAM_LOG_INFO, "Setting property \"%s\"", new_property.get_name().c_str());

    return property_handler->set_property(new_property);
}


bool V4l2Device::get_property (Property& p)
{

    // TODO: make properties updateable

    return property_handler->get_property(p);
}


bool V4l2Device::set_video_format (const VideoFormat& new_format)
{
    if (is_stream_on == true)
    {
        tcam_log(TCAM_LOG_ERROR, "Device is streaming.");
        return false;
    }

    tcam_log(TCAM_LOG_DEBUG, "Requested format change to '%s' %x", new_format.to_string().c_str(), new_format.get_fourcc());

    // if (!validate_video_format(new_format))
    // {
    // tcam_log(TCAM_LOG_ERROR, "Not a valid format.");
    // return false;
    // }


    uint32_t fourcc  = new_format.get_fourcc();

    // use greyscale for camera interaction
    if (emulate_bayer)
    {
        // TODO: correctly handle bit deptht
        emulated_fourcc = fourcc;
        fourcc = FOURCC_Y800;
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

    int ret = tcam_xioctl(this->fd, VIDIOC_S_FMT, &fmt);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Error while setting format '%s'", strerror(errno));
        return false;
    }

    /* framerate */

    if (!set_framerate(new_format.get_framerate()))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set framerate to %f", new_format.get_framerate());
        return false;
    }

    /* validation */

    determine_active_video_format();
    tcam_log(TCAM_LOG_DEBUG, "Active format is: '%s'",
             active_video_format.to_string().c_str() );

    return true;
}


bool V4l2Device::validate_video_format (const VideoFormat& format) const
{
    return false;
}


VideoFormat V4l2Device::get_active_video_format () const
{
    return active_video_format;
}


std::vector<VideoFormatDescription> V4l2Device::get_available_video_formats ()
{
    tcam_log(TCAM_LOG_DEBUG, "Returning %zu formats.", available_videoformats.size());
    return available_videoformats;
}


bool V4l2Device::set_framerate (double framerate)
{
    if (is_stream_on == true)
    {
        tcam_log(TCAM_LOG_ERROR, "Device is streaming.");
        return false;
    }
    // auto fps = std::find_if(framerate_conversions.begin(),
    // framerate_conversions.end(),
    // [&framerate] (const framerate_conv& f) {tcam_log(TCAM_LOG_ERROR,"%f", f.fps);
    // return (framerate == f.fps);});

    // if (fps == framerate_conversions.end())
    // {
    // tcam_log(TCAM_LOG_ERROR,"unable to find corresponding framerate settings.");
    // return false;
    // }

    std::vector<double> vec;

    for (auto& d : framerate_conversions)
    {
        vec.push_back(d.fps);
    }

    std::sort(vec.begin(), vec.end());

    auto iter_low = std::lower_bound(vec.begin(), vec.end(), framerate);

    if (iter_low == vec.end())
    {
        tcam_log(TCAM_LOG_ERROR, "No framerates available");
        return false;
    }

    framerate = *iter_low;

    auto f =[&framerate] (const framerate_conv& f)
        {
            return compare_double(framerate, f.fps);
        };

    auto fps = std::find_if(framerate_conversions.begin(),
                            framerate_conversions.end(),
                            f);



    if (fps == framerate_conversions.end())
    {
        tcam_log(TCAM_LOG_ERROR,"unable to find corresponding framerate settings.");
        return false;
    }

    // TODO what about range framerates?
    struct v4l2_streamparm parm;

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

        for (frmival.index = 0; tcam_xioctl( fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival ) >= 0; frmival.index++)
        {
            tcam_log(TCAM_LOG_DEBUG,
                     "F: %s %dx%d @ %d/%d",
                     fourcc2description(frmival.pixel_format),
                     frmival.width, frmival.height,
                     frmival.discrete.numerator, frmival.discrete.denominator);
            // Workaround for erratic frame rate handling of dfk73uc cameras:
            // Always set the highest possible frame rate for the current
            // video format via the UVC control. Then set the sensor clock
            // divider register to achieve the actual frame rate.
            if (frmival.type != V4L2_FRMIVAL_TYPE_DISCRETE)
            {
                tcam_log(TCAM_LOG_ERROR, "Got invalid frame interval from camera");
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
            if ((frmival.discrete.numerator == fps->numerator) &&
                (frmival.discrete.denominator == fps->denominator))
            {
                fi_index = frmival.index;
                // fi_index = 1;
                break;
            }
        }
        tcam_log(TCAM_LOG_DEBUG, "Setting framerate to '%f' =  %d / %d and frame rate divisor to %d",
                 framerate,
                 parm.parm.capture.timeperframe.denominator,
                 parm.parm.capture.timeperframe.numerator,
                 fi_index);

        ret = tcam_xioctl(fd, VIDIOC_S_PARM, &parm);
        if (ret < 0)
        {
            tcam_log(TCAM_LOG_ERROR, "Failed to set frame rate");
            return false;
        }

        errno = 0;
        ret = dfk73_v4l2_set_framerate_index(this->fd, fi_index);

        if (ret != 0)
        {
            tcam_log(TCAM_LOG_ERROR, "dfk73: Failed to set frame rate index, %s", strerror(errno));
            return false;
        }
    }
    else
    {
        parm.parm.capture.timeperframe.numerator = fps->numerator;
        parm.parm.capture.timeperframe.denominator = fps->denominator;
        tcam_log(TCAM_LOG_DEBUG, "Setting framerate to '%f' =  %d / %d",
                 framerate,
                 parm.parm.capture.timeperframe.denominator,
                 parm.parm.capture.timeperframe.numerator);

        int ret = tcam_xioctl(fd, VIDIOC_S_PARM, &parm);

        if (ret < 0)
        {
            tcam_log(TCAM_LOG_ERROR, "Failed to set frame rate\n");
            return false;
        }
    }

    return true;
}


double V4l2Device::get_framerate ()
{
    // TODO what about range framerates?
    struct v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = tcam_xioctl( fd, VIDIOC_G_PARM, &parm );

    if (ret < 0)
    {
        tcam_log (TCAM_LOG_ERROR, "Failed to get frame rate\n");
        return 0.0;
    }

    tcam_log(TCAM_LOG_INFO, "Current framerate is %d / %d fps",
             parm.parm.capture.timeperframe.denominator,
             parm.parm.capture.timeperframe.numerator);

    return (double)parm.parm.capture.timeperframe.denominator / (double)parm.parm.capture.timeperframe.numerator;
}


bool V4l2Device::set_sink (std::shared_ptr<SinkInterface> sink)
{
    if (is_stream_on)
    {
        return false;
    }

    this->listener = sink;

    return true;
}


bool V4l2Device::initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>> b)
{
    if (is_stream_on)
    {
        tcam_log(TCAM_LOG_ERROR, "Stream running.");

        return false;
    }

    this->buffers.clear();
    this->buffers.reserve(b.size());

    for (unsigned int i = 0; i < b.size(); ++i)
    {
        buffer_info info = {b.at(i), false};

        this->buffers.push_back(info);
    }

    init_mmap_buffers();

    return true;
}


bool V4l2Device::release_buffers ()
{
    if (is_stream_on)
    {
        return false;
    }

    free_mmap_buffers();

    buffers.clear();
    return true;
}


bool V4l2Device::start_stream ()
{
    enum v4l2_buf_type type;

    tcam_log(TCAM_LOG_DEBUG, "Will use %d buffers", buffers.size());

    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == tcam_xioctl(fd, VIDIOC_QBUF, &buf))
        {
            std::string s = "Unable to queue v4l2_buffer 'VIDIOC_QBUF'";
            tcam_log(TCAM_LOG_ERROR, s.c_str());
            return false;
        }
        else
            tcam_log(TCAM_LOG_DEBUG, "Successfully queued v4l2_buffer");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == tcam_xioctl(fd, VIDIOC_STREAMON, &type))
    {
        // TODO: error
        tcam_log(TCAM_LOG_ERROR, "Unable to set ioctl VIDIOC_STREAMON %d", errno);
        return false;
    }

    statistics = {};

    is_stream_on = true;

    if (!this->notification_thread.joinable())
    {
        this->notification_thread = std::thread(&V4l2Device::notification_loop, this);
    }

    tcam_log(TCAM_LOG_INFO, "Starting stream in work thread.");
    this->work_thread = std::thread(&V4l2Device::stream, this);

    return true;
}


bool V4l2Device::stop_stream ()
{
    is_stream_on = false;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = tcam_xioctl(fd, VIDIOC_STREAMOFF, &type);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set ioctl VIDIOC_STREAMOFF %d", errno);
        return false;
    }

    if (work_thread.joinable())
    {
        try
        {
            work_thread.join();
        }
        catch (const std::runtime_error& e)
        {
            tcam_log(TCAM_LOG_ERROR, "%s", e.what());
        }
    }
    tcam_log(TCAM_LOG_DEBUG, "Stopped stream");

    this->abort_all = true;

    return true;
}


void V4l2Device::notification_loop ()
{

    while(this->is_stream_on)
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        this->cv.wait(lck);

        if (this->abort_all)
        {
            break;
        }

        if (this->device_is_lost)
        {
            tcam_log(TCAM_LOG_DEBUG, "notifying callbacks about lost device");
            this->lost_device();

        }
    }
}


void V4l2Device::lost_device ()
{
    this->notify_device_lost();
}


/*
 * in kernel versions < 3.15 uvcvideo does not correctly interpret bayer 8-bit
 * this function detects those cases and corrects all settings
 */
static bool checkForBayer (const struct v4l2_fmtdesc& fmtdesc, struct v4l2_fmtdesc& new_desc)
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
        memcpy(new_desc.description, "BayerGR8", sizeof(new_desc.description));
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "42474752-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_BGGR8;
        new_desc.pixelformat = FOURCC_RGGB8;
        memcpy(new_desc.description, "BayerRG8", sizeof(new_desc.description));
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "31384142-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_RGGB8;
        new_desc.pixelformat = FOURCC_BGGR8;
        memcpy(new_desc.description, "BayerBG8", sizeof(new_desc.description));
        return true;
    }
    else if (strcmp((const char*)fmtdesc.description, "47524247-0000-0010-8000-00aa003") == 0)
    {
        // new_desc.pixelformat = FOURCC_GRBG8;
        new_desc.pixelformat = FOURCC_GBRG8;
        memcpy(new_desc.description, "BayerGB8", sizeof(new_desc.description));
        return true;
    }

    return false;
}


void V4l2Device::index_formats ()
{
    struct v4l2_fmtdesc fmtdesc = {};
    struct v4l2_frmsizeenum frms = {};

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (fmtdesc.index = 0; ! tcam_xioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc); fmtdesc.index ++)
    {
        struct tcam_video_format_description desc = {};

        // TODO: Assure format descriptions are always consistent
        struct v4l2_fmtdesc new_desc = {};
        emulate_bayer = checkForBayer(fmtdesc, new_desc);

        // internal fourcc definitions are identical with v4l2
        desc.fourcc = new_desc.pixelformat;
        memcpy (desc.description, new_desc.description, 256);
        frms.pixel_format = fmtdesc.pixelformat;

        std::vector<struct framerate_mapping> rf;

        for (frms.index = 0; ! tcam_xioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frms); frms.index++)
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
                tcam_log(TCAM_LOG_ERROR, "Encountered unknown V4L2_FRMSIZE_TYPE");
            }
        }

        desc.resolution_count = rf.size();

        // algorithms, etc. use Y800 as an identifier.
        // declare format as such.
        if (desc.fourcc == V4L2_PIX_FMT_GREY) // equals FOURCC_GREY
        {
            desc.fourcc = FOURCC_Y800;
        }

        // VideoFormatDescription format(format_handler, desc, rf);
        VideoFormatDescription format(nullptr, desc, rf);
        this->available_videoformats.push_back(format);

        tcam_log(TCAM_LOG_DEBUG, "Found format: %s", fourcc2description(format.get_fourcc()));

    }

}


std::vector<double> V4l2Device::index_framerates (const struct v4l2_frmsizeenum& frms)
{
    struct v4l2_frmivalenum frmival = {};

    frmival.pixel_format = frms.pixel_format;
    frmival.width = frms.discrete.width;
    frmival.height = frms.discrete.height;

    std::vector<double> f;

    for (frmival.index = 0; tcam_xioctl( fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival ) >= 0; frmival.index++)
    {
        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {

            // v4l2 lists frame rates as fractions (number of seconds / frames (e.g. 1/30))
            // we however use framerates as fps (e.g. 30/1)
            // therefor we have to switch numerator and denominator

            double frac = (double)frmival.discrete.denominator/frmival.discrete.numerator;
            f.push_back(frac);

            framerate_conv c = {frac, frmival.discrete.numerator, frmival.discrete.denominator};
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

void V4l2Device::determine_active_video_format ()
{

    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = tcam_xioctl(this->fd, VIDIOC_G_FMT, &fmt);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Error while querying video format");

        return;
    }

    // TODO what about range framerates?
    struct v4l2_streamparm parm = {};

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = tcam_xioctl( fd, VIDIOC_G_PARM, &parm );

    if (ret < 0)
    {

        tcam_log(TCAM_LOG_ERROR, "Failed to set frame rate");
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

    this->active_video_format = VideoFormat(format);
}


void V4l2Device::create_emulated_properties ()
{
    auto tmp_props = generate_simulated_properties(property_handler->create_property_vector(),
                                                   property_handler);

    for (auto& p : tmp_props)
    {
        property_description pd = { EMULATED_PROPERTY, 0, p};
        tcam_log(TCAM_LOG_DEBUG, "Adding '%s' to property list", p->get_name().c_str());
        property_handler->properties.push_back(pd);
    }
}


void V4l2Device::sort_properties ()
{
    if (property_handler->properties.empty())
    {
        return;
    }

    TCAM_PROPERTY_ID id;
    auto search_func = [&id] (const property_description& desc)
        {
            if (desc.id == id)
            {
                return true;
            }
            return false;
        };

    // ensure only one exposure interface is published
    id = TCAM_V4L2_EXPOSURE_TIME_US;
    auto exp_prop = std::find_if(property_handler->properties.begin(),
                                 property_handler->properties.end(),
                                 search_func);

    if (exp_prop != property_handler->properties.end())
    {
        // exposure time us is similar to the interface we want to publish
        // we therefor now search for all other exposure properties and move them
        // to the special section in case we need them

        auto p = property_handler->properties.begin();
        while (p != property_handler->properties.end())
        {
            if (p->id != TCAM_V4L2_EXPOSURE_TIME_US
                && p->prop->get_ID() == TCAM_PROPERTY_EXPOSURE)
            {
                property_handler->special_properties.push_back(*p);
                p = property_handler->properties.erase(p);
            }
            p++;
        }
    }
}


std::shared_ptr<Property> V4l2Device::apply_conversion_factor (std::shared_ptr<Property> prop,
                                                               const double factor)
{

    auto s = prop->get_struct();

    if (s.type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        s.value.i.min *= factor;
        s.value.i.max *= factor;
        s.value.i.step *= factor;
        s.value.i.value *= factor;
        s.value.i.default_value *= factor;


        return std::make_shared<Property>(PropertyInteger(property_handler, s, prop->get_value_type()));
    }
    else if (s.type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        s.value.d.min *= factor;
        s.value.d.max *= factor;
        s.value.d.step *= factor;
        s.value.d.value *= factor;
        s.value.d.default_value *= factor;

        return std::make_shared<Property>(PropertyDouble(property_handler, s, prop->get_value_type()));
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "Trying to apply conversion factor to property that does not represent numbers!");
        return nullptr;
    }
}


void V4l2Device::create_conversion_factors ()
{
    if (property_handler->properties.empty())
    {
        return;
    }

    TCAM_PROPERTY_ID id;
    auto search_func = [&id] (const property_description& desc)
        {
            if (desc.prop->get_ID() == id)
            {
                return true;
            }
            return false;
        };

    id = TCAM_PROPERTY_EXPOSURE;
    auto exposure = std::find_if(property_handler->properties.begin(),
                                 property_handler->properties.end(),
                                 search_func);

    if (exposure != property_handler->properties.end())
    {
        if (exposure->id == TCAM_V4L2_EXPOSURE_TIME_US)
        {
            // do nothing already the correct unit
            exposure->conversion_factor = 0.0;
        }
        else
        {
            // we have exposure_absolute which is in 100µs
            exposure->conversion_factor = 100.0;
            auto new_one = apply_conversion_factor(exposure->prop, exposure->conversion_factor);

            if (new_one != nullptr)
            {
                exposure->prop = new_one;
            }
        }
    }
}


void V4l2Device::index_all_controls (std::shared_ptr<PropertyImpl> impl)
{
    struct v4l2_queryctrl qctrl = { V4L2_CTRL_FLAG_NEXT_CTRL };

    while (tcam_xioctl(this->fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        index_control(&qctrl, impl);
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    // sort out duplicated interfaces
    sort_properties();
    // create conversion factors so that properties allways use the same units
    create_conversion_factors();
    // create library only properties
    create_emulated_properties();
}


int V4l2Device::index_control (struct v4l2_queryctrl* qctrl, std::shared_ptr<PropertyImpl> impl)
{

    if (qctrl->flags & V4L2_CTRL_FLAG_DISABLED)
    {
        return 1;
    }

    if (qctrl->type == V4L2_CTRL_TYPE_CTRL_CLASS)
    {
        // ignore unneccesary controls descriptions such as control "groups"
        // tcam_log(TCAM_LOG_DEBUG, "/n%s/n", qctrl->name);
        return 1;
    }
    struct v4l2_ext_control ext_ctrl = {};
    struct v4l2_ext_controls ctrls = {};

    ext_ctrl.id = qctrl->id;
    ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(qctrl->id);
    ctrls.count = 1;
    ctrls.controls = &ext_ctrl;

    if (V4L2_CTRL_ID2CLASS(qctrl->id) != V4L2_CTRL_CLASS_USER &&
        qctrl->id < V4L2_CID_PRIVATE_BASE)
    {
        if (qctrl->type == V4L2_CTRL_TYPE_STRING)
        {
            ext_ctrl.size = qctrl->maximum + 1;
            ext_ctrl.string = (char *)malloc(ext_ctrl.size);
            ext_ctrl.string[0] = 0;
        }

        if (qctrl->flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        {
            tcam_log(TCAM_LOG_INFO, "Encountered write only control.");
        }

        else if (tcam_xioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls))
        {
            tcam_log(TCAM_LOG_ERROR, "Errno %d getting ext_ctrl %s", errno, qctrl->name);
            return -1;
        }
    }
    else
    {
        struct v4l2_control ctrl = {};

        ctrl.id = qctrl->id;
        if (tcam_xioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
            tcam_log(TCAM_LOG_ERROR, "error %d getting ctrl %s", errno, qctrl->name);
            return -1;
        }
        ext_ctrl.value = ctrl.value;
    }

    auto p = create_property(fd, qctrl, &ext_ctrl, property_handler);

    if (p == nullptr)
    {
        tcam_log(TCAM_LOG_ERROR, "Property '%s' is null", qctrl->name);
        return -1;
    }

    struct property_description desc;

    desc.id = qctrl->id;
    desc.conversion_factor = 0.0;
    desc.prop = p;

    static std::vector<TCAM_PROPERTY_ID> special_controls = {};

    if (std::find(special_controls.begin(), special_controls.end(), p->get_ID()) != special_controls.end())
    {
        property_handler->special_properties.push_back(desc);
    }
    else
    {
        property_handler->properties.push_back(desc);
    }

    if (qctrl->type == V4L2_CTRL_TYPE_STRING)
    {
        free(ext_ctrl.string);
    }
    return 1;
}

bool save_value_of_control (const v4l2_control* ctrl,
                            tcam_device_property* cp,
                            double conversion_factor)
{
    switch (cp->type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (ctrl->value == 0)
            {
                cp->value.b.value = false;
            }
            else if (ctrl->value > 0)
            {
                cp->value.b.value = true;
            }
            else
            {
                tcam_log(TCAM_LOG_ERROR,
                         "Boolean '%s' has impossible value: %d Setting to false",
                         cp->name,
                         ctrl->value);
                cp->value.b.value = false;
            }
            return true;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            cp->value.i.value = ctrl->value;

            if (conversion_factor != 0.0)
            {
                cp->value.i.value *= conversion_factor;
            }
        }
        default:
        {
            return false;
        }
    }

}


void V4l2Device::updateV4L2Property (V4l2Device::property_description& desc)
{
    struct v4l2_control ctrl = {};
    ctrl.id = desc.id;

    if (desc.prop->get_type() == TCAM_PROPERTY_TYPE_BUTTON)
    {
        return;
    }

    if (tcam_xioctl(fd, VIDIOC_G_CTRL, &ctrl))
    {
        tcam_log(TCAM_LOG_ERROR, "Could not retrieve current value of %s. ioctl return '%s'", desc.prop->get_name().c_str(), strerror(errno));
    }

    auto cp = desc.prop->get_struct();

    save_value_of_control(&ctrl, &cp, desc.conversion_factor);
    tcam_log(TCAM_LOG_DEBUG, "Updated property %s to %d", desc.prop->get_name().c_str(), cp.value.i.value);

    desc.prop->set_struct(cp);
}


bool V4l2Device::changeV4L2Control (const property_description& prop_desc)
{

    TCAM_PROPERTY_TYPE type = prop_desc.prop->get_type();

    if (type == TCAM_PROPERTY_TYPE_STRING ||
        type == TCAM_PROPERTY_TYPE_UNKNOWN ||
        type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        tcam_log(TCAM_LOG_ERROR, "Property type not supported. Property changes not submitted to device.");
        return false;
    }

    struct v4l2_control ctrl = {};

    ctrl.id = prop_desc.id;

    if (type == TCAM_PROPERTY_TYPE_INTEGER || type == TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        ctrl.value = (std::static_pointer_cast<PropertyInteger>(prop_desc.prop))->get_value();
        if (prop_desc.conversion_factor != 0.0)
        {
            ctrl.value /= prop_desc.conversion_factor;
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        if ((std::static_pointer_cast<PropertyBoolean>(prop_desc.prop))->get_value())
        {
            ctrl.value = 1;
        }
        else
        {
            ctrl.value = 0;
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_BUTTON)
    {
        ctrl.value = 1;
    }

    int ret = tcam_xioctl(fd, VIDIOC_S_CTRL, &ctrl);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to submit property change for %s.", prop_desc.prop->get_name().c_str());
    }
    else
    {
        tcam_log(TCAM_LOG_DEBUG,
                 "Changed ctrl %s to value %d.",
                 prop_desc.prop->get_name().c_str(),
                 ctrl.value);
    }

    return true;
}


void V4l2Device::stream ()
{
    current_buffer = 0;

    while (this->is_stream_on)
    {
        if (current_buffer >= buffers.size())
        {
            current_buffer = 0;
        }
        else
        {
            current_buffer++;
        }
        for (;;)
        {
            if (!this->is_stream_on)
            {
                break;
            }

            fd_set fds;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            // TODO: should timeout be configurable?
            /* Timeout. */
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            /* Wait until device gives go */
            int ret = select(fd + 1, &fds, NULL, NULL, &tv);

            if (ret == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    /* error during select */
                    tcam_log(TCAM_LOG_ERROR, "Error during select");
                    return;
                }
            }

            auto is_trigger_mode_enabled = [this] ()
            {
                for (auto& p : this->property_handler->properties)
                {
                    if (p.prop->get_ID() == TCAM_PROPERTY_TRIGGER_MODE)
                    {
                        return static_cast<const PropertyBoolean*>(p.prop.get())->get_value();
                    }

                }
                return false;
            };

            /* timeout! */
            if (is_trigger_mode_enabled() && ret == 0)
            {
                continue;
            }


            if (ret == 0)
            {
                tcam_log(TCAM_LOG_ERROR, "Timeout while waiting for new image buffer.");
                statistics.frames_dropped++;

                this->lost_countdown--;
                if (this->lost_countdown <= 0)
                {
                    this->device_is_lost = true;
                    tcam_log(TCAM_LOG_ERROR, "Unable to retrieve buffer");
                    this->cv.notify_all();
                    break;
                }
                continue;
            }

            bool ret_value = get_frame();
            if (ret_value)
            {
                this->lost_countdown = lost_countdown_default;
                break;
            }
            else
            {
                this->lost_countdown--;
                if (this->lost_countdown <= 0)
                {
                    this->device_is_lost = true;

                    tcam_log(TCAM_LOG_ERROR, "Unable to retrieve buffer");
                    this->cv.notify_all();
                    break;
                }
            }
            /* receive frame since device is ready */
            // if ( == 0)
            // {
            // break;
            // }
            /* EAGAIN - continue select loop. */
        }
    }

}


bool V4l2Device::requeue_mmap_buffer ()
{
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        if (!buffers[i].buffer->is_locked() && !buffers[i].is_queued)
        {
            struct v4l2_buffer buf = {};

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            buf.index = i;

            // requeue buffer
            int ret = tcam_xioctl(fd, VIDIOC_QBUF, &buf);
            if (ret == -1)
            {
                return false;
            }
            buffers[i].is_queued = true;
        }
    }


    return true;
}


bool V4l2Device::get_frame ()
{
    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    int ret = tcam_xioctl(fd, VIDIOC_DQBUF, &buf);

    if (ret == -1)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to dequeue buffer.");
        return false;
    }

    buffers.at(buf.index).is_queued = false;

    if (buf.bytesused != (this->active_video_format.get_required_buffer_size()))
    {
        tcam_log(TCAM_LOG_ERROR, "Buffer has wrong size. Dropping...");
        if (!requeue_mmap_buffer())
        {
            return false;
        }
        return true;
    }

    // v4l2 timestamps contain seconds and microseconds
    // here they are converted to nanoseconds
    statistics.capture_time_ns = ((long long)buf.timestamp.tv_sec * 1000 * 1000 * 1000) + (buf.timestamp.tv_usec * 1000);
    statistics.frame_count++;
    buffers.at(buf.index).buffer->set_statistics(statistics);


    tcam_log(TCAM_LOG_DEBUG, "pushing new buffer");

    if (auto ptr = listener.lock())
    {
        ptr->push_image(buffers.at(buf.index).buffer);
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "ImageSink expired. Unable to deliver images.");
        requeue_mmap_buffer();
        return false;
    }

    if (!requeue_mmap_buffer())
    {
        return false;
    }

    return true;
}


// TODO: look into mmap with existing memory
void V4l2Device::init_mmap_buffers ()
{
    if (buffers.empty())
    {
        tcam_log(TCAM_LOG_ERROR, "Number of used buffers has to be >= 2");
        return;
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "Mmaping %d buffers", buffers.size());
    }

    struct v4l2_requestbuffers req = {};

    req.count = buffers.size();
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (tcam_xioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        // TODO: error
        return;
    }

    if (req.count < 2)
    {
        tcam_log(TCAM_LOG_ERROR, "Insufficient memory for memory mapping");
        // TODO: errno
        return;
    }

    for (unsigned int n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (tcam_xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            return;
        }

        struct tcam_image_buffer buffer = {};

        // buffer.length = active_video_format.get_required_buffer_size();
        buffer.length = buf.length;
        buffer.pData =
            /* use pre-allocated memory */
            (unsigned char*) mmap( NULL,
                                   buffer.length,
                                   PROT_READ | PROT_WRITE, /* required */
                                   MAP_SHARED, /* recommended */
                                   fd,
                                   buf.m.offset);

        buffer.format = active_video_format.get_struct();

        if (buffer.format.fourcc == mmioFOURCC('G', 'R', 'E', 'Y'))
        {
            buffer.format.fourcc = FOURCC_Y800;
        }

        if (emulate_bayer)
        {
            if (emulated_fourcc != 0)
            {
                buffer.format.fourcc = emulated_fourcc;
            }
        }

        buffer.pitch = active_video_format.get_pitch_size();
        if (buffer.pData == MAP_FAILED)
        {
            tcam_log(TCAM_LOG_ERROR, "MMAP failed for buffer %d. Aborting. %s", n_buffers, strerror(errno));
            // TODO: errno
            return;
        }

        tcam_log(TCAM_LOG_DEBUG, "mmap pointer %p %p", buffer.pData,
                 buffers.at(n_buffers).buffer->get_data());

        buffers.at(n_buffers).buffer->set_image_buffer(buffer);
        buffers.at(n_buffers).is_queued = true;
    }

}


static bool reqbufs_mmap (int fd, v4l2_requestbuffers &reqbuf, uint32_t buftype, int count)
{
	memset(&reqbuf, 0, sizeof (reqbuf));
	reqbuf.type = buftype;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = count;

	return tcam_xioctl(fd, VIDIOC_REQBUFS, &reqbuf) >= 0;
}


void V4l2Device::free_mmap_buffers ()
{
    if (buffers.empty())
        return;


    unsigned int i;

    for (i = 0; i < buffers.size(); ++i)
    {
        if (-1 == munmap(buffers.at(i).buffer->getImageBuffer().pData,
                         buffers.at(i).buffer->getImageBuffer().length))
        {
            // TODO: error

            return;
        }
        else
        {
            // auto buf = buffers.at(i).buffer->getImageBuffer();

            // buf.pData = nullptr;
            // buf.length = 0;

            // buffers.at(i).buffer->set_image_buffer(buf);
        }
    }

	uint32_t buftype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_requestbuffers reqbufs;

    reqbufs_mmap(fd, reqbufs, buftype, 1);  // videobuf workaround
    reqbufs_mmap(fd, reqbufs, buftype, 0);
}


tcam_image_size V4l2Device::get_sensor_size () const
{
    tcam_image_size size = {};
    for (const auto& f : available_videoformats)
    {
        for (const auto& r :f.get_resolutions())
        {
            if (r.max_size.width > size.width || r.max_size.height > size.width)
            {
                size = r.max_size;
            }
        }
    }

    return size;
}


void V4l2Device::monitor_v4l2_device ()
{
    auto udev = udev_new();
    /* Set up a monitor to monitor hidraw devices */
    auto mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "video4linux", NULL);
    udev_monitor_enable_receiving(mon);
    /* Get the file descriptor (fd) for the monitor.
       This fd will get passed to select() */
    int udev_fd = udev_monitor_get_fd(mon);

    /* This section will run continuously, calling usleep() at
       the end of each pass. This is to demonstrate how to use
       a udev_monitor in a non-blocking way. */
    while (!stop_all)
    {
        /* Set up the call to select(). In this case, select() will
           only operate on a single file descriptor, the one
           associated with our udev_monitor. Note that the timeval
           object is set to 0, which will cause select() to not
           block. */
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(udev_fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(udev_fd+1, &fds, NULL, NULL, &tv);

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
                        tcam_log(TCAM_LOG_ERROR, "Lost device! %s", device.get_name().c_str());
                        this->lost_device();
                        break;
                    }
                    else
                    {
                        tcam_log(TCAM_LOG_WARNING,
                                 "Received an event for device: '%s' This should not happen.",
                                 udev_device_get_action(dev));
                    }
                }

                udev_device_unref(dev);
            }
            else
            {
                tcam_log(TCAM_LOG_ERROR, "No Device from udev_monitor_receive_device. An error occured.");
            }
        }
        usleep(250*1000);
    }
}
