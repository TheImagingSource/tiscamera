

/**
This file contains basic c types that are used for:
- internal representation
- C API
*/


#ifndef BASE_TYPES_H_
#define BASE_TYPES_H_

#include "image_transform_base.h"

#include <stdint.h>
#include <stdbool.h>


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
    TCAM_DEVICE_TYPE_UNKNOWN = 0,
    TCAM_DEVICE_TYPE_V4L2,
    TCAM_DEVICE_TYPE_FIREWIRE,    /* both 400 and 800 */
    TCAM_DEVICE_TYPE_ARAVIS,      /* currently through aravis */
};

/**
 * @name tcam_device_info
 */
struct tcam_device_info
{
    enum TCAM_DEVICE_TYPE type; ///< type of camera connection
    char name[128];            ///< Camera name (e.g. DFK 23UP031)
    char identifier[128];      ///< identifier used for camera interaction (e.g. /dev/video0)
    char serial_number[64];    ///< unique identifier
};


struct IMG_SIZE
{
    uint32_t width;
    uint32_t height;
};

/**
 * format capabilities
 */
enum TCAM_FRAMERATE_TYPE
{
    TCAM_FRAMERATE_TYPE_RANGE, ///< returned values should be interpreted as boundaries for value range
    TCAM_FRAMERATE_TYPE_FIXED, ///< only non-negotiable framerates are offered
};


/**
 * @name tcam_video_format_description
 * @brief generic format description
 */
struct tcam_video_format_description
{
    uint32_t fourcc;
    char description [256];

    IMG_SIZE min_size;
    IMG_SIZE max_size;

    uint32_t binning;

    enum TCAM_FRAMERATE_TYPE framerate_type;
};


/**
 * @name tcam_video_format
 * @brief description of a specific video format
 */
struct tcam_video_format
{
    uint32_t fourcc;

    uint32_t width;
    uint32_t height;

    uint32_t binning;

    double   framerate;
};


/**
 * Statistic container for additional image_buffer descriptions
 */
struct tcam_stream_statistics
{
    uint64_t frame_count;      //
    uint64_t frames_dropped;   //
    uint64_t capture_time_ns;  // capture time reported by lib
    uint64_t camera_time_ns;   // capture time reported by camera; empty if not supported
    double   framerate;        // in contrast to selected one
};

/**
 * @name tcam_image_buffer
 * @brief container for image transfer
 */
struct tcam_image_buffer
{
    unsigned char*      pData;  /**< pointer to actual image buffer */
    unsigned int        length; /**< size if image buffer in bytes */
    struct tcam_video_format format; /**< tcam_video_format the image buffer has */
    unsigned int        pitch;  /**< length of single image line in bytes */
    struct tcam_stream_statistics statistics;
};


enum PROPERTY_ID
{
    PROPERTY_INVALID,
    PROPERTY_EXPOSURE,
    PROPERTY_EXPOSURE_AUTO,
    PROPERTY_GAIN,
    PROPERTY_GAIN_RED,
    PROPERTY_GAIN_GREEN,
    PROPERTY_GAIN_BLUE,
    PROPERTY_GAIN_AUTO,
    PROPERTY_TRIGGER_MODE,
    PROPERTY_TRIGGER_SOURCE,
    PROPERTY_TRIGGER_ACTIVATION,
    PROPERTY_SOFTWARETRIGGER,
    PROPERTY_GPIO,
    PROPERTY_GPIN,
    PROPERTY_GPOUT,
    PROPERTY_OFFSET_X,
    PROPERTY_OFFSET_Y,
    PROPERTY_OFFSET_AUTO,
    PROPERTY_BRIGHTNESS,
    PROPERTY_CONTRAST,
    PROPERTY_SATURATION,
    PROPERTY_HUE,
    PROPERTY_GAMMA,
    PROPERTY_WB,
    PROPERTY_WB_AUTO,
    PROPERTY_WB_RED,
    PROPERTY_WB_GREEN,
    PROPERTY_WB_BLUE,
    PROPERTY_IRCUT,
    PROPERTY_IRIS,
    PROPERTY_IRIS_AUTO,
    PROPERTY_FOCUS,
    PROPERTY_FOCUS_AUTO,
    PROPERTY_ZOOM,
    PROPERTY_STROBE_ENABLE,
    PROPERTY_SKIPPING,
    PROPERTY_BINNING,
};

/* Property types */

enum TCAM_PROPERTY_TYPE
{
    TCAM_PROPERTY_TYPE_UNKNOWN      = 0,
    TCAM_PROPERTY_TYPE_BOOLEAN      = 1,
    TCAM_PROPERTY_TYPE_INTEGER      = 2,
    TCAM_PROPERTY_TYPE_DOUBLE       = 3,
    TCAM_PROPERTY_TYPE_STRING       = 4,
    TCAM_PROPERTY_TYPE_STRING_TABLE = 5,
    /* the button type is just a command to trigger some functionality */
    /* which doesn't care about parameters because that is actually not necessary. */
    /* For instance, "adjusting white balance" button in digital camera could */
    /* be a good example which is performing adjustment of white balance for */
    /* one time and no need for any kind of parameter for this but needs to */
    /* be triggered. */
    TCAM_PROPERTY_TYPE_BUTTON       = 6,
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
 * @struct tcam_camera_capability
 * @brief unified property description
 */
struct camera_property
{
    PROPERTY_ID id;
    char name [32];              /* unique string identifier */
    enum TCAM_PROPERTY_TYPE type;     /* type identification */

    union
    {
        struct tcam_value_int i;
        struct tcam_value_double d;
        struct tcam_value_string s;
        struct tcam_value_bool b;
    } value; ///< actual value settings

    uint32_t flags;             ///< bit flags
};

enum TCAM_PROPERTY_FLAGS
{
    /* This control is permanently disabled and
       should be ignored by the application. Any
       attempt to change the control will
       result in an EINVAL error code. */
    TCAM_ROPERTY_FLAG_DISABLED = 0x0001,
    /*This control is temporarily unchangeable,
      for example because another application
      took over control of the respective
      resource. Such controls may be displayed
      specially in a user interface. Attempts to
      change the control may result in an EBUSY error code. */
    TCAM_PROPERTY_FLAG_GRABBED = 0x0002,
    /* This control is permanently readable only.
       Any attempt to change the control will result
       in an EINVAL error code. */
    TCAM_PROPERTY_FLAG_READ_ONLY = 0x0004,
    /* This control is realized through library code and is
       not available in the camera */
    TCAM_PROPERTY_FLAG_EXTERNAL = 0x0008,
    /* This control is not applicable to the
       current configuration and should be displayed
       accordingly in a user interface. For example
       the flag may be set on a MPEG audio level 2
       bitrate control when MPEG audio encoding
       level 1 was selected with another control. */
    TCAM_PROPERTY_FLAG_INACTIVE = 0x0010,
    /* This control is permanently writable only.
       Any attempt to read the control will result
       in an EACCES error code error code. This flag
       is typically present for relative controls
       or action controls where writing a value will
       cause the device to carry out a given action
       (e. g. motor control) but no meaningful value can be returned. */
    TCAM_PROPERTY_FLAG_WRITE_ONLY = 0x0020,
};

#endif /* BASE_TYPES_H_ */
