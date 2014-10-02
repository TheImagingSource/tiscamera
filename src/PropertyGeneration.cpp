
#include "PropertyGeneration.h"

#include "base_types.h"
#include "tis_utils.h"
#include "tis_logging.h"

#include "standard_properties.h"

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


    uint32_t id = ctrl->id;

    auto find_control_ref = [id] (const struct control_reference& ref)
        {
            // id comparison
            auto id_comp = [id] (const uint32_t& i)
            {
                return (id == i);
            };

            if (std::find_if(ref.v4l2_id.begin(), ref.v4l2_id.end(), id_comp) != ref.v4l2_id.end())
            {
                return true;
            }
            return false;
        };

    auto ctrl_m = std::find_if( ctrl_reference_table.begin(),
                                                     ctrl_reference_table.end(),
                                                     find_control_ref);

    PROPERTY_TYPE type_to_use;
    camera_property cp = {};

    if (ctrl_m == ctrl_reference_table.end())
    {
        tis_log(TIS_LOG_WARNING, "Unable to find std property. Passing raw property identifier through. '%s'(%x)", (char*)queryctrl->name, queryctrl->id);
        // pass through and do not associate with anything existing
        type_to_use = value_type_to_ctrl_type(type);
        memcpy(cp.name, (char*)queryctrl->name, sizeof(cp.name));
        cp.type = value_type_to_ctrl_type(type);
    }
    else
    {
        type_to_use = ctrl_m->type_to_use;
        memcpy(cp.name, ctrl_m->name.c_str(), ctrl_m->name.size());
        cp.type = ctrl_m->type_to_use;
    }

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
            if (queryctrl->default_value == 0)
            {
                cp.value.b.default_value = false;
            }
            else if (queryctrl->default_value == 1)
            {
                cp.value.b.default_value = true;
            }
            else
            {
                tis_log(TIS_LOG_ERROR,
                        "Boolean '%s' has impossible default value: %d Setting to false",
                        cp.name,
                        queryctrl->default_value);
                cp.value.b.default_value = false;
            }

            if (ctrl->value == 0)
            {
                cp.value.b.value = false;
            }
            else if (ctrl->value == 1)
            {
                cp.value.b.value = true;
            }
            else
            {
                tis_log(TIS_LOG_ERROR,
                        "Boolean '%s' has impossible value: %d Setting to false",
                        cp.name,
                        ctrl->value);
                cp.value.b.value = false;
            }
            cp.flags = flags;

            return std::make_shared<Property>(PropertySwitch(impl, cp, type));
        }
        case PROPERTY_TYPE_INTEGER:
        {
            cp.value.i.min = queryctrl->minimum;
            cp.value.i.max = queryctrl->maximum;
            cp.value.i.step = queryctrl->step;
            cp.value.i.default_value = queryctrl->default_value;
            cp.value.i.value = ctrl->value;
            cp.flags = flags;

            return std::make_shared<Property>(PropertyInteger(impl, cp, type));
        }
        // case TIS_CTRL_TYPE_DOUBLE:
        // {
        // Does not exist in v4l2
        // }
        case PROPERTY_TYPE_STRING:
        {
            memcpy(cp.value.s.value,(char*)queryctrl->name, sizeof(cp.value.s.value));
            memcpy(cp.value.s.default_value, (char*)queryctrl->name, sizeof(cp.value.s.default_value));
            cp.flags = flags;

            return std::make_shared<Property>(PropertyString(impl, cp, type));
        }
        case PROPERTY_TYPE_STRING_TABLE:
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
                if (tis_xioctl(fd, VIDIOC_QUERYMENU, &qmenu))
                    continue;

                std::string map_string((char*) qmenu.name);
                m.emplace(map_string, i);
            }

            return std::make_shared<Property>(PropertyStringMap(impl, cp, m, type));
        }
        case PROPERTY_TYPE_BUTTON:
        {
            cp.flags = flags;

            return std::make_shared<Property>(PropertyButton(impl, cp, type));
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



#if HAVE_ARAVIS

std::shared_ptr<Property> tis_imaging::createProperty (ArvCamera* camera,
                                                       ArvGcNode* node,
                                                       std::shared_ptr<PropertyImpl> impl)
{
    // Map genicam feature to library feature
    // Check if an existing control should be used
    // if no control was found simply pass the genicam feature through

    const char* feature = arv_gc_feature_node_get_name ((ArvGcFeatureNode*) node);

    Property::VALUE_TYPE type;

    const char* node_type = arv_dom_node_get_node_name (ARV_DOM_NODE (node));

    if (strcmp(node_type, "Integer") == 0)
    {
        type = Property::INTEGER;
    }
    else if (strcmp(node_type, "IntSwissKnife") == 0)
    {
        type = Property::INTSWISSKNIFE;
    }
    else if (strcmp(node_type, "Float") == 0)
    {
        type = Property::FLOAT;
    }
    else if (strcmp(node_type, "Boolean") == 0)
    {
        type = Property::BOOLEAN;
    }
    else if (strcmp(node_type, "Command") == 0)
    {
        type = Property::COMMAND;
    }
    else if (strcmp(node_type, "Enumeration") == 0)
    {
        type = Property::ENUM;
    }
    else
    {
        tis_log(TIS_LOG_ERROR, "%s has unknown node type '%s'", feature, node_type);
        // TODO ERROR
    }


    // tis_log(TIS_LOG_DEBUG, "'%s' has node type '%s'", feature, node_type);

    // check if this node should be mapped to a known control by comparing its name
    // with a list of known namens
    auto find_control_ref = [feature] (const struct control_reference& ref)
        {
            //
            auto id_comp = [feature] (const std::string& s)
            {
                return (s.compare(feature) == 0);
            };

            if (std::find_if(ref.genicam_name.begin(), ref.genicam_name.end(), id_comp) != ref.genicam_name.end())
            {
                return true;
            }
            return false;
        };

    auto ctrl_m = std::find_if( ctrl_reference_table.begin() ,
                                ctrl_reference_table.end(),
                                find_control_ref);

    // type which the control shall use
    PROPERTY_TYPE type_to_use;
    camera_property prop = {};

    if (ctrl_m == ctrl_reference_table.end())
    {
        tis_log(TIS_LOG_WARNING, "Unable to find std property. Passing raw property identifier through. %s", feature);
        // pass through and do not associate with anything existing
        // TODO LOG
        type_to_use = value_type_to_ctrl_type(type);
        memcpy(prop.name, feature, sizeof(prop.name));
        prop.type = value_type_to_ctrl_type(type);
    }
    else
    {
        type_to_use = ctrl_m->type_to_use;
        memcpy(prop.name, ctrl_m->name.c_str(), ctrl_m->name.size());
        prop.type = ctrl_m->type_to_use;
    }

    if (ARV_IS_GC_ENUMERATION (node))
    {
        const GSList* children;
        const GSList* iter;

        children = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (node));

        std::map<std::string, int> var;

        for (iter = children; iter != NULL; iter = iter->next)
        {
            if (arv_gc_feature_node_is_implemented ((ArvGcFeatureNode*)iter->data, NULL))
            {
                if (strcmp(arv_dom_node_get_node_name ((ArvDomNode*)iter->data), "EnumEntry") == 0)
                {
                    // TODO should enums be stored in separate vector
                    var.emplace(arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), var.size());
                }
            }
        }

        return std::make_shared<PropertyStringMap>(impl, prop, var, type);
    }


    // const char* node_type = arv_dom_node_get_node_name (ARV_DOM_NODE (node));

    // TODO TEST IF CONTROL IS USER VISIBLE

    if (strcmp(node_type, "Integer") == 0)
    {
        // IN/OUT types are identical
        if (type_to_use == PROPERTY_TYPE_INTEGER)
        {
            //camera_property prop = {};
            prop.type = PROPERTY_TYPE_INTEGER;
            prop.value.i.value = arv_device_get_integer_feature_value(arv_camera_get_device(camera), feature);
            prop.value.i.default_value = prop.value.i.value;

            prop.value.i.step = 1;

            arv_device_get_integer_feature_bounds(arv_camera_get_device(camera),
                                                  feature,
                                                  &prop.value.i.min,
                                                  &prop.value.i.max);

            return std::make_shared<PropertyInteger>(impl, prop, type);

        }
        else if (type_to_use == PROPERTY_TYPE_DOUBLE)
        {

            prop.type = PROPERTY_TYPE_DOUBLE;
            prop.value.d.value = arv_device_get_integer_feature_value(arv_camera_get_device(camera), feature);
            prop.value.d.default_value = prop.value.i.value;


            prop.value.d.step = 1.0;

            int64_t min, max;
            arv_device_get_integer_feature_bounds(arv_camera_get_device(camera), feature, &min, &max);

            prop.value.d.min = min;
            prop.value.d.max = max;

            return std::make_shared<PropertyDouble>(impl, prop, type);

            // TODO: implement
            tis_log(TIS_LOG_ERROR, "Trying to map int to double. not implemented.");
            return nullptr;
        }
        else
        {
            tis_log(TIS_LOG_ERROR, "\n\nBAD");
        }

        // ctrl.value.i.value = arv_device_get_integer_feature_value(arv_camera_get_device(camera), feature);

        // arv_device_get_integer_feature_bounds(arv_camera_get_device(camera),
        //                                       feature,
        //                                       &ctrl.value.i.min,
        //                                       &ctrl.value.i.max);
        // ctrl.value.i.step = 0;
        // ctrl.value.i.default_value = ctrl.value.i.value;
    }
    else if (strcmp(node_type, "IntSwissKnife") == 0)
    {
        // printf ("%s: '%s'%s\n",
        //         arv_dom_node_get_node_name (ARV_DOM_NODE (node)),
        //         feature,
        //         arv_gc_feature_node_is_available (ARV_GC_FEATURE_NODE (node), NULL) ? "" : " (Not available)");

    }
    else if (strcmp(node_type, "Float") == 0)
    {
        if (type_to_use == PROPERTY_TYPE_INTEGER)
        {
            prop.type = PROPERTY_TYPE_INTEGER;
            prop.value.i.value = arv_device_get_float_feature_value(arv_camera_get_device(camera), feature);
            prop.value.i.default_value = prop.value.i.value;


            prop.value.i.step = 1;

            double min, max;
            arv_device_get_float_feature_bounds(arv_camera_get_device(camera), feature, &min, &max);

            prop.value.i.min = min;
            prop.value.i.max = max;

            return std::make_shared<PropertyInteger>(impl, prop, type);
        }
    }
    else if (strcmp(node_type, "Boolean") == 0)
    {
        // struct camera_property ctrl = {};
        prop.type = PROPERTY_TYPE_BOOLEAN;

        memcpy(prop.name, feature, sizeof(prop.name));
        prop.value.i.max = 1;
        prop.value.i.min = 0;
        prop.value.i.step = 1;
        // // TODO: default value

        return std::make_shared<PropertySwitch>(impl, prop, type);

    }
    else if (strcmp(node_type, "Enumeration") == 0)
    {

        if (type_to_use == PROPERTY_TYPE_BOOLEAN)
        {

            // c_map.control.value.i.value = 0;
            // c_map.control.value.i.min = 0;
            // c_map.control.value.i.max = c_map.values.size() - 1; // index begins with 0
            return nullptr;

        }
        else if (type_to_use == PROPERTY_TYPE_STRING_TABLE)
        {
            // printf ("??? %s: '%s'%s\n",
            //         arv_dom_node_get_node_name (ARV_DOM_NODE (node)),
            //         feature,
            //         arv_gc_feature_node_is_available (ARV_GC_FEATURE_NODE (node), NULL) ? "" : " (Not available)");

            // c_map.control.value.i.value = 0;
            // c_map.control.value.i.min = 0;
            // c_map.control.value.i.max = c_map.values.size() - 1; // index begins with 0
        }

        // IN/OUT types are identical
        if (type_to_use == PROPERTY_TYPE_DOUBLE)
        {

            // c_map.control.value.d.value = arv_device_get_float_feature_value(arv_camera_get_device(camera), feature);
            // c_map.control.value.d.default_value = c_map.control.value.d.value;
            // c_map.control.value.d.step = 0;
            // arv_device_get_float_feature_bounds(arv_camera_get_device(camera),
            //                                     feature,
            //                                     &c_map.control.value.d.min,
            //                                     &c_map.control.value.d.max);

        }
        else if (type_to_use == PROPERTY_TYPE_INTEGER)
        {

            // c_map.control.value.i.value = arv_device_get_float_feature_value(arv_camera_get_device(camera), feature);
            // c_map.control.value.i.default_value = c_map.control.value.i.value;
            // c_map.control.value.i.step = 1;

            // double min = 0.0;
            // double max = 0.0;
            // arv_device_get_float_feature_bounds(arv_camera_get_device(camera),
            //                                     feature,
            //                                     &min,
            //                                     &max);

            // c_map.control.value.i.min = round(min);
            // c_map.control.value.i.max = round(max);

        }
        else
        {}
    }
    else if (strcmp(node_type, "Command") == 0)
    {
        // ignore them

        // printf ("== %s: '%s' (ignored)\n",
        //         arv_dom_node_get_node_name (ARV_DOM_NODE (node)),
        //         feature);


        if (type_to_use == PROPERTY_TYPE_BUTTON)
        {
            prop.type = PROPERTY_TYPE_BUTTON;

            return std::make_shared<PropertyButton>(impl, prop, type);
        }
        else
        {
            tis_log(TIS_LOG_DEBUG,"SHIT");
        }
    }
    else
    {
        tis_log(TIS_LOG_WARNING, "Unknown Control '%s'", feature);
    }

    return nullptr;

}

#endif /* HAVE_ARAVIS */

