/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include <QObject>

enum class FormatHandling : int
{
    Static = 0,
    Auto,
    Dialog,
};


enum class ConversionElement : int
{
    Auto = 0,
    TcamConvert,
    TcamDutils,
    TcamDutilsCuda,
};


// supported types are identical
// to the types QImage::save supports
enum class ImageSaveType
{
    BMP,
    GIF,
    JPG,
    JPEG,
    PNG,
    PBM,
    PGM,
    XBM,
    XPM,
};


enum class VideoCodec
{
    H264,
    //H265,
    MJPEG,
};


inline const char* conversion_element_to_string (const ConversionElement entry)
{
    switch(entry)
    {
        case ConversionElement::Auto:
        {
            return "auto";
        }
        case ConversionElement::TcamConvert:
        {
            return "tcamconvert";
        }
        case ConversionElement::TcamDutils:
        {
            return "tcamdutils";
        }
        case ConversionElement::TcamDutilsCuda:
        {
            return "tcamdutils-cuda";
        }
        default:
        {
            return "";
        }
    }
}

inline QString video_codec_to_string(VideoCodec c)
{
    switch (c)
    {
        case VideoCodec::H264:
        {
            return "H264";
        }
        case VideoCodec::MJPEG:
        {
            return "MJPEG";
        }
    }
    return "";
}


inline QString video_codec_file_extension(QString c)
{
    if (c == video_codec_to_string(VideoCodec::H264))
    {
        return "mp4";
    }
    if (c == video_codec_to_string(VideoCodec::MJPEG))
    {
        return "avi";
    }
    return "";
}

inline QString video_codec_file_extension(VideoCodec c)
{
    switch (c)
    {
        case VideoCodec::H264:
        {
            return "mp4";
        }
        case VideoCodec::MJPEG:
        {
            return "avi";
        }
    }
    return "";
}

inline QString image_save_type_to_string(ImageSaveType t)
{
    switch (t)
    {
        case ImageSaveType::BMP:
        {
            return "BMP";
        }
        case ImageSaveType::GIF:
        {
            return "GIF";
        }
        case ImageSaveType::JPG:
        {
            return "JPG";
        }
        case ImageSaveType::JPEG:
        {
            return "JPEG";
        }
        case ImageSaveType::PNG:
        {
            return "PNG";
        }
        case ImageSaveType::PBM:
        {
            return "BPM";
        }
        case ImageSaveType::PGM:
        {
            return "PGM";
        }
        case ImageSaveType::XBM:
        {
            return "XBM";
        }
        case ImageSaveType::XPM:
        {
            return "XPM";
        }
    }
    return "";
}


inline std::vector<QString> get_image_save_type_names()
{
    return {
        "BMP", "GIF", "JPG", "JPEG", "PNG", "PBM", "PGM", "XBM", "XPM",
    };
}


inline std::vector<QString> get_video_codec_names()
{
    return {
        "h264",
        "MJPEG",
    };
}
