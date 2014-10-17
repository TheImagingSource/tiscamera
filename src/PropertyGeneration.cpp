
#include "PropertyGeneration.h"

#include "base_types.h"
#include "utils.h"
#include "logging.h"

#include "standard_properties.h"

#include <linux/videodev2.h>
#include <cstring>
#include <algorithm>

using namespace tis_imaging;

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

