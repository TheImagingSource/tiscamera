
#include "UsbCapture.h"
#include "tis_logging.h"
#include "tis_utils.h"
#include "PropertyGeneration.h"

#include <algorithm>
#include <unistd.h>
#include <fcntl.h> /* O_RDWR O_NONBLOCK */
#include <linux/videodev2.h>
#include <cstring> /* memcpy*/

using namespace tis_imaging;


UsbCapture::UsbCapture (const CaptureDevice& _device)
    : device(_device)
{

    if ((fd = open(device.getInfo().identifier, O_RDWR /* required */ | O_NONBLOCK, 0)) == -1)
    {
        tis_log(TIS_LOG_ERROR, "Unable to open device \'%s\'.", device.getInfo().identifier);
        throw std::runtime_error("Failed opening device.");
    }

    this->index_formats();
}


UsbCapture::~UsbCapture ()
{
    if (this->fd != -1)
    {
        close(fd);
        fd = -1;
    }
}


CaptureDevice UsbCapture::getDeviceDescription () const
{
    return device;
}


std::vector<std::shared_ptr<Property>> UsbCapture::getProperties ()
{
    if (this->properties.empty())
    {
        index_all_controls(shared_from_this());
    }
    
    std::vector<std::shared_ptr<Property>> props;

    for ( const auto& p : properties )
    {
        props.push_back(p.prop);
    }
    
    return props;
}



bool UsbCapture::isAvailable (const Property&)
{
    return false;
}


bool UsbCapture::setProperty (const Property& _property)
{
    auto f = [&_property] (const property_description& d)
        {
            return (*d.prop == _property);
        };
    
    auto desc = std::find_if(properties.begin(), properties.end(),f);

    if (desc == properties.end())
    {
        // TODO: failure description
        return false;
    }

    const property_description write_desc = {desc->id, NULL};
    
    if (changeV4L2Control(write_desc))
    {
        // now save property internally
        *desc->prop = _property;
        
        return true;
    }

    return false;
}


bool UsbCapture::getProperty (Property&)
{
    return false;
}


bool UsbCapture::setVideoFormat (const VideoFormat& _format)
{
    if (!validateVideoFormat(_format))
    {
        tis_log(TIS_LOG_ERROR, "Not a valid format.");
        return false;
    }

    // set format in camera
    
    struct v4l2_format fmt = {};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = _format.getSize().width;
    fmt.fmt.pix.height = _format.getSize().height;

    fmt.fmt.pix.pixelformat = _format.getFourcc();
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    int ret = tis_xioctl(this->fd, VIDIOC_S_FMT, &fmt);

    if (ret < 0)
    {
        tis_log(TIS_LOG_ERROR, "Error while setting format");

        // TODO error handling
        return false;
    }

    // set framerate
    
    
    // copy format as local reference
    active_video_format = _format;
    
    return true;
}


bool UsbCapture::validateVideoFormat (const VideoFormat& _format)
{

    for (const auto& f : available_videoformats)
    {
        if (f.isValidVideoFormat(_format))
        {
            return true;
        }
    }
    return false;
}


VideoFormat UsbCapture::getActiveVideoFormat () const
{
    return active_video_format;
}


std::vector<VideoFormatDescription> UsbCapture::getAvailableVideoFormats () const
{
    return available_videoformats;
}


bool UsbCapture::setFramerate (double framerate)
{

    // TODO what about range framerates?
	struct v4l2_streamparm parm;

    auto fps = std::find_if(framerate_conversions.begin(),
                            framerate_conversions.end(),
                            [&framerate] (const framerate_conv& f) { return (framerate == f.fps);});

    if (fps == framerate_conversions.end())
    {
        return false;
    }
    
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = fps->numerator;
	parm.parm.capture.timeperframe.denominator = fps->denominator;

    int ret = tis_xioctl( fd, VIDIOC_S_PARM, &parm );
    
	if (ret < 0)
    {
        
		fprintf (stderr, "Failed to set frame rate\n");
        return false;
    }
    return true;
}


void UsbCapture::index_formats ()
{
    struct v4l2_fmtdesc fmtdesc = {};
    struct v4l2_frmsizeenum frms = {};

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (fmtdesc.index = 0; ! tis_xioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc); fmtdesc.index ++)
    {
        struct video_format_description desc = {};

        // internal fourcc definitions are identical with v4l2
        desc.fourcc = fmtdesc.pixelformat;
        memcpy (desc.description, fmtdesc.description, 256);
        frms.pixel_format = fmtdesc.pixelformat;

        for (frms.index = 0; ! tis_xioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frms); frms.index++)
        {
            if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                desc.min_size.width = frms.discrete.width;
                desc.max_size.width = frms.discrete.width;
                desc.min_size.height = frms.discrete.height;
                desc.max_size.height = frms.discrete.height;
                desc.framerate_type = TIS_FRAMERATE_TYPE_FIXED;
                std::vector<double> f = index_framerates(frms);

                VideoFormatDescription format(desc, f);
                available_videoformats.push_back(format);

            }
            else
            {
                // TIS USB cameras do not have this kind of setting
                tis_log(TIS_LOG_ERROR, "Encountered unknown V4L2_FRMSIZE_TYPE");
            }
        }
    }

}


