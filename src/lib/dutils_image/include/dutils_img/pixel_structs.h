
#pragma once

#include <cstdint>

#pragma pack(push,1)

namespace img {
namespace pixel_type {

    struct UYVY
    {
        uint8_t	u, y0, v, y1;
    };

    struct YUY2
    {
        uint8_t	y0, u, y1, v;
    };

    struct IYU2
    {
        uint8_t	u0, y0, v0;
    };

    struct YUV48_LE {
        uint16_t y0, u0, v0;
    };
    using YUV48 = YUV48_LE;

    struct Y411 {
        uint8_t u, y0, y1, v, y2, y3;
    };

    struct YUV24 {
        uint8_t y0, u0, v0;
    };
    using Y8U8V8 = YUV24;


    struct B8G8R8
    {
        uint8_t b, g, r;
    };
    using BGR24 = B8G8R8;

    struct R8G8B8
    {
        uint8_t r, g, b;
    };

    //using RGB24 = R8G8B8; // this is a bad idea, because there are several things that think RGB24 == bgr order

    struct BGRA32   // This is the default order on windows
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };

    struct R8G8B8A8   // NOTE this is directly r g b a in this order, which is different to the windows BGRA32 default
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    struct BGRA64_LE {
        uint16_t b, g, r, a;
    };
    using BGRA64 = BGRA64_LE;

    struct YUVf {
        float y0, u0, v0;
    };
    struct RGBf {
        float r, g, b;
    };

    struct Y8 {
        uint8_t y;
    };
    struct Y16_LE {
        uint16_t y;
    };
    using Y16 = Y16_LE;

    
    struct [[deprecated( "Don't use this in current/new code, this is underspecified" )]]  YGB0 {
        uint16_t y;
    };
    
    struct HSVx32
    {
        uint8_t h;
        uint8_t s;
        uint8_t v;
        uint8_t x;
    };
    struct HSV24
    {
        uint8_t h;
        uint8_t s;
        uint8_t v;
    };

    // Y8 + A8 channels
    struct Y8A8
    {
        uint8_t y;
        uint8_t a;
    };
    // Y8U8V8A8 channels
    struct  Y8U8V8A8
    {
        uint8_t y;
        uint8_t u;
        uint8_t v;
        uint8_t a;
    };

    struct RAW10
    {
        uint16_t v0;
    };
    struct RAW10_SPACKED // 
    {
        uint8_t v0; // high bits y0
		uint8_t v1; // ...
		uint8_t v2;
		uint8_t v3;
		uint8_t v4;
    };
    struct RAW10_MIPI_PACKED // 
    {
        uint8_t v0; // 8-high bits of y0
        uint8_t v1; // 8-high bits of y1
        uint8_t v2; // 8-high bits of y2
        uint8_t v3; // 8-high bits of y3
        uint8_t v4; // low 2-bits each for y0-3
    };

    struct RAW12
    {
        uint16_t v0;
    };
	struct RAW12_PACKED // 
	{
		uint8_t v0; // hi 8 bits y0
		uint8_t v1; // lo 4 bis y0 | hi 4 bits of y1
		uint8_t v2; // lo 8 bits y1
	};
	struct RAW12_SPACKED // 
	{
		uint8_t v0; // high bits y0
		uint8_t v1; // low bits of y0 | low bits of y1
		uint8_t v2; // high bits y1
	};
	struct RAW12_MIPI_PACKED // 
	{
		uint8_t v0; // high bits y0
		uint8_t v1; // high bits y1
		uint8_t v2; // low bits of y0 | low bits of y1
	};


    namespace polarization
    {
        /** Structure representing some polarization properties of a polarizes image */
        struct ADI_MONO8 {
            uint8_t angleOfMaxPolarization;
            uint8_t degreeOfPolarization;
            uint8_t intensity;
            uint8_t unused;
        };
        struct ADI_MONO16_LE {
            uint16_t angleOfMaxPolarization;
            uint16_t degreeOfPolarization;
            uint16_t intensity;
            uint16_t unused;
        };
        struct ADI_RGB8 {
            uint8_t angleOfMaxPolarization;
            uint8_t dolpRed, dolpGreen, dolpBlue;
            uint8_t intensityRed, intensityGreen, intensityBlue;
            uint8_t unused;
        };
        struct ADI_RGB16_LE {
            uint16_t angleOfMaxPolarization;
            uint16_t dolpRed, dolpGreen, dolpBlue;
            uint16_t intensityRed, intensityGreen, intensityBlue;
            uint16_t unused;
        };
        struct POL_PACKED8 {
            uint8_t angle0;
            uint8_t angle45;
            uint8_t angle90;
            uint8_t angle135;
        };
        struct POL_PACKED16_LE {
            uint16_t angle0;
            uint16_t angle45;
            uint16_t angle90;
            uint16_t angle135;
        };
    }
}
}

#pragma pack(pop)
