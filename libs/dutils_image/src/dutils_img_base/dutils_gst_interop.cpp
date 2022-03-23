
#include <dutils_img_lib/dutils_gst_interop.h>

#include <dutils_img/image_fourcc_enum.h>

#include <cstring>  // strcmp
#include <cassert>  // assert

namespace
{
    struct gst_dutil_mapping
    {
        const img::fourcc fourcc = img::fourcc::FCC_NULL;
        const char* gst_format_type = nullptr;
        const char* gst_format_string = nullptr;
    };

    static const char* g_gst_video_raw = "video/x-raw";
    static const char* g_gst_video_bayer = "video/x-bayer";
    static const char* g_gst_video_tis = "video/tis";

    static const gst_dutil_mapping tcam_gst_caps_info[] =
    {
        { img::fourcc::BGRA32,              g_gst_video_raw,    "BGRx", },
        { img::fourcc::BGR24,               g_gst_video_raw,    "BGR", },
        { img::fourcc::BGRA64,              g_gst_video_raw,    "RGBx64", },

        { img::fourcc::MONO8,               g_gst_video_raw,    "GRAY8", },
        { img::fourcc::MONO10,              g_gst_video_raw,    "GRAY10" },
        { img::fourcc::MONO10_SPACKED,      g_gst_video_raw,    "GRAY10sp" },
        { img::fourcc::MONO10_MIPI_PACKED,  g_gst_video_raw,    "GRAY10m" },
        { img::fourcc::MONO12,              g_gst_video_raw,    "GRAY12" },
        { img::fourcc::MONO12_PACKED,       g_gst_video_raw,    "GRAY12p" },
        { img::fourcc::MONO12_SPACKED,      g_gst_video_raw,    "GRAY12sp" },
        { img::fourcc::MONO12_MIPI_PACKED,  g_gst_video_raw,    "GRAY12m" },
        { img::fourcc::MONO16,              g_gst_video_raw,    "GRAY16_LE" },

        { img::fourcc::GRBG8,               g_gst_video_bayer,  "grbg", },
        { img::fourcc::RGGB8,               g_gst_video_bayer,  "rggb", },
        { img::fourcc::GBRG8,               g_gst_video_bayer,  "gbrg", },
        { img::fourcc::BGGR8,               g_gst_video_bayer,  "bggr", },

        { img::fourcc::GBRG10,              g_gst_video_bayer,  "gbrg10", },
        { img::fourcc::BGGR10,              g_gst_video_bayer,  "bggr10", },
        { img::fourcc::GRBG10,              g_gst_video_bayer,  "grbg10", },
        { img::fourcc::RGGB10,              g_gst_video_bayer,  "rggb10", },

        { img::fourcc::GBRG10_SPACKED,      g_gst_video_bayer,  "gbrg10sp", },
        { img::fourcc::BGGR10_SPACKED,      g_gst_video_bayer,  "bggr10sp", },
        { img::fourcc::GRBG10_SPACKED,      g_gst_video_bayer,  "grbg10sp", },
        { img::fourcc::RGGB10_SPACKED,      g_gst_video_bayer,  "rggb10sp", },

        { img::fourcc::GBRG10_MIPI_PACKED,  g_gst_video_bayer,  "gbrg10m", },
        { img::fourcc::BGGR10_MIPI_PACKED,  g_gst_video_bayer,  "bggr10m", },
        { img::fourcc::GRBG10_MIPI_PACKED,  g_gst_video_bayer,  "grbg10m", },
        { img::fourcc::RGGB10_MIPI_PACKED,  g_gst_video_bayer,  "rggb10m", },

        { img::fourcc::GBRG12,              g_gst_video_bayer,  "gbrg12", },
        { img::fourcc::BGGR12,              g_gst_video_bayer,  "bggr12", },
        { img::fourcc::GRBG12,              g_gst_video_bayer,  "grbg12", },
        { img::fourcc::RGGB12,              g_gst_video_bayer,  "rggb12", },

        { img::fourcc::GBRG12_PACKED,       g_gst_video_bayer,  "gbrg12p", },
        { img::fourcc::BGGR12_PACKED,       g_gst_video_bayer,  "bggr12p", },
        { img::fourcc::GRBG12_PACKED,       g_gst_video_bayer,  "grbg12p", },
        { img::fourcc::RGGB12_PACKED,       g_gst_video_bayer,  "rggb12p", },

        { img::fourcc::GBRG12_SPACKED,      g_gst_video_bayer,  "gbrg12sp", },
        { img::fourcc::BGGR12_SPACKED,      g_gst_video_bayer,  "bggr12sp", },
        { img::fourcc::GRBG12_SPACKED,      g_gst_video_bayer,  "grbg12sp", },
        { img::fourcc::RGGB12_SPACKED,      g_gst_video_bayer,  "rggb12sp", },

        { img::fourcc::GBRG12_MIPI_PACKED,  g_gst_video_bayer,  "gbrg12m", },
        { img::fourcc::BGGR12_MIPI_PACKED,  g_gst_video_bayer,  "bggr12m", },
        { img::fourcc::GRBG12_MIPI_PACKED,  g_gst_video_bayer,  "grbg12m", },
        { img::fourcc::RGGB12_MIPI_PACKED,  g_gst_video_bayer,  "rggb12m", },

        { img::fourcc::GBRG16,              g_gst_video_bayer,  "gbrg16", },
        { img::fourcc::BGGR16,              g_gst_video_bayer,  "bggr16", },
        { img::fourcc::GRBG16,              g_gst_video_bayer,  "grbg16", },
        { img::fourcc::RGGB16,              g_gst_video_bayer,  "rggb16", },

        { img::fourcc::GBRGFloat,               g_gst_video_bayer,  "gbrgf", },
        { img::fourcc::BGGRFloat,               g_gst_video_bayer,  "bggrf", },
        { img::fourcc::GRBGFloat,               g_gst_video_bayer,  "grbgf", },
        { img::fourcc::RGGBFloat,               g_gst_video_bayer,  "rggbf", },

        { img::fourcc::YUY2,                    g_gst_video_raw, "YUY2", },
        { img::fourcc::UYVY,                    g_gst_video_raw,  "UYVY", },
        { img::fourcc::Y411,                    g_gst_video_raw, "IYU1", },
        { img::fourcc::MJPG,                    "image/jpeg", nullptr, },
        { img::fourcc::NV12,                    g_gst_video_raw, "NV12", },
        { img::fourcc::YV12,                    g_gst_video_raw, "YV12", },

        { img::fourcc::POLARIZATION_MONO8_90_45_135_0,          g_gst_video_raw,    "polarized-GRAY8-v0", },
        { img::fourcc::POLARIZATION_MONO12_PACKED_90_45_135_0,  g_gst_video_raw,    "polarized-GRAY12p-v0", },
        { img::fourcc::POLARIZATION_MONO12_SPACKED_90_45_135_0, g_gst_video_raw,    "polarized-GRAY12sp-v0", },
        { img::fourcc::POLARIZATION_MONO16_90_45_135_0,         g_gst_video_raw,    "polarized-GRAY16-v0", },

        { img::fourcc::POLARIZATION_BG8_90_45_135_0,            g_gst_video_bayer,  "polarized-bggr8-v0", },
        { img::fourcc::POLARIZATION_BG12_SPACKED_90_45_135_0,   g_gst_video_bayer,  "polarized-bggr12sp-v0", },
        { img::fourcc::POLARIZATION_BG12_PACKED_90_45_135_0,    g_gst_video_bayer,  "polarized-bggr12p-v0", },
        { img::fourcc::POLARIZATION_BG16_90_45_135_0,           g_gst_video_bayer,  "polarized-bggr16-v0", },

        { img::fourcc::POLARIZATION_ADI_MONO8,              g_gst_video_tis,    "polarized-ADI-GRAY8" },
        { img::fourcc::POLARIZATION_ADI_MONO16,             g_gst_video_tis,    "polarized-ADI-GRAY16" },
        { img::fourcc::POLARIZATION_ADI_RGB8,               g_gst_video_tis,    "polarized-ADI-RGB8" },
        { img::fourcc::POLARIZATION_ADI_RGB16,              g_gst_video_tis,    "polarized-ADI-RGB16" },
        { img::fourcc::POLARIZATION_PACKED8,                g_gst_video_raw,    "polarized-packed-GRAY8" },
        { img::fourcc::POLARIZATION_PACKED16,               g_gst_video_raw,    "polarized-packed-GRAY16" },
        { img::fourcc::POLARIZATION_PACKED8_BAYER_BG,       g_gst_video_bayer,  "polarized-packed-bggr8" },
        { img::fourcc::POLARIZATION_PACKED16_BAYER_BG,      g_gst_video_bayer,  "polarized-packed-bggr16" },

        { img::fourcc::PWL_RG12_MIPI,           g_gst_video_bayer, "pwl-rggb12m" },
        { img::fourcc::PWL_RG12,                g_gst_video_bayer, "pwl-rggb12" },
        { img::fourcc::PWL_RG16H12,             g_gst_video_bayer, "pwl-rggb16H12" },

        { img::fourcc::MONOFloat,               g_gst_video_raw, "GREYf" },
    };
}

