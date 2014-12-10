

#ifndef TCAM_BASE_TYPES_H
#define TCAM_BASE_TYPES_H

/**
 * @addtogroup API
 * @{
*/

#include <stdint.h>
#include <stdbool.h>

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
    TCAM_DEVICE_TYPE_FIREWIRE,    /**< both 400 and 800 */
    TCAM_DEVICE_TYPE_ARAVIS,      /**< currently through aravis */
};

/**
 * @name tcam_device_info
 */
struct tcam_device_info
{
    enum TCAM_DEVICE_TYPE type; /**< type of camera connection */
    char name[128];             /**< Camera name (e.g. DFK 23UP031) */
    char identifier[128];       /**< identifier used for camera interaction (e.g. /dev/video0) */
    char serial_number[64];     /**< unique identifier */
};


struct tcam_image_size
{
    uint32_t width;
    uint32_t height;
};

/**
 * format capabilities
 */
enum TCAM_FRAMERATE_TYPE
{
    TCAM_FRAMERATE_TYPE_RANGE, /**< returned values should be interpreted as boundaries for value range */
    TCAM_FRAMERATE_TYPE_FIXED, /**< only non-negotiable framerates are offered */
};


/**
 * @name tcam_video_format_description
 * @brief generic format description
 */
struct tcam_video_format_description
{
    uint32_t fourcc;                         /**< pixel format that is used e.g. RGB32 or Y800 */
    char description [256];

    struct tcam_image_size min_size;         /**< smallest available resolution */
    struct tcam_image_size max_size;         /**< biggest available resolution */

    enum TCAM_FRAMERATE_TYPE framerate_type; /**< type of available framerates*/
};


/**
 * @name tcam_video_format
 * @brief description of a specific video format
 */
struct tcam_video_format
{
    uint32_t fourcc;  /**< pixel format that is used e.g. RGB32 or Y800 */

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
    unsigned int             length;   /**< size if image buffer in bytes */
    struct tcam_video_format format;   /**< tcam_video_format the image buffer has */
    unsigned int             pitch;    /**< length of single image line in bytes */
    struct tcam_stream_statistics statistics;
};


enum TCAM_PROPERTY_ID
{
    TCAM_PROPERTY_INVALID,
    TCAM_PROPERTY_EXPOSURE,
    TCAM_PROPERTY_EXPOSURE_AUTO,
    TCAM_PROPERTY_GAIN,
    TCAM_PROPERTY_GAIN_RED,
    TCAM_PROPERTY_GAIN_GREEN,
    TCAM_PROPERTY_GAIN_BLUE,
    TCAM_PROPERTY_GAIN_AUTO,
    TCAM_PROPERTY_TRIGGER_MODE,
    TCAM_PROPERTY_TRIGGER_SOURCE,
    TCAM_PROPERTY_TRIGGER_ACTIVATION,
    TCAM_PROPERTY_SOFTWARETRIGGER,
    TCAM_PROPERTY_GPIO,
    TCAM_PROPERTY_GPIN,
    TCAM_PROPERTY_GPOUT,
    TCAM_PROPERTY_OFFSET_X,
    TCAM_PROPERTY_OFFSET_Y,
    TCAM_PROPERTY_OFFSET_AUTO,
    TCAM_PROPERTY_BRIGHTNESS,
    TCAM_PROPERTY_CONTRAST,
    TCAM_PROPERTY_SATURATION,
    TCAM_PROPERTY_HUE,
    TCAM_PROPERTY_GAMMA,
    TCAM_PROPERTY_WB,
    TCAM_PROPERTY_WB_AUTO,
    TCAM_PROPERTY_WB_RED,
    TCAM_PROPERTY_WB_GREEN,
    TCAM_PROPERTY_WB_BLUE,
    TCAM_PROPERTY_IRCUT,
    TCAM_PROPERTY_IRIS,
    TCAM_PROPERTY_IRIS_AUTO,
    TCAM_PROPERTY_FOCUS,
    TCAM_PROPERTY_FOCUS_AUTO,
    TCAM_PROPERTY_FOCUS_ONE_PUSH,
    TCAM_PROPERTY_ZOOM,
    TCAM_PROPERTY_STROBE_ENABLE,
    TCAM_PROPERTY_SKIPPING,
    TCAM_PROPERTY_BINNING,
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
    TCAM_PROPERTY_TYPE_STRING_TABLE = 5,
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
    double step;         /* 0 if steps not possible */
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
};


/**
 * @struct tcam_property_group
 * grouping description for properties
 */
struct tcam_property_group
{
    enum TCAM_PROPERTY_CATEGORY property_category; /**< category of the property */
    enum TCAM_PROPERTY_ID       property_group;    /**< group of the property
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
    enum TCAM_PROPERTY_ID id;          /**< unique identifier */
    char name [32];                    /**< string identifier */

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