std::vector<double> UsbCapture::index_framerates (const struct v4l2_frmsizeenum& frms)
{
    struct v4l2_frmivalenum frmival = {};

    frmival.pixel_format = frms.pixel_format;
    frmival.width = frms.discrete.width;
    frmival.height = frms.discrete.height;

    std::vector<double> f;

    for (frmival.index = 0; tis_xioctl( fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival ) >= 0; frmival.index++)
    {
        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {

            // v4l2 lists frame rates as fractions (number of seconds / frames (e.g. 1/30))
            // we however use framerates as fps (e.g. 30/1)
            // therefor we have to switch numerator and denominator

            double frac = (double)frmival.discrete.numerator/frmival.discrete.denominator;
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


void UsbCapture::index_all_controls (std::shared_ptr<PropertyImpl> impl)
{

    
    struct v4l2_queryctrl qctrl = { V4L2_CTRL_FLAG_NEXT_CTRL };

    while (tis_xioctl(this->fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        index_control(&qctrl, impl);
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
    
    if (qctrl.id != V4L2_CTRL_FLAG_NEXT_CTRL)
    {
        return;
    }
}


int UsbCapture::index_control (struct v4l2_queryctrl* qctrl, std::shared_ptr<PropertyImpl> impl)
{

    if (qctrl->flags & V4L2_CTRL_FLAG_DISABLED)
    {
        return 1;
    }

    if (qctrl->type == V4L2_CTRL_TYPE_CTRL_CLASS)
    {
        // ignore unneccesary controls descriptions such as control "groups"
        // tis_log(TIS_LOG_DEBUG, "/n%s/n", qctrl->name);
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
            tis_log(TIS_LOG_INFO, "Encountered write only control.");
        }
        
        else if (tis_xioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls))
        {
            tis_log(TIS_LOG_ERROR, "Errno %d getting ext_ctrl %s", errno, qctrl->name);
            return -1;
        }
    }
    else
    {
        struct v4l2_control ctrl = {};
        
        ctrl.id = qctrl->id;
        if (tis_xioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
            tis_log(TIS_LOG_ERROR, "error %d getting ctrl %s", errno, qctrl->name);
            return -1;
        }
        ext_ctrl.value = ctrl.value;
    }

    auto p = createProperty(fd, qctrl, &ext_ctrl, impl);

    struct property_description desc;

    desc.id = qctrl->id;
    desc.prop = p;

    properties.push_back(desc);
    
    if (qctrl->type == V4L2_CTRL_TYPE_STRING)
    {
        free(ext_ctrl.string);
    }
    return 1;
}


bool UsbCapture::propertyChangeEvent (const Property& _property)
{

    tis_log(TIS_LOG_DEBUG,"Property %s changed!", _property.getName().c_str());

    if (!setProperty(_property))
    {
        tis_log(TIS_LOG_ERROR, "Error while setting property");

    }
    
    return true;
}


bool UsbCapture::changeV4L2Control (const property_description& _property)
{

    // currently no extended controls are in use
    // always use v4l2_control instead of v4l2_ext_control
    

    PROPERTY_TYPE type = _property.prop->getType();
    
    if (type == PROPERTY_TYPE_STRING ||
        type == PROPERTY_TYPE_UNKNOWN ||
        type == PROPERTY_TYPE_DOUBLE)
    {
        tis_log(TIS_LOG_ERROR, "Property type not supported. Property changes not submitted to device.");
        return false;
    }

    struct v4l2_control ctrl = {0};
    
    ctrl.id = _property.id;
    // TODO: get value
    ctrl.value = 0;

    if (type == PROPERTY_TYPE_INTEGER)
    {
        ctrl.value = (std::dynamic_pointer_cast<PropertyInteger>(_property.prop))->getValue();
    }
    else if (type == PROPERTY_TYPE_BOOLEAN)
    {
        ctrl.value = (std::dynamic_pointer_cast<PropertySwitch>(_property.prop))->getValue();
    }
    
    int ret = tis_xioctl(fd, VIDIOC_S_CTRL, &ctrl);

    if (ret < 0)
    {
        tis_log(TIS_LOG_ERROR, "Unable to submit property change.");
    }
    else
    {
        tis_log(TIS_LOG_ERROR,
                "Changed ctrl %s to value %d.",
                _property.prop->getName().c_str(),
                ctrl.value);
    }

    return true;
}