std::string     img_lib::gst::fourcc_to_gst_caps_string( uint32_t fourcc )
{
    for( const auto& info : tcam_gst_caps_info )
    {
        if( fourcc == (uint32_t) info.fourcc )
        {
            std::string str = info.gst_format_type;
            if( info.gst_format_string ) {  // this may be necessary for MJPEG where gst_format_string == nullptr
                str += ",format=(string)";
                str += info.gst_format_string;
            }
            return str;
        }
    }
    return {};
}

std::string     img_lib::gst::fourcc_to_gst_caps_string( img::fourcc fourcc )
{
    for( const auto& info : tcam_gst_caps_info )
    {
        if( fourcc == info.fourcc )
        {
            std::string str = info.gst_format_type;
            if( info.gst_format_string ) {  // this may be necessary for MJPEG where gst_format_string == nullptr
                str += ",format=(string)";
                str += info.gst_format_string;
            }
            return str;
        }
    }
    return {};
}

img::fourcc    img_lib::gst::gst_caps_string_to_fourcc( std::string_view format_type, std::string_view format_string )
{
    for( const auto& info : tcam_gst_caps_info )
    {
        if( format_type != info.gst_format_type ) {
            continue;
        }

        if( info.gst_format_string == nullptr ) {   // this is currently only for image/jpeg, there we don't have a gst_format to match
            return info.fourcc;
        }

        assert( format_string.empty() != true );
        if( format_string.empty() ) { // this has practically no meaning, we would return the first fourcc
            return img::fourcc::FCC_NULL;
        }

        if( format_string == info.gst_format_string )
        {
            return info.fourcc;
        }
    }
    return img::fourcc::FCC_NULL;
}

img_lib::gst::gst_caps_descr img_lib::gst::fourcc_to_gst_caps_descr(img::fourcc fourcc)
{
    for (const auto& info : tcam_gst_caps_info)
    {
        if (fourcc == info.fourcc)
        {
            return { info.gst_format_type, info.gst_format_string };
        }
    }
    return { nullptr, nullptr };
}
