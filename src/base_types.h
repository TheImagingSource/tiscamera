

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
* @name PIPELINE_STATUS
* @brief overview over possible pipeline states
*/
enum PIPELINE_STATUS
{
    PIPELINE_UNDEFINED = 0,
    PIPELINE_STOPPED,
    PIPELINE_PAUSED,
    PIPELINE_PLAYING,
    PIPELINE_ERROR,

};





/**
 * Supported camera types
 */
enum TIS_DEVICE_TYPE
{
    TIS_DEVICE_TYPE_UNKNOWN = 0,
    TIS_DEVICE_TYPE_V4L2,
    TIS_DEVICE_TYPE_FIREWIRE,    /* both 400 and 800 */
    TIS_DEVICE_TYPE_GIGE,        /* currently through aravis */
};

/**
 * @name tis_device_info
 */
struct tis_device_info
{
    enum TIS_DEVICE_TYPE type; ///< type of camera connection
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
enum TIS_FRAMERATE_TYPE
{
    TIS_FRAMERATE_TYPE_RANGE, ///< returned values should be interpreted as boundaries for value range
    TIS_FRAMERATE_TYPE_FIXED, ///< only non-negotiable framerates are offered
};


/**
 * @name video_format_description
 * @brief generic format description
 */
struct video_format_description
{
    uint32_t fourcc;
    char description [256];

    IMG_SIZE min_size;
    IMG_SIZE max_size;

    uint32_t binning;

    enum TIS_FRAMERATE_TYPE framerate_type;
};


/**
 * @name video_format
 * @brief description of a specific video format
 */
struct video_format
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
struct stream_statistics
{
    uint64_t frame_count;      //
    uint64_t frames_dropped;   //
    uint64_t capture_time_ns;  // capture time reported by lib
    uint64_t camera_time_ns;   // capture time reported by camera
    double   actual_framerate; // in contrast to selected one
};

/**
 * @name image_buffer
 * @brief container for image transfer
 */
struct image_buffer
{
    unsigned char*      pData;  /**< pointer to actual image buffer */
    unsigned int        length; /**< size if image buffer in bytes */
    struct video_format format; /**< video_format the image buffer has */
    unsigned int        pitch;  /**< length of single image line in bytes */
};


/* Property types */

enum PROPERTY_TYPE
{
    PROPERTY_TYPE_UNKNOWN      = 0,
    PROPERTY_TYPE_BOOLEAN      = 1,
    PROPERTY_TYPE_INTEGER      = 2,
    PROPERTY_TYPE_DOUBLE       = 3,
    PROPERTY_TYPE_STRING       = 4,
    PROPERTY_TYPE_STRING_TABLE = 5,
    /* the button type is just a command to trigger some functionality */
    /* which doesn't care about parameters because that is actually not necessary. */
    /* For instance, "adjusting white balance" button in digital camera could */
    /* be a good example which is performing adjustment of white balance for */
    /* one time and no need for any kind of parameter for this but needs to */
    /* be triggered. */
    PROPERTY_TYPE_BUTTON       = 6,
    PROPERTY_TYPE_BITMASK      = 7,
};


struct tis_value_int
{
    int64_t min;
    int64_t max;
    int64_t step;         /* 0 if steps not possible */
    int64_t default_value;
    int64_t value;
};


struct tis_value_double
{
    double min;
    double max;
    double step;         /* 0 if steps not possible */
    double default_value;
    double value;
};


struct tis_value_string
{
    char value[64];
    char default_value[64];
};


struct tis_value_bool
{
    bool value;
    bool default_value;
};

/**
 * @struct tis_camera_capability
 * @brief unified property description
 */
struct camera_property
{
    char name [128];             /* unique string identifier */
    enum PROPERTY_TYPE type;     /* type identification */

    union
    {
        struct tis_value_int i;
        struct tis_value_double d;
        struct tis_value_string s;
        struct tis_value_bool b;
    } value; ///< actual value settings

    uint32_t flags;             ///< bit flags
};

/* V4L2_CTRL_FLAG_DISABLED	0x0001	This control is permanently disabled and
   should be ignored by the application. Any
   attempt to change the control will
   result in an EINVAL error code. */
#define PROPERTY_FLAG_DISABLED      0x0001
/* V4L2_CTRL_FLAG_GRABBED	0x0002	This control is temporarily unchangeable,
   for example because another application
   took over control of the respective
   resource. Such controls may be displayed
   specially in a user interface. Attempts to
   change the control may result in an EBUSY error code. */
#define PROPERTY_FLAG_GRABBED       0x0002
/* V4L2_CTRL_FLAG_READ_ONLY	0x0004	This control is permanently readable only.
   Any attempt to change the control will result
   in an EINVAL error code. */
#define PROPERTY_FLAG_READ_ONLY     0x0004
/* OWN FLAG                         This control is realized through library code and is
   not available in the camera */
#define PROPERTY_FLAG_EXTERNAL      0x0008
/* V4L2_CTRL_FLAG_INACTIVE	0x0010	This control is not applicable to the
   current configuration and should be displayed
   accordingly in a user interface. For example
   the flag may be set on a MPEG audio level 2
   bitrate control when MPEG audio encoding
   level 1 was selected with another control. */
#define PROPERTY_FLAG_INACTIVE      0x0010
/* V4L2_CTRL_FLAG_WRITE_ONLY 0x0040	This control is permanently writable only.
   Any attempt to read the control will result
   in an EACCES error code error code. This flag
   is typically present for relative controls
   or action controls where writing a value will
   cause the device to carry out a given action
   (e. g. motor control) but no meaningful value can be returned. */
#define PROPERTY_FLAG_WRITE_ONLY    0x0020


#endif /* BASE_TYPES_H_ */
