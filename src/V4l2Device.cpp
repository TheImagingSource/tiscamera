
#include "V4l2Device.h"
#include "logging.h"
#include "utils.h"
#include "v4l2_utils.h"
#include "PropertyGeneration.h"
#include "Error.h"

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


bool V4l2Device::V4L2PropertyHandler::setProperty (const Property& new_property)
{
    auto f = [&new_property] (const property_description& d)
        {
            return ((*d.prop).getName().compare(new_property.getName()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        setError(Error("", 0));
        tcam_log(TCAM_LOG_ERROR, "Unable to find Property \"%s\"", new_property.getName().c_str());
        // TODO: failure description
        return false;
    }

    if (desc->id == EMULATED_PROPERTY)
    {
        if (new_property.getID() == TCAM_PROPERTY_OFFSET_AUTO)
        {
            auto props = create_property_vector();
            return handle_auto_center(new_property,
                                      props,
                                      device->get_sensor_size(),
                                      device->active_video_format.getSize());
        }
        else
        {
            setError(Error("Emulated property not implemented", ENOENT));
            tcam_log(TCAM_LOG_ERROR, "Emulated property not implemented \"%s\"", new_property.getName().c_str());
            return false;
        }
    }
    else
    {
        desc->prop->setStruct(new_property.getStruct());

        if (device->changeV4L2Control(*desc))
        {
            return true;
        }
    }
    return false;
}


bool V4l2Device::V4L2PropertyHandler::getProperty (Property& p)
{
    auto f = [&p] (const property_description& d)
        {
            return ((*d.prop).getName().compare(p.getName()) == 0);
        };

    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        std::string s = "Unable to find Property \"" + p.getName() + "\"";
        tcam_log(TCAM_LOG_ERROR, "%s", s.c_str());
        setError(Error(s, ENOENT));
        return false;
    }

    p.setStruct(desc->prop->getStruct());

    // TODO: ask device for current value
    return false;
}


V4l2Device::V4l2Device (const DeviceInfo& device_desc)
    : device(device_desc), emulate_bayer(false), emulated_fourcc(0),
      property_handler(nullptr), is_stream_on(false)
{

    if ((fd = open(device.getInfo().identifier, O_RDWR /* required */ | O_NONBLOCK, 0)) == -1)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to open device \'%s\'.", device.getInfo().identifier);
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


DeviceInfo V4l2Device::getDeviceDescription () const
{
    return device;
}


std::vector<std::shared_ptr<Property>> V4l2Device::getProperties ()
{
    return property_handler->create_property_vector();
}


bool V4l2Device::setProperty (const Property& new_property)
{
    return false;
}


bool V4l2Device::getProperty (Property& p)
{
    return false;
}


bool V4l2Device::setVideoFormat (const VideoFormat& new_format)
{
    if (is_stream_on == true)
    {
        tcam_log(TCAM_LOG_ERROR, "Device is streaming.");
        return false;
    }

    // if (!validateVideoFormat(new_format))
    // {
    // tcam_log(TCAM_LOG_ERROR, "Not a valid format.");
    // return false;
    // }


    uint32_t fourcc  = new_format.getFourcc();

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

    fmt.fmt.pix.width = new_format.getSize().width;
    fmt.fmt.pix.height = new_format.getSize().height;

    fmt.fmt.pix.pixelformat = fourcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    int ret = tcam_xioctl(this->fd, VIDIOC_S_FMT, &fmt);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Error while setting format '%s'", strerror(errno));
        setError(Error("Unable to set format", errno));
        return false;
    }

    // set framerate
    if (!setFramerate(new_format.getFramerate()))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set framerate to %f", new_format.getFramerate());
        // return false;
    }

    fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = tcam_xioctl(this->fd, VIDIOC_G_FMT, &fmt);

    if (ret < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "VIDIOC_G_FMT failed with '%s'", strerror(errno));
    }

    tcam_video_format f = {};

    f.fourcc = fmt.fmt.pix.pixelformat;
    f.width = fmt.fmt.pix.width;
    f.height = fmt.fmt.pix.height;
    f.binning = 0;
    f.framerate = getFramerate();


    if (f.fourcc == V4L2_PIX_FMT_GREY)
    {
        f.fourcc = FOURCC_Y800;
    }

    // copy format as local reference
    active_video_format = VideoFormat(f);
    tcam_log(TCAM_LOG_DEBUG, "Active format is: '%s'",
             active_video_format.toString().c_str() );

    return true;
}


