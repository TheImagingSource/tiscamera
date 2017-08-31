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

#include "v4l2_utils.h"

#include "standard_properties.h"
#include "v4l2_uvc_identifier.h"

#include "utils.h"
#include "logging.h"

#if HAVE_UDEV
#include <libudev.h>
#endif

#include <glob.h>

#include <vector>
#include <algorithm>

using namespace tcam;

struct v4l2_property
{
    TCAM_PROPERTY_ID id;
    std::vector<int> v4l2_id;
};


static const std::vector<struct v4l2_property> v4l2_mappings =
{
    {
        .id = TCAM_PROPERTY_INVALID,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE,
        .v4l2_id = { V4L2_CID_EXPOSURE_ABSOLUTE, V4L2_CID_EXPOSURE, TCAM_V4L2_EXPOSURE_TIME_US },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO,
        .v4l2_id = { V4L2_CID_AUTO_EXPOSURE_BIAS, 0x009a0901, 0x0199e202},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        .v4l2_id = { 0x0199e203},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        .v4l2_id = {0x199e254},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        .v4l2_id = {0x199e256},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        .v4l2_id = {0x199e255},
    },
    {
        .id = TCAM_PROPERTY_GAIN,
        .v4l2_id = { V4L2_CID_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_RED,
        .v4l2_id = { 0x980921, TCAM_V4L2_CID_EUVC_GAIN_R },
    },
    {
        .id = TCAM_PROPERTY_GAIN_GREEN,
        .v4l2_id = {  0x980922, TCAM_V4L2_CID_EUVC_GAIN_G },
    },
    {
        .id = TCAM_PROPERTY_GAIN_BLUE,
        .v4l2_id = {  0x980923, TCAM_V4L2_CID_EUVC_GAIN_B },
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO,
        .v4l2_id = { 0x0199e205 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MODE,
        .v4l2_id = { V4L2_CID_PRIVACY, 0x0199e208, 0x980924},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_SOURCE,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_ACTIVATION,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_SOFTWARETRIGGER,
        .v4l2_id = {/* usb 2: */ 0x980926, /* usb 3: */ 0x0199e209},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DELAY,
        .v4l2_id = {0x199e210},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_POLARITY,
        .v4l2_id = { 0x0199e234 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_EXPOSURE_MODE,
        .v4l2_id = { 0x0199e236 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        .v4l2_id = { 0x0199e237 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DEBOUNCE_TIME_US,
        .v4l2_id = { 0x0199e238 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MASK_TIME_US,
        .v4l2_id = { 0x0199e239 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_NOISE_SURPRESSION_TIME_US,
        .v4l2_id = { 0x0199e240 },
    },


    {
        .id = TCAM_PROPERTY_WB_PRESET,
        .v4l2_id = {/* usb 3: */ 0x0199e207},
    },
    {
        .id = TCAM_PROPERTY_GPIO,
        .v4l2_id = {/* usb 2: */ 0x980920, /* usb 3: */ 0x0199e217},
    },
    {
        .id = TCAM_PROPERTY_GPIN,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_GPOUT,
        .v4l2_id = {0x0199e216},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_X,
        .v4l2_id = {0x00980927 /*usb2*/, 0x0199e218 /*usb3*/},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_Y,
        .v4l2_id = {0x00980928 /*usb2*/, 0x0199e219/*usb3*/},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_AUTO,
        .v4l2_id = {0x0199e220},
    },
    {
        .id = TCAM_PROPERTY_BRIGHTNESS,
        .v4l2_id = {V4L2_CID_BRIGHTNESS},
    },
    {
        .id = TCAM_PROPERTY_CONTRAST,
        .v4l2_id = {V4L2_CID_CONTRAST},
    },
    {
        .id = TCAM_PROPERTY_SATURATION,
        .v4l2_id = {V4L2_CID_SATURATION},
    },
    {
        .id = TCAM_PROPERTY_HUE,
        .v4l2_id = {V4L2_CID_HUE},
    },
    {
        .id = TCAM_PROPERTY_GAMMA,
        .v4l2_id = {V4L2_CID_GAMMA},
    },
    {
        .id = TCAM_PROPERTY_WB,
        .v4l2_id = { 0x199e246 },
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO,
        .v4l2_id = {0x0098090c},
    },
    {
        .id = TCAM_PROPERTY_WB_RED,
        .v4l2_id = {0x0098090e},
    },
    {
        .id = TCAM_PROPERTY_WB_GREEN,
        .v4l2_id = { 0x199e248 },
    },
    {
        .id = TCAM_PROPERTY_WB_BLUE,
        .v4l2_id = {0x0098090f},
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO_PRESET,
        .v4l2_id = { 0x199e247 },
    },
    {
        .id = TCAM_PROPERTY_WB_TEMPERATURE,
        .v4l2_id = { 0x199e250 },
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        .v4l2_id = { 0x199e249 },
    },
    {
        .id = TCAM_PROPERTY_IRCUT,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_IRIS,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_FOCUS,
        .v4l2_id = {V4L2_CID_FOCUS_ABSOLUTE},
    },
    {
        .id = TCAM_PROPERTY_ZOOM,
        .v4l2_id = {V4L2_CID_ZOOM_ABSOLUTE},
    },
    {
        .id = TCAM_PROPERTY_FOCUS_AUTO,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_FOCUS_ONE_PUSH,
        .v4l2_id = {0x199e206},
    },
    {
        .id = TCAM_PROPERTY_STROBE_ENABLE,
        .v4l2_id = {0x0199e211},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DELAY,
        .v4l2_id = {0x199e215},
    },
    {
        .id = TCAM_PROPERTY_STROBE_POLARITY,
        .v4l2_id = {0x199e212},
    },
    {
        .id = TCAM_PROPERTY_STROBE_EXPOSURE,
        .v4l2_id = {0x199e213},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DURATION,
        .v4l2_id = {0x199e214},
    },
    {
        .id = TCAM_PROPERTY_BINNING,
        .v4l2_id = { 0x980925, TCAM_V4L2_CID_BINNING },
    },
    {
        .id = TCAM_PROPERTY_SKIPPING,
        .v4l2_id = { 0x980929, TCAM_V4L2_CID_SKIP },
    },
    {
        .id = TCAM_PROPERTY_SHARPNESS,
        .v4l2_id = { 0x0098091b},
    },
    {
        .id = TCAM_PROPERTY_NOISE_REDUCTION,
        .v4l2_id = { 0x199e232 },
    },
    {
        .id = TCAM_PROPERTY_FACE_DETECTION,
        .v4l2_id = { 0x199e233},
    },
    {
        .id = TCAM_PROPERTY_IMAGE_STABILIZATION,
        .v4l2_id = { 0x199e231 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        .v4l2_id = { 0x199e241 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        .v4l2_id = { 0x199e242 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        .v4l2_id = { 0x199e243 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        .v4l2_id = { 0x199e244 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        .v4l2_id = { 0x199e245 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        .v4l2_id = { 0x199e2510 },
    },
    {
        .id = TCAM_PROPERTY_REVERSE_X,
        .v4l2_id = { 0x199e226, 0x199e251 },
    },
    {
        .id = TCAM_PROPERTY_REVERSE_Y,
        .v4l2_id = { 0x199e227, 0x199e252 },
    },
    {
        .id = TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        .v4l2_id = { 0x199e228 },
    },



};


uint32_t tcam::convert_v4l2_flags (uint32_t v4l2_flags)
{
    uint32_t internal_flags = 0;

    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_DISABLED))
    {
        internal_flags = set_bit(internal_flags, TCAM_PROPERTY_FLAG_DISABLED);
    }
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_GRABBED))
    {
        internal_flags = set_bit(internal_flags, TCAM_PROPERTY_FLAG_GRABBED);
    }
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_READ_ONLY))
    {
        internal_flags = set_bit(internal_flags, TCAM_PROPERTY_FLAG_READ_ONLY);
    }
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_UPDATE))
    {}
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_INACTIVE))
    {
        internal_flags = set_bit(internal_flags, TCAM_PROPERTY_FLAG_INACTIVE);
    }
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_SLIDER))
    {}
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_WRITE_ONLY))
    {
        internal_flags = set_bit(internal_flags, TCAM_PROPERTY_FLAG_WRITE_ONLY);
    }
    if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_VOLATILE))
    {}
    // if (is_bit_set(v4l2_flags, V4L2_CTRL_FLAG_HAS_PAYLOAD))
    // {}

    return internal_flags;
}


