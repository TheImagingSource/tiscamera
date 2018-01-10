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

#include "v4l2_property_mapping.h"
#include "utils.h"
#include "logging.h"

#if HAVE_UDEV
#include <libudev.h>
#endif

#include <glob.h>

#include <vector>
#include <algorithm>

using namespace tcam;


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


TCAM_PROPERTY_ID tcam::find_v4l2_mapping (int v4l2_id)
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
        case V4L2_CTRL_TYPE_MENU:
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

    auto prop_id = find_v4l2_mapping (ctrl->id);

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
