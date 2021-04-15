
#include "V4L2PropertyBackend.h"

#include <linux/videodev2.h>
#include "utils.h"

tcam::property::V4L2PropertyBackend::V4L2PropertyBackend(int fd)
    : p_fd(fd)
{}


int tcam::property::V4L2PropertyBackend::write_control(int v4l2_id, int new_value)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;
    ctrl.value = new_value;

    return tcam_xioctl(p_fd, VIDIOC_S_CTRL, &ctrl);
}


int tcam::property::V4L2PropertyBackend::read_control(int v4l2_id, int64_t& new_value)
{
    struct v4l2_control ctrl = {};
    ctrl.id = v4l2_id;

    if (tcam_xioctl(p_fd, VIDIOC_G_CTRL, &ctrl) != 0)
    {
        // SPDLOG_WARN("Could not retrieve current value for {}. ioctl return '{}'",
        //              desc.prop->get_name().c_str(),
        //              strerror(errno));
        return -1;
    }
    new_value = ctrl.value;
    return 0;
}