bool V4l2Device::validateVideoFormat (const VideoFormat& format)
{

    for (const auto& f : available_videoformats)
    {
        if (f.isValidVideoFormat(format))
        {
            return true;
        }
    }
    return false;
}


VideoFormat V4l2Device::getActiveVideoFormat () const
{
    return active_video_format;
}


std::vector<VideoFormatDescription> V4l2Device::getAvailableVideoFormats ()
{
    return available_videoformats;
}


bool V4l2Device::setFramerate (double framerate)
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
             parm.parm.capture.timeperframe.numerator,
             parm.parm.capture.timeperframe.denominator);
    int ret = tcam_xioctl(fd, VIDIOC_S_PARM, &parm);

    if (ret < 0)
    {
        tcam_log (TCAM_LOG_ERROR, "Failed to set frame rate\n");
        return false;
    }

    return true;
}


double V4l2Device::getFramerate ()
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

    tcam_log(TCAM_LOG_INFO, "Set framerate to %d / %d fps",
             parm.parm.capture.timeperframe.numerator,
             parm.parm.capture.timeperframe.denominator);

    return (double)parm.parm.capture.timeperframe.denominator / (double)parm.parm.capture.timeperframe.numerator;
}


bool V4l2Device::setSink (std::shared_ptr<SinkInterface> sink)
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

    this->buffers = b;

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

    // VideoFormat v;

    // setVideoFormat(v);
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
            // TODO: error
            std::string s = "Unable to queue v4l2_buffer 'VIDIOC_QBUF'";
            tcam_log(TCAM_LOG_ERROR, s.c_str());
            setError(Error(s, EIO));
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
    bool wait_for_join = is_stream_on;

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
    else if (strcmp((const char*)fmtdesc.description, "52474742-0000-0010-8000-00aa003") == 0)
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

        std::vector<res_fps> rf;

        for (frms.index = 0; ! tcam_xioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frms); frms.index++)
        {
            if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                desc.min_size.width = frms.discrete.width;
                desc.max_size.width = frms.discrete.width;
                desc.min_size.height = frms.discrete.height;
                desc.max_size.height = frms.discrete.height;

                tcam_image_size s = { frms.discrete.width, frms.discrete.height };

                desc.framerate_type = TCAM_FRAMERATE_TYPE_FIXED;
                std::vector<double> f = index_framerates(frms);

                res_fps r = { s, f };
                rf.push_back(r);
            }
            else
            {
                // TIS USB cameras do not have this kind of setting
                tcam_log(TCAM_LOG_ERROR, "Encountered unknown V4L2_FRMSIZE_TYPE");
            }
        }

        // algorithms, etc. use Y800 as an identifier.
        // declare format as such.
        if (desc.fourcc == V4L2_PIX_FMT_GREY) // equals FOURCC_GREY
        {
            desc.fourcc = FOURCC_Y800;
        }


        std::vector<int> binnings;

        binnings.push_back(1); // default value; binning disabled

        auto binning_desc = std::find_if(property_handler->special_properties.begin(),
                                         property_handler->special_properties.end(),
                                         [] (const property_description& d)
                                         {
                                             if (d.prop->getID() == TCAM_PROPERTY_BINNING)
                                                 return true;
                                             return false;
                                         });

        if (binning_desc != property_handler->special_properties.end())
        {

            auto bp = (PropertyInteger&) *(binning_desc->prop);

            for (auto i = bp.getMin(); i <= bp.getMax(); i += bp.getStep())
            {
                if (i % 2 == 0)
                    binnings.push_back(i);
            }

        }
        else
        {
        }

        for (const auto& b : binnings)
        {
            desc.binning = b;

            VideoFormatDescription format(desc, rf);
            available_videoformats.push_back(format);
        }
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

        fprintf (stderr, "Failed to set frame rate\n");
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

    auto find_binning = [] (const property_description& d)
        {
            if (d.prop->getID() == TCAM_PROPERTY_BINNING)
            {
                return true;
            }
            return false;
        };

    auto bin_ptr = std::find_if(property_handler->special_properties.begin(),
                                property_handler->special_properties.end(),
                                find_binning);

    if (bin_ptr == property_handler->special_properties.end())
        format.binning = 1;
    else
    {
        format.binning = ((PropertyInteger&)*((*bin_ptr).prop)).getValue();
    }

    format.framerate = getFramerate();

    this->active_video_format = VideoFormat(format);
}


