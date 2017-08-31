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

#include <stdint.h>
#include <stdbool.h>

#include "property_identifications.h"
#include "image_fourcc.h"

/**
* @name TCAM_PIPELINE_STATUS
* @brief overview over possible pipeline states
*/
enum TCAM_PIPELINE_STATUS
{
    TCAM_PIPELINE_UNDEFINED = 0,
    TCAM_PIPELINE_STOPPED,
    TCAM_PIPELINE_PAUSED,
    TCAM_PIPELINE_PLAYING,
    TCAM_PIPELINE_ERROR,
};


/**
 * Supported camera types
 */
enum TCAM_DEVICE_TYPE
{
    TCAM_DEVICE_TYPE_UNKNOWN = 0, /**< Unknown device type*/
    TCAM_DEVICE_TYPE_V4L2,        /**< device that uses the v4l2 API */
    TCAM_DEVICE_TYPE_ARAVIS,      /**< currently through aravis */
    TCAM_DEVICE_TYPE_LIBUSB,      /**< libusb backends */
};


/**
 * @name tcam_device_info
 * Simple device description containing all information to uniquely identify a device
 */
struct tcam_device_info
{
    enum TCAM_DEVICE_TYPE type; /**< type of camera connection */
    char name[128];             /**< Camera name (e.g. DFK 23UP031) */
    char identifier[128];       /**< identifier used for camera interaction (e.g. /dev/video0) */
    char serial_number[64];     /**< unique identifier */
    char additional_identifier[128]; /**< additional information for identification of camera model / must not be required by user*/
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
    struct tcam_image_size min_size;         /**< smallest available resolution */
    struct tcam_image_size max_size;         /**< biggest available resolution */

    uint32_t framerate_count;                /**< number of framerates this resolution supports */
};


/**
 * @name tcam_video_format_description
 * @brief generic format description
 */
struct tcam_video_format_description
{
    uint32_t fourcc;                         /**< pixel format that is used e.g. RGB32 or Y800 */
    char description [256];
    uint32_t binning;
    uint32_t skipping;

    uint32_t resolution_count;               /**< number of resolutions this format supports */
};


/**
 * @name tcam_video_format
 * @brief description of a specific video format
 */
struct tcam_video_format
{
    uint32_t fourcc;  /**< pixel format that is used e.g. RGB32 or Y800 */
    uint32_t binning;
    uint32_t skipping;
    uint32_t width;
    uint32_t height;
    double   framerate;
};


/**
 * Statistic container for additional image_buffer descriptions
 */
struct tcam_stream_statistics
{
    uint64_t frame_count;      /**< current frame number */
    uint64_t frames_dropped;   /**< number of frames that where not delivered */
    uint64_t capture_time_ns;  /**< capture time reported by lib */
    uint64_t camera_time_ns;   /**< capture time reported by camera; empty if not supported */
    double   framerate;        /**< in contrast to selected one */
};

/**
 * @name tcam_image_buffer
 * @brief container for image transfer
 */
struct tcam_image_buffer
{
    unsigned char*           pData;    /**< pointer to actual image buffer */
    unsigned int             length;   /**< size of image in bytes */
    unsigned int             size;     /**< size of image buffer in bytes */
    struct tcam_video_format format;   /**< tcam_video_format the image buffer has */
    unsigned int             pitch;    /**< length of single image line in bytes */
    struct tcam_stream_statistics statistics;

    uint32_t lock_count;
};


/**
 * @enum TCAM_PROPERTY_TYPE
 * Available property types
*/
enum TCAM_PROPERTY_TYPE
{
    TCAM_PROPERTY_TYPE_UNKNOWN      = 0,
    TCAM_PROPERTY_TYPE_BOOLEAN      = 1,
    TCAM_PROPERTY_TYPE_INTEGER      = 2,
    TCAM_PROPERTY_TYPE_DOUBLE       = 3,
    TCAM_PROPERTY_TYPE_STRING       = 4,
    TCAM_PROPERTY_TYPE_ENUMERATION  = 5,
    TCAM_PROPERTY_TYPE_BUTTON       = 6, /**< the button type is just a command
                                            to trigger some functionality
                                            which doesn't care about parameters
                                            because that is actually not necessary.
                                            For instance, "adjusting white balance"
                                            button in digital camera could
                                            be a good example which is performing
                                            adjustment of white balance for
                                            one time and no need for any kind of
                                            parameter for this but needs to
                                            be triggered. */
};


