
#include "PropertyGeneration.h"

#include "base_types.h"
#include "tis_utils.h"
#include "tis_logging.h"

// #include "user_properties.h"

#include <linux/videodev2.h>
#include <cstring>
#include <algorithm>

using namespace tis_imaging;



PROPERTY_TYPE value_type_to_ctrl_type (const Property::VALUE_TYPE& t)
{
    switch (t)
    {
        case Property::BOOLEAN:
            return PROPERTY_TYPE_BOOLEAN;
        case Property::STRING:
            return PROPERTY_TYPE_STRING;
        case Property::ENUM:
            return PROPERTY_TYPE_STRING_TABLE;
        case Property::INTSWISSKNIFE:
        case Property::INTEGER:
            return PROPERTY_TYPE_INTEGER;
        case Property::FLOAT:
            return PROPERTY_TYPE_DOUBLE;
        case Property::BUTTON:
            return PROPERTY_TYPE_BUTTON;
        case Property::COMMAND:
        default:
            return PROPERTY_TYPE_UNKNOWN;
    };
}


std::shared_ptr<Property> tis_imaging::createProperty (int fd,
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
            // TODO error
            type = Property::UNDEFINED;
            break;
        }
    }

    PROPERTY_TYPE type_to_use;

    type_to_use = value_type_to_ctrl_type(type);

    uint32_t flags;
    // simply copy existing flags
    if (queryctrl->flags)
    {
        flags = queryctrl->flags;
    }

    switch (type_to_use)
    {
        case PROPERTY_TYPE_BOOLEAN:
        {
            camera_property cp = {};
            memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));
            cp.type = PROPERTY_TYPE_BOOLEAN;
            cp.value.i.min = 0;
            cp.value.i.max = 1;
            cp.value.i.step = 1;
            cp.value.i.default_value = queryctrl->default_value;
            cp.value.i.value = ctrl->value;
            cp.flags = flags;
            return std::make_shared<Property>(PropertySwitch(impl, cp));

        }
        case PROPERTY_TYPE_INTEGER:
        {
            camera_property cp = {};
            memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));

            cp.type = PROPERTY_TYPE_INTEGER;
            cp.value.i.min = queryctrl->minimum;
            cp.value.i.max = queryctrl->maximum;
            cp.value.i.step = queryctrl->step;
            cp.value.i.default_value = queryctrl->default_value;
            cp.value.i.value = ctrl->value;
            cp.flags = flags;

            return std::make_shared<Property>(PropertyInteger(impl, cp));
        }
        // case TIS_CTRL_TYPE_DOUBLE:
        // {
        // Does not exist in v4l2
        // }
        case PROPERTY_TYPE_STRING:
        {
            camera_property cp = {};
            memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));
            cp.type = PROPERTY_TYPE_STRING;
            memcpy(cp.value.s.value,(char*)queryctrl->name, sizeof(cp.value.s.value));
            memcpy(cp.value.s.default_value, (char*)queryctrl->name, sizeof(cp.value.s.default_value));
            cp.flags = flags;

            return std::make_shared<Property>(PropertyString(impl, cp));
        }
        case PROPERTY_TYPE_STRING_TABLE:
        {
            camera_property cp = {};

            memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));
            cp.type = PROPERTY_TYPE_STRING_TABLE;
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
                if (tis_xioctl(fd, VIDIOC_QUERYMENU, &qmenu))
                    continue;

                std::string map_string((char*) qmenu.name);
                m.emplace(map_string, i);
            }

            return std::make_shared<Property>(PropertyStringMap(impl, cp, m));
        }
        case PROPERTY_TYPE_BUTTON:
        {
            camera_property cp = {};
            memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));

            cp.type = PROPERTY_TYPE_BUTTON;
            cp.flags = flags;

            return std::make_shared<Property>(PropertyButton(impl, cp));
        }
        default:
        {
            tis_log(TIS_LOG_ERROR, "Unknown V4L2 Control type. %s", (char*)queryctrl->name);
            // break;
            // TODO: exception
        }
    }
    return nullptr;
}