void V4l2Device::create_emulated_properties ()
{
    auto tmp_props = generate_simulated_properties(property_handler->create_property_vector(),
                                                   property_handler);

    for (auto& p : tmp_props)
    {
        property_description pd = { EMULATED_PROPERTY, p};
        tcam_log(TCAM_LOG_DEBUG, "Adding '%s' to property list", p->getName().c_str());
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

    if (std::find(special_controls.begin(), special_controls.end(), p->getID()) != special_controls.end())
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

    TCAM_PROPERTY_TYPE type = prop_desc.prop->getType();

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
        ctrl.value = (std::static_pointer_cast<PropertyInteger>(prop_desc.prop))->getValue();
    }
    else if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        if ((std::static_pointer_cast<PropertyBoolean>(prop_desc.prop))->getValue())
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
        tcam_log(TCAM_LOG_ERROR, "Unable to submit property change.");
    }
    else
    {
        // tcam_log(TCAM_LOG_ERROR,
        // "Changed ctrl %s to value %d.",
        // prop_desc.prop->getName().c_str(),
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
                    // TODO: errno
                    /* error during select */
                    tcam_log(TCAM_LOG_ERROR, "Error during select");
                    return;
                }
            }

            /* timeout! */ // TODO: check if trigger is enabled
            if (ret == 0)
            {
                tcam_log(TCAM_LOG_ERROR, "Timeout while waiting for new image buffer.");
                statistics.frames_dropped++;
            }

            if (get_frame())
            {
                break;
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
    buffers.at(buf.index)->setStatistics(statistics);

    listener->pushImage(buffers.at(buf.index));

    // requeue buffer
    ret = tcam_xioctl(fd, VIDIOC_QBUF, &buf);
    if (ret == -1)
    {
        setError(Error("Unable to requeue buffer. ioctl error", errno));
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

        buffer.length = active_video_format.getRequiredBufferSize();
        buffer.pData =
            (unsigned char*) mmap( NULL, /* start anywhere */
                                   buf.length,
                                   PROT_READ | PROT_WRITE, /* required */
                                   MAP_SHARED, /* recommended */
                                   fd,
                                   buf.m.offset);

        buffer.format = active_video_format.getFormatDescription();

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

        buffer.pitch = active_video_format.getPitchSize();
        if (buffer.pData == MAP_FAILED)
        {
            tcam_log(TCAM_LOG_ERROR, "MMAP failed for buffer %d. Aborting.", n_buffers);
            // TODO: errno
            return;
        }

        buffers.at(n_buffers)->setImageBuffer(buffer);
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
        if (-1 == munmap(buffers.at(i)->getImageBuffer().pData, buffers.at(i)->getImageBuffer().length))
        {
            // TODO: error

            return;
        }
        else
        {
            auto buf = buffers.at(i)->getImageBuffer();

            buf.pData = nullptr;
            buf.length = 0;

            buffers.at(i)->setImageBuffer(buf);
        }
    }

	uint32_t buftype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_requestbuffers reqbufs;

    reqbufs_mmap(fd, reqbufs, buftype, 1);  // videobuf workaround
    reqbufs_mmap(fd, reqbufs, buftype, 0);
}


tcam_image_size V4l2Device::get_sensor_size ()
{
    tcam_image_size size = {};
    for (const auto& f : available_videoformats)
    {
        for (const auto& r :f.getResolutions())
        {
            if (r.width > size.width || r.height > size.width)
            {
                size = r;
            }
        }
    }

    return size;
}