static TCAM_PROPERTY_ID find_mapping (int v4l2_id)
{
    auto f = [v4l2_id] (int p)
        {
            if (v4l2_id == p)
                return true;
            return false;
        };

    for (const auto& m : v4l2_mappings)
    {
        auto match = std::find_if(m.v4l2_id.begin(), m.v4l2_id.end(), f);

        if (match != m.v4l2_id.end())
            return m.id;
    }
    return TCAM_PROPERTY_INVALID;
}


std::shared_ptr<Property> tcam::create_property (int fd,
                                                 struct v4l2_queryctrl* queryctrl,
                                                 struct v4l2_ext_control* ctrl,
                                                 std::shared_ptr<PropertyImpl> impl)
{

    // assure we have the typ
    Property::VALUE_TYPE type;

    switch (queryctrl->type)
    {
        case V4L2_CTRL_TYPE_BOOLEAN:
        {
            type = Property::BOOLEAN;
            break;
        }
        case V4L2_CTRL_TYPE_INTEGER:
        {
            type = Property::INTEGER;
            break;
        }
        case V4L2_CTRL_TYPE_STRING:
        {
            type = Property::STRING;
            break;
        }
        case V4L2_CTRL_TYPE_INTEGER_MENU:
        {
            type = Property::ENUM;
            break;
        }
        case V4L2_CTRL_TYPE_BUTTON:
        {
            type = Property::BUTTON;
            break;
        }
        default:
        {
            type = Property::UNDEFINED;
            break;
        }
    }

    auto prop_id = find_mapping (ctrl->id);

    auto ctrl_m = get_control_reference(prop_id);

    TCAM_PROPERTY_TYPE type_to_use;
    tcam_device_property cp = {};

    if (ctrl_m.id == TCAM_PROPERTY_INVALID)
    {
        tcam_log(TCAM_LOG_WARNING, "Unable to find std property. Passing raw property identifier through. '%s'(%x)", (char*)queryctrl->name, queryctrl->id);
        // pass through and do not associate with anything existing
        type_to_use = value_type_to_ctrl_type(type);
        memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));
        cp.type = value_type_to_ctrl_type(type);
        // generate id so that identfication of passed through properties is guaranteed
        cp.id = generate_unique_property_id();
    }
    else
    {
        type_to_use = ctrl_m.type_to_use;
        cp = create_empty_property(ctrl_m.id);
    }

    uint32_t flags = convert_v4l2_flags(queryctrl->flags);

    switch (type_to_use)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (queryctrl->default_value == 0)
            {
                cp.value.b.default_value = false;
            }
            else if (queryctrl->default_value > 0)
            {
                cp.value.b.default_value = true;
            }
            else
            {
                tcam_log(TCAM_LOG_ERROR,
                        "Boolean '%s' has impossible default value: %d Setting to false",
                        cp.name,
                        queryctrl->default_value);
                cp.value.b.default_value = false;
            }

            if (ctrl->value == 0)
            {
                cp.value.b.value = false;
            }
            else if (ctrl->value > 0)
            {
                cp.value.b.value = true;
            }
            else
            {
                tcam_log(TCAM_LOG_ERROR,
                        "Boolean '%s' has impossible value: %d Setting to false",
                        cp.name,
                        ctrl->value);
                cp.value.b.value = false;
            }
            cp.flags = flags;

            return std::make_shared<Property>(PropertyBoolean(impl, cp, type));
        }
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            cp.value.i.min = queryctrl->minimum;
            cp.value.i.max = queryctrl->maximum;
            cp.value.i.step = queryctrl->step;

            if (cp.value.i.min > cp.value.i.max)
            {
                tcam_log(TCAM_LOG_ERROR,
                         "Range boundaries for property '%s' are faulty. Ignoring property as a precaution.",
                         cp.name);
                return nullptr;
            }

            if (cp.value.i.step == 0)
            {
                tcam_log(TCAM_LOG_WARNING,
                         "Detected stepsize 0 for property %s. Setting to 1.",
                         cp.name);

                cp.value.i.step = 1;
            }

            cp.value.i.default_value = queryctrl->default_value;
            cp.value.i.value = ctrl->value;
            cp.flags = flags;

            return std::make_shared<Property>(PropertyInteger(impl, cp, type));
        }
        // case TCAM_CTRL_TYPE_DOUBLE:
        // {
        // Does not exist in v4l2
        // }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            memcpy(cp.value.s.value,(char*)queryctrl->name, sizeof(cp.value.s.value));
            memcpy(cp.value.s.default_value, (char*)queryctrl->name, sizeof(cp.value.s.default_value));
            cp.flags = flags;

            return std::make_shared<Property>(PropertyString(impl, cp, type));
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            cp.value.i.min = queryctrl->minimum;
            cp.value.i.max = queryctrl->maximum;
            cp.value.i.step = 0;
            cp.value.i.default_value = queryctrl->default_value;
            cp.value.i.value = ctrl->value;
            cp.flags = flags;

            struct v4l2_querymenu qmenu = {};

            qmenu.id = queryctrl->id;

            std::map<std::string, int> m;

            for (int i = 0; i <= queryctrl->maximum; i++)
            {
                qmenu.index = i;
                if (tcam_xioctl(fd, VIDIOC_QUERYMENU, &qmenu))
                    continue;

                std::string map_string((char*) qmenu.name);
                m.emplace(map_string, i);
            }

            return std::make_shared<Property>(PropertyEnumeration(impl, cp, m, type));
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            cp.flags = flags;

            return std::make_shared<Property>(PropertyButton(impl, cp, type));
        }
        default:
        {
            std::string s = "Unknown V4L2 Control type: ";
            s.append((char*)queryctrl->name);
            tcam_log(TCAM_LOG_ERROR, s.c_str());
            break;
        }
    }
    return nullptr;
}


