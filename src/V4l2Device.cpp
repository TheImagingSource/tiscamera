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
#include "logging.h"
#include "utils.h"
#include "v4l2_utils.h"
#include "PropertyGeneration.h"
#include "Error.h"
#include <errno.h>

#include <algorithm>
#include <unistd.h>
#include <fcntl.h>              /* O_RDWR O_NONBLOCK */
#include <sys/mman.h>           /* mmap PROT_READ*/
#include <linux/videodev2.h>
#include <cstring>              /* memcpy*/

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
        setError(Error("No such property.", ENOENT));
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
            setError(Error("Emulated property not implemented", ENOENT));
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
        setError(Error(s, ENOENT));
        return false;
    }

    p.set_struct(desc->prop->get_struct());

    // TODO: ask device for current value
    return false;
}


V4l2Device::V4l2Device (const DeviceInfo& device_desc)
    : device(device_desc), emulate_bayer(false), emulated_fourcc(0),
      property_handler(nullptr), is_stream_on(false)
{

    if ((fd = open(device.get_info().identifier, O_RDWR /* required */ | O_NONBLOCK, 0)) == -1)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to open device \'%s\'.", device.get_info().identifier);
        throw std::runtime_error("Failed opening device.");
    }

    property_handler = std::make_shared<V4L2PropertyHandler>(this);

    determine_active_video_format();

    this->index_all_controls(property_handler);
    this->index_formats();
}


V4l2Device::~V4l2Device ()
{
    if (is_stream_on)
        stop_stream();

    if (this->fd != -1)
    {
        close(fd);
        fd = -1;
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
        setError(Error("Unable to set format", errno));
        return false;
    }

    /* framerate */

    if (!set_framerate(new_format.get_framerate()))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set framerate to %f", new_format.get_framerate());
        setError(Error("Unable to set framerate", errno));
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
        setError(Error("Unable to find corresponding framerate settings.", EIO));
        return false;
    }


    // TODO what about range framerates?
    struct v4l2_streamparm parm;

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    parm.parm.capture.timeperframe.numerator = fps->numerator;
    parm.parm.capture.timeperframe.denominator = fps->denominator;

    tcam_log(TCAM_LOG_DEBUG, "Setting framerate to '%f' =  %d / %d", framerate,
             parm.parm.capture.timeperframe.denominator,
             parm.parm.capture.timeperframe.numerator);

    int ret = tcam_xioctl(fd, VIDIOC_S_PARM, &parm);

    if (ret < 0)
    {
        tcam_log (TCAM_LOG_ERROR, "Failed to set frame rate\n");
        return false;
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

    // if (listener.expired())
    // {
    // tcam_log(TCAM_LOG_ERROR,"WAIT WHAT");
    // }
    // else
    // {
    // tcam_log(TCAM_LOG_ERROR,"VALID LISTENER");

    // }

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

    // if (listener.expired())
    // {
    // tcam_log(TCAM_LOG_ERROR, "Expired listener.");
    // return false;
    // }

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
            setError(Error(s, errno));
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
        return false;
    }

    if (work_thread.joinable())
        work_thread.join();

    return true;
}


/*
 * in kernel versions > 3.15 uvcvideo does not correctly interpret bayer 8-bit
 * this function detects those cases and corrects all settings
 */
static bool checkForBayer (const struct v4l2_fmtdesc& fmtdesc, struct v4l2_fmtdesc& new_desc)
{

    new_desc = fmtdesc;
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

        VideoFormatDescription format(desc, rf);
        this->available_videoformats.push_back(format);

        tcam_log(TCAM_LOG_DEBUG, "Found format: %s", fourcc2description(format.getFourcc()));

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

        setError(Error("VIDIOC_G_FMT failed", EIO));
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
        property_description pd = { EMULATED_PROPERTY, p};
        tcam_log(TCAM_LOG_DEBUG, "Adding '%s' to property list", p->get_name().c_str());
        property_handler->properties.push_back(pd);
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

    auto p = createProperty(fd, qctrl, &ext_ctrl, property_handler);

    struct property_description desc;

    desc.id = qctrl->id;
    desc.prop = p;

    static std::vector<TCAM_PROPERTY_ID> special_controls = {TCAM_PROPERTY_BINNING};

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

    if (type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        ctrl.value = (std::static_pointer_cast<PropertyInteger>(prop_desc.prop))->get_value();
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
        // tcam_log(TCAM_LOG_ERROR,
        // "Changed ctrl %s to value %d.",
        // prop_desc.prop->get_name().c_str(),
        // ctrl.value);
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
                    setError(Error("Unable to retrieve image. Select threw error.", errno));
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
            }

            ushort failure_counter = 0;
            bool ret_value = get_frame();
            if (ret_value)
            {
                failure_counter = 0;
                break;
            }
            else
            {
                failure_counter++;
                if (failure_counter >= 3)
                {
                    tcam_log(TCAM_LOG_ERROR, "Unable to retrieve buffer");
                    setError(Error("Unable to retrieve buffer.", errno));
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
                setError(Error("Unable to requeue buffer. ioctl error", errno));
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
        setError(Error("Unable to dequeue buffer.", errno));
        return false;
    }

    // v4l2 timestamps contain seconds and microseconds
    // here they are converted to nanoseconds
    statistics.capture_time_ns = (buf.timestamp.tv_sec * 1000 * 1000 * 1000) + (buf.timestamp.tv_usec * 1000);
    statistics.frame_count++;
    buffers.at(buf.index).buffer->set_statistics(statistics);

    buffers.at(buf.index).is_queued = false;

    listener->push_image(buffers.at(buf.index).buffer);

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
        setError(Error("Number of used buffers has to be >= 2", EINVAL));
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
            setError(Error("Unable to query v4l2_buffer ", EIO));
            return;
        }

        struct tcam_image_buffer buffer = {};

        buffer.length = active_video_format.get_required_buffer_size();
        buffer.pData =
            (unsigned char*) mmap( NULL, /* start anywhere */
                                   buf.length,
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
            tcam_log(TCAM_LOG_ERROR, "MMAP failed for buffer %d. Aborting.", n_buffers);
            // TODO: errno
            return;
        }

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
            auto buf = buffers.at(i).buffer->getImageBuffer();

            buf.pData = nullptr;
            buf.length = 0;

            buffers.at(i).buffer->set_image_buffer(buf);
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