struct tcam_value_int
{
    int64_t min;
    int64_t max;
    int64_t step;         /* 0 if steps not possible */
    int64_t default_value;
    int64_t value;
};


struct tcam_value_double
{
    double min;
    double max;
    double step;         /* 0.0 if steps not possible */
    double default_value;
    double value;
};


struct tcam_value_string
{
    char value[64];
    char default_value[64];
};


struct tcam_value_bool
{
    bool value;
    bool default_value;
};


/**
 * Categorization of properties works by
 * assigning every property a category and group.
 * A group is defined by the property id of the leading member
 * e.g. TCAM_PROPERTY_EXPOSURE_AUTO is a member of the group TCAM_PROPERTY_EXPOSURE.
 * The resulting tree structure allows for a
 */

/**
 * @enum TCAM_PROPERTY_CATEGORY
 * available property categories
 */
enum TCAM_PROPERTY_CATEGORY
{
    TCAM_PROPERTY_CATEGORY_UNKNOWN = 0,
    TCAM_PROPERTY_CATEGORY_EXPOSURE,
    TCAM_PROPERTY_CATEGORY_COLOR,
    TCAM_PROPERTY_CATEGORY_LENS,
    TCAM_PROPERTY_CATEGORY_SPECIAL,
    TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN,
    TCAM_PROPERTY_CATEGORY_IMAGE,
    TCAM_PROPERTY_CATEGORY_AUTO_ROI,
    TCAM_PROPERTY_CATEGORY_WDR,
};


/**
 * @struct tcam_property_group
 * grouping description for properties
 */
struct tcam_property_group
{
    enum TCAM_PROPERTY_CATEGORY property_category; /**< category of the property */
    TCAM_PROPERTY_ID            property_group;    /**< group of the property
                                                      if property_group and tcam_device_property.id
                                                      are identical the property should be considered
                                                      the group master */
};


/**
 * @struct tcam_device_property
 * @brief unified property description
 */
struct tcam_device_property
{
    TCAM_PROPERTY_ID id;               /**< unique identifier */
    char name [64];                    /**< string identifier */

    struct tcam_property_group group;  /**< grouping; if you simply want to
                                          iterate properties you can ignore this */

    enum TCAM_PROPERTY_TYPE type;      /**< type the property has */
    union
    {
        struct tcam_value_int i;
        struct tcam_value_double d;
        struct tcam_value_string s;
        struct tcam_value_bool b;
    } value;                           /**< actual value settings */

    uint32_t flags;                    /**< bit flags */
};


/**
 * used property flags
 */
enum TCAM_PROPERTY_FLAGS
{
    TCAM_PROPERTY_FLAG_DISABLED = 0x0001,    /**< This control is permanently disabled and
                                                should be ignored by the application. Any
                                                attempt to change the control will
                                                result in an EINVAL error code. */

    TCAM_PROPERTY_FLAG_GRABBED = 0x0002,     /**< This control is temporarily unchangeable,
                                                for example because another application
                                                took over control of the respective
                                                resource. Such controls may be displayed
                                                specially in a user interface. Attempts to
                                                change the control may result in an EBUSY error code. */

    TCAM_PROPERTY_FLAG_READ_ONLY = 0x0004,   /**< This control is permanently readable only.
                                                Any attempt to change the control will result
                                                in an EINVAL error code. */

    TCAM_PROPERTY_FLAG_EXTERNAL = 0x0008,    /**< This control is realized through library
                                                 code and is not available in the camera */

    TCAM_PROPERTY_FLAG_INACTIVE = 0x0010,    /**< This control is not applicable to the
                                                current configuration and should be displayed
                                                accordingly in a user interface. For example
                                                the flag may be set on a MPEG audio level 2
                                                bitrate control when MPEG audio encoding
                                                level 1 was selected with another control. */

    TCAM_PROPERTY_FLAG_WRITE_ONLY = 0x0020,  /**< This control is permanently writable only.
                                                Any attempt to read the control will result
                                                in an EACCES error code error code. This flag
                                                is typically present for relative controls
                                                or action controls where writing a value will
                                                cause the device to carry out a given action
                                                (e. g. motor control) but no meaningful
                                                value can be returned. */
};

/** @} */

#endif /* TCAM_BASE_TYPES_H */
