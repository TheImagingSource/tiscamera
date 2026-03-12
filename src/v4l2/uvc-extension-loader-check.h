#pragma once

#include <linux/videodev2.h>

namespace tcam::uvc
{

inline __u32 get_effective_v4l2_caps(const v4l2_capability& caps) noexcept
{
    if (caps.capabilities & V4L2_CAP_DEVICE_CAPS)
    {
        return caps.device_caps;
    }
    return caps.capabilities;
}

inline bool is_metadata_capture_device(const v4l2_capability& caps) noexcept
{
    return (get_effective_v4l2_caps(caps) & V4L2_CAP_META_CAPTURE) != 0;
}

} // namespace tcam::uvc