std::vector<DeviceInfo> tcam::get_v4l2_device_list ()
{
    std::vector<DeviceInfo> device_list;

    struct udev* udev = udev_new();
    if (!udev)
    {
        return device_list;
    }

    /* Create a list of the devices in the 'video4linux' subsystem. */
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* dev_list_entry;

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char* path;
        char needed_path[100];

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/

        /* we need to copy the devnode (/dev/videoX) before the path
           is changed to the path of the usb device behind it (/sys/class/....) */
        strcpy(needed_path, udev_device_get_devnode(dev));

        struct udev_device* parent_device = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        /* skip this device if we can't get the usb parent */
        if (!parent_device)
        {
            continue;
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passed into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the directory which represents
           the USB device. Note that USB strings are Unicode, UCS2
           encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */

        static const char* TCAM_VENDOR_ID_STRING = "199e";

        if (strcmp(udev_device_get_sysattr_value(parent_device, "idVendor"), TCAM_VENDOR_ID_STRING) == 0)
        {
            tcam_device_info info = {};
            info.type = TCAM_DEVICE_TYPE_V4L2;
            strncpy(info.identifier, needed_path, sizeof(info.identifier));

            if (udev_device_get_sysattr_value(parent_device, "idProduct") != NULL)
            {
                strncpy(info.additional_identifier, udev_device_get_sysattr_value(parent_device, "idProduct"), sizeof(info.additional_identifier));
            }

            if (udev_device_get_sysattr_value(parent_device, "product") != NULL)
                strncpy(info.name, udev_device_get_sysattr_value(parent_device, "product"), sizeof(info.name));
            else
                memcpy(info.name, "\0", sizeof(info.name));

            if (udev_device_get_sysattr_value(parent_device, "serial") != NULL)
            {
                std::string tmp = udev_device_get_sysattr_value(parent_device, "serial");
                tmp.erase(remove_if(tmp.begin(), tmp.end(), isspace), tmp.end());
                strncpy(info.serial_number, tmp.c_str(), sizeof(info.serial_number));
            }
            else
            {
                memcpy(info.serial_number, "\0", sizeof(info.serial_number));
            }
            device_list.push_back(DeviceInfo(info));
        }

        udev_device_unref(dev);
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    return device_list;
}
