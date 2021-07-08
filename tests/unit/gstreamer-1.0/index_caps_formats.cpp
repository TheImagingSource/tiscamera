
#include "tcamgstbase.h"

#include <catch.hpp>
#include <gst/gst.h>
#include <string>
#include <vector>


TEST_CASE("tcamgstbase.cpp::index_caps_formats")
{

    SECTION("single format, single entry")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }

    SECTION("single format, multiple entries")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE; "
                                           "video/x-raw,format=GRAY16_LE");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }

    SECTION("two formats, single entries")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-bayer,format=rggb",
                                              "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }

    SECTION("two formats, multiple entries")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb; "
                                           "video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-bayer,format=rggb",
                                              "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }

    SECTION("three formats, single entries")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb; "
                                           "video/x-bayer,format=rggb16");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-bayer,format=rggb",
                                              "video/x-bayer,format=rggb16",
                                              "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }

    SECTION("three formats, multiple entries")
    {
        GstCaps* in = gst_caps_from_string("video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb; "
                                           "video/x-bayer,format=rggb16; "
                                           "video/x-bayer,format=rggb16; "
                                           "video/x-raw,format=GRAY16_LE; "
                                           "video/x-bayer,format=rggb");

        std::vector<std::string> out = tcam::gst::index_caps_formats(in);

        std::vector<std::string> expected = { "video/x-bayer,format=rggb",
                                              "video/x-bayer,format=rggb16",
                                              "video/x-raw,format=GRAY16_LE" };

        REQUIRE(out == expected);
    }
}
