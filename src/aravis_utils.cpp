
#include "aravis_utils.h"

#include "standard_properties.h"
#include "logging.h"

#include <algorithm>
#include <vector>

using namespace tcam;

struct aravis_property
{
    TCAM_PROPERTY_ID id;
    std::vector<std::string> genicam_name; // list of genicam identifiers that shall be mapped to control
};


static std::vector<struct aravis_property> aravis_mappings =
{
    {
        .id = TCAM_PROPERTY_INVALID,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE,
        .genicam_name = {"ExposureTime"},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO,
        .genicam_name = {"ExposureAuto"},
    },
    {
        .id = TCAM_PROPERTY_GAIN,
        .genicam_name = {"Gain"},
    },
    {
        .id = TCAM_PROPERTY_GAIN_RED,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_GAIN_GREEN,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_GAIN_BLUE,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MODE,
        .genicam_name = {"TriggerMode"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_SOURCE,
        .genicam_name = {"TriggerSource"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_ACTIVATION,
        .genicam_name = {"TriggerActivation"},
    },
    {
        .id = TCAM_PROPERTY_SOFTWARETRIGGER,
        .genicam_name = {"TriggerSoftware"},
    },
    {
        .id = TCAM_PROPERTY_GPIO,
        .genicam_name = {"GPIO"},
    },
    {
        .id = TCAM_PROPERTY_GPIN,
        .genicam_name = {"GPIn"},
    },
    {
        .id = TCAM_PROPERTY_GPOUT,
        .genicam_name = {"GPOut"},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_X,
        .genicam_name = {"OffsetX"},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_Y,
        .genicam_name = {"OffsetY"},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_AUTO,
        .genicam_name = {"OffsetAutoCenter"},
    },
    {
        .id = TCAM_PROPERTY_BRIGHTNESS,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_CONTRAST,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_SATURATION,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_HUE,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_GAMMA,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_IRCUT,
        .genicam_name = {"IRCutFilter"},
    },
    {
        .id = TCAM_PROPERTY_IRIS,
        .genicam_name = {"Iris"},
    },
    {
        .id = TCAM_PROPERTY_FOCUS,
        .genicam_name = {"Focus"},
    },
    {
        .id = TCAM_PROPERTY_ZOOM,
        .genicam_name = {"Zoom"},
    },
    {
        .id = TCAM_PROPERTY_FOCUS_AUTO,
        .genicam_name = {},
    },
    {
        .id = TCAM_PROPERTY_STROBE_ENABLE,
        .genicam_name = {"StrobeEnable"},
    },
    // {
    //     .name = "Strobe Polarity",
    //     .type_to_use = ,
    //     .genicam_name = {"StrobePolarity"},
    // },
    // {
    //     .name = "Strobe Operation",
    //     .type_to_use = ,
    //     .genicam_name = {"StrobeOperation"},
    // },
    // {
    //     .name = "",
    //     .type_to_use = ,
    //     .genicam_name = {},
    // },

};


static TCAM_PROPERTY_ID find_mapping (const std::string& genicam_id)
{
    auto f = [&genicam_id] (std::string p)
        {
            if (genicam_id == p)
                return true;
            return false;
        };

    for (const auto& m : aravis_mappings)
    {
        auto match = std::find_if(m.genicam_name.begin(), m.genicam_name.end(), f);

        if (match != m.genicam_name.end())
            return m.id;
    }
    return TCAM_PROPERTY_INVALID;
}


std::shared_ptr<Property> tcam::createProperty (ArvCamera* camera,
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
        tcam_log(TCAM_LOG_ERROR, "%s has unknown node type '%s'", feature, node_type);
        // TODO ERROR
    }

    auto prop_id = find_mapping(feature);

    auto ctrl_m = get_control_reference(prop_id);

    // type which the control shall use
    TCAM_PROPERTY_TYPE type_to_use;
    tcam_device_property prop = {};

    if (ctrl_m.id == TCAM_PROPERTY_INVALID)
    {
        tcam_log(TCAM_LOG_WARNING, "Unable to find std property. Passing raw property identifier through. %s", feature);
        // pass through and do not associate with anything existing
        // TODO LOG
        type_to_use = value_type_to_ctrl_type(type);
        memcpy(prop.name, feature, sizeof(prop.name));
        prop.type = value_type_to_ctrl_type(type);
    }
    else
    {
        type_to_use = ctrl_m.type_to_use;
        memcpy(prop.name, ctrl_m.name.c_str(), ctrl_m.name.size());
        prop.type = ctrl_m.type_to_use;
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
        if (type_to_use == TCAM_PROPERTY_TYPE_INTEGER)
        {
            //camera_property prop = {};
            prop.type = TCAM_PROPERTY_TYPE_INTEGER;
            prop.value.i.value = arv_device_get_integer_feature_value(arv_camera_get_device(camera), feature);
            prop.value.i.default_value = prop.value.i.value;

            prop.value.i.step = 1;

            arv_device_get_integer_feature_bounds(arv_camera_get_device(camera),
                                                  feature,
                                                  &prop.value.i.min,
                                                  &prop.value.i.max);

            return std::make_shared<PropertyInteger>(impl, prop, type);

        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_DOUBLE)
        {

            prop.type = TCAM_PROPERTY_TYPE_DOUBLE;
            prop.value.d.value = arv_device_get_integer_feature_value(arv_camera_get_device(camera), feature);
            prop.value.d.default_value = prop.value.i.value;


            prop.value.d.step = 1.0;

            int64_t min, max;
            arv_device_get_integer_feature_bounds(arv_camera_get_device(camera), feature, &min, &max);

            prop.value.d.min = min;
            prop.value.d.max = max;

            return std::make_shared<PropertyDouble>(impl, prop, type);

            // TODO: implement
            tcam_log(TCAM_LOG_ERROR, "Trying to map int to double. not implemented.");
            return nullptr;
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "\n\nBAD");
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
        if (type_to_use == TCAM_PROPERTY_TYPE_INTEGER)
        {
            prop.type = TCAM_PROPERTY_TYPE_INTEGER;
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
        prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;

        memcpy(prop.name, feature, sizeof(prop.name));
        prop.value.i.max = 1;
        prop.value.i.min = 0;
        prop.value.i.step = 1;
        // // TODO: default value

        return std::make_shared<PropertyBoolean>(impl, prop, type);

    }
    else if (strcmp(node_type, "Enumeration") == 0)
    {

        if (type_to_use == TCAM_PROPERTY_TYPE_BOOLEAN)
        {

            // c_map.control.value.i.value = 0;
            // c_map.control.value.i.min = 0;
            // c_map.control.value.i.max = c_map.values.size() - 1; // index begins with 0
            return nullptr;

        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_STRING_TABLE)
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
        if (type_to_use == TCAM_PROPERTY_TYPE_DOUBLE)
        {

            // c_map.control.value.d.value = arv_device_get_float_feature_value(arv_camera_get_device(camera), feature);
            // c_map.control.value.d.default_value = c_map.control.value.d.value;
            // c_map.control.value.d.step = 0;
            // arv_device_get_float_feature_bounds(arv_camera_get_device(camera),
            //                                     feature,
            //                                     &c_map.control.value.d.min,
            //                                     &c_map.control.value.d.max);

        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_INTEGER)
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


        if (type_to_use == TCAM_PROPERTY_TYPE_BUTTON)
        {
            prop.type = TCAM_PROPERTY_TYPE_BUTTON;

            return std::make_shared<PropertyButton>(impl, prop, type);
        }
        else
        {
            tcam_log(TCAM_LOG_DEBUG,"SHIT");
        }
    }
    else
    {
        tcam_log(TCAM_LOG_WARNING, "Unknown Control '%s'", feature);
    }

    return nullptr;

}


std::vector<DeviceInfo> tcam::get_aravis_device_list ()
{
    std::vector<DeviceInfo> device_list;

    arv_update_device_list ();

    unsigned int number_devices = arv_get_n_devices();

    if (number_devices == 0)
    {
        return device_list;
    }

    for (unsigned int i = 0; i < number_devices; ++i)
    {
        tcam_device_info info = {};
        std::string name = arv_get_device_id(i);
        memcpy(info.identifier, name.c_str(), name.size());

        ArvCamera* cam = arv_camera_new(name.c_str());

        info.type = TCAM_DEVICE_TYPE_ARAVIS;
        const char* n =  arv_camera_get_model_name(cam);

        if (n != NULL)
        {
            strncpy(info.name, n, sizeof(info.name));
        }
        else
        {
            tcam_log(TCAM_LOG_WARNING, "Unable to determine model name.");
        }
        size_t t = name.find("-");

        if (t != std::string::npos)
        {
            strcpy(info.serial_number, name.substr((t+1)).c_str());
        }

        device_list.push_back(DeviceInfo(info));

        g_object_unref(cam);
    }

    return device_list;
}
