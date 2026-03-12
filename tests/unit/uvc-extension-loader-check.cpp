#define CATCH_CONFIG_NO_POSIX_SIGNALS
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "v4l2/uvc-extension-loader-check.h"

TEST_CASE("metadata capture nodes are rejected from device_caps", "[uvc-extension-loader]")
{
    v4l2_capability caps = {};
    caps.capabilities = V4L2_CAP_DEVICE_CAPS | V4L2_CAP_VIDEO_CAPTURE;
    caps.device_caps = V4L2_CAP_META_CAPTURE;

    REQUIRE(tcam::uvc::is_metadata_capture_device(caps));
}

TEST_CASE("metadata capture nodes are rejected from capabilities when device_caps is absent",
          "[uvc-extension-loader]")
{
    v4l2_capability caps = {};
    caps.capabilities = V4L2_CAP_META_CAPTURE | V4L2_CAP_VIDEO_CAPTURE;

    REQUIRE(tcam::uvc::is_metadata_capture_device(caps));
}

TEST_CASE("device_caps take precedence over capabilities", "[uvc-extension-loader]")
{
    v4l2_capability caps = {};
    caps.capabilities =
        V4L2_CAP_DEVICE_CAPS | V4L2_CAP_META_CAPTURE | V4L2_CAP_VIDEO_CAPTURE;
    caps.device_caps = V4L2_CAP_VIDEO_CAPTURE;

    REQUIRE_FALSE(tcam::uvc::is_metadata_capture_device(caps));
}
