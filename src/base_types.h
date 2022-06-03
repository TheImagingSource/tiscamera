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

#ifndef TCAM_BASE_TYPES_H
#define TCAM_BASE_TYPES_H

/**
 * @addtogroup API
 * @{
*/

#include <cstdint>
#include <cstring>


namespace tcam
{

enum TCAM_MEMORY_TYPE
{
    TCAM_MEMORY_TYPE_USERPTR = 0,
    TCAM_MEMORY_TYPE_MMAP = 1,
    TCAM_MEMORY_TYPE_DMA,
    TCAM_MEMORY_TYPE_DMA_IMPORT,
};

/**
 * Supported camera types
 */
enum TCAM_DEVICE_TYPE
{
    TCAM_DEVICE_TYPE_UNKNOWN = 0, /**< Unknown device type*/
    TCAM_DEVICE_TYPE_V4L2, /**< device that uses the v4l2 API */
    TCAM_DEVICE_TYPE_ARAVIS, /**< currently through aravis */
    TCAM_DEVICE_TYPE_LIBUSB, /**< libusb backends */
    TCAM_DEVICE_TYPE_PIMIPI, /**< mipi cameras on raspberry pi*/
    TCAM_DEVICE_TYPE_MIPI, /**< mipi cameras*/
    TCAM_DEVICE_TYPE_TEGRA, /**< tegra fpd/mipi cameras*/
    TCAM_DEVICE_TYPE_VIRTCAM, /**< virtual camera */
};


/**
 * @name tcam_device_info
 * Simple device description containing all information to uniquely identify a device
 */
struct tcam_device_info
{
    enum TCAM_DEVICE_TYPE type; /**< type of camera connection */
    char name[128]; /**< Camera name (e.g. DFK 23UP031) */
    char identifier[128]; /**< identifier used for camera interaction (e.g. /dev/video0) */
    char serial_number[64]; /**< unique identifier */
    char additional_identifier
        [128]; /**< additional information for identification of camera model / must not be required by user*/
};


/**
 * @name device_lost_callback
 *
 */
typedef void (*tcam_device_lost_callback)(const struct tcam_device_info* info, void* user_data);


/**
 * @name tcam_image_size
 */
struct tcam_image_size
{
    uint32_t width;
    uint32_t height;

    bool operator==(const struct tcam_image_size& other) const
    {
        if (height == other.height && width == other.width)
        {
            return true;
        }
        return false;
    }

    bool operator!=(const struct tcam_image_size& other) const
    {
        return !operator==(other);
    }
};

inline bool is_inside_dim_range(tcam_image_size min, tcam_image_size max, tcam_image_size check) noexcept
{
    if( min.width > check.width || max.width < check.width )
        return false;
    if( min.height > check.height || max.height < check.height )
        return false;
    return true;
}

enum ImageScalingType
{
    Unknown = 0,
    None,
    Override,
    Binning,
    Skipping,
    BinningSkipping,
};


// this struct contains all information
// about binning/skipping/scaling
// generally influences possible resolutions
struct image_scaling
{
    // if a camera has only binning
    // binning_h and binning_v will
    // be identical for all entries

    // if camera does not have binning, etc.
    // the value is 1 aka "none"
    int32_t binning_h = 1;
    int32_t binning_v = 1;

    int32_t skipping_h = 1;
    int32_t skipping_v = 1;

    bool operator==(const image_scaling& other) const
    {
        if (binning_h == other.binning_h
            && binning_v == other.binning_v
            && skipping_h == other.skipping_h
            && skipping_v == other.skipping_v)
        {
            return true;
        }

        return false;
    }

    bool is_default() const
    {
        if (binning_h == 1
            && binning_v == 1
            && skipping_h == 1
            && skipping_v == 1)
        {
            return true;
        }
        return false;
    }

    bool legal_resolution (const tcam_image_size& sensor_size,
                           const tcam_image_size& resolution) const
    {
        uint32_t allowed_width = sensor_size.width / (binning_h * skipping_h);
        uint32_t allowed_height = sensor_size.height / (binning_v * skipping_v);
        if (resolution.width <= allowed_width
            && resolution.height <= allowed_height)
        {
            return true;
        }
        return false;
    }

    tcam_image_size allowed_max (const tcam_image_size& sensor_size) const
    {
        return {sensor_size.width / (binning_h * skipping_h), sensor_size.height / (binning_v * skipping_v)};
    }
};


enum TCAM_RESOLUTION_TYPE
{
    TCAM_RESOLUTION_TYPE_RANGE,
    TCAM_RESOLUTION_TYPE_FIXED,
};


struct tcam_resolution_description
{
    enum TCAM_RESOLUTION_TYPE type;

    // these are identical if type is FIXED
    struct tcam_image_size min_size; /**< smallest available resolution */
    struct tcam_image_size max_size; /**< biggest available resolution */

    unsigned int width_step_size;
    unsigned int height_step_size;

    image_scaling scaling;

    bool operator==(const struct tcam_resolution_description& other) const
    {
        if (type == other.type
            && max_size == other.max_size && min_size == other.min_size
            && scaling == other.scaling)
        {
            return true;
        }

        return false;
    }
};


/**
 * @name tcam_video_format_description
 * @brief generic format description
 */
struct tcam_video_format_description
{
    uint32_t fourcc; /**< pixel format that is used e.g. RGB32 or Y800 */
    char description[256];

    bool operator==(const struct tcam_video_format_description& other) const
    {
        if (fourcc == other.fourcc
            && strcmp(description, other.description) == 0)
        {
            return true;
        }

        return false;
    }
};


/**
 * @name tcam_video_format
 * @brief description of a specific video format
 */
struct tcam_video_format
{
    uint32_t fourcc; /**< pixel format that is used e.g. RGB32 or Y800 */

    image_scaling scaling;

    uint32_t width;
    uint32_t height;
    double framerate;
};


/**
 * Statistic container for additional image_buffer descriptions
 */
struct tcam_stream_statistics
{
    uint64_t frame_count; // current frame number
    uint64_t frames_dropped; // number of frames that where not delivered
    uint64_t capture_time_ns; // capture time reported by lib
    uint64_t camera_time_ns; //capture time reported by camera; empty if not supported
    bool is_damaged; // flag indicating if the associated buffer had lost packages or other problems
};


struct tcam_value_int
{
    int64_t min;
    int64_t max;
    int64_t step; /* 0 if steps not possible */
    int64_t default_value;
    int64_t value;
};


struct tcam_value_double
{
    double min;
    double max;
    double step; /* 0.0 if steps not possible */
    double default_value;
    double value;
};

} // namespace tcam

#endif /* TCAM_BASE_TYPES_H */
