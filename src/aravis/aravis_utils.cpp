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

#include "aravis_utils.h"

#include "standard_properties.h"
#include "logging.h"

#include "internal.h"

#include <algorithm>
#include <vector>

// gige-daemon communication
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "gige-daemon.h"
#include "tcam-semaphores.h"

#include "genicam_property_mapping.h"

using namespace tcam;


std::shared_ptr<Property> tcam::create_property (ArvCamera* camera,
                                                ArvGcNode* node,
                                                std::shared_ptr<PropertyImpl> impl)
{
    // Map genicam feature to library feature
    // Check if an existing control should be used
    // if no control was found simply pass the genicam feature through

    const char* feature = arv_gc_feature_node_get_name ((ArvGcFeatureNode*) node);

    Property::VALUE_TYPE type = Property::INTEGER;

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
        tcam_error("%s has unknown node type '%s'", feature, node_type);
        return nullptr;
    }

    auto prop_id = find_mapping(feature);

    auto ctrl_m = get_control_reference(prop_id);

    // type which the control shall use
    TCAM_PROPERTY_TYPE type_to_use;
    tcam_device_property prop = {};

    if (ctrl_m.id == TCAM_PROPERTY_INVALID)
    {
        tcam_warning("Unable to find std property. Passing raw property identifier through. %s", feature);
        // pass through and do not associate with anything existing
        type_to_use = value_type_to_ctrl_type(type);
        memcpy(prop.name, feature, sizeof(prop.name));
        prop.type = value_type_to_ctrl_type(type);
        // generate id so that identfication of passed through properties is guaranteed
        prop.id = generate_unique_property_id();
    }
    else
    {
        // type_to_use = ctrl_m.type_to_use;
        type_to_use = value_type_to_ctrl_type(type);

        prop = create_empty_property(ctrl_m.id);

        prop.type = type_to_use;

        if (prop.id == TCAM_PROPERTY_EXPOSURE)
        {
            prop.type = TCAM_PROPERTY_TYPE_INTEGER;
            type_to_use = TCAM_PROPERTY_TYPE_INTEGER;

        }
    }

    if (ARV_IS_GC_ENUMERATION (node))
    {
        if (type_to_use == TCAM_PROPERTY_TYPE_ENUMERATION)
        {
            const GSList* children;
            const GSList* iter;

            children = arv_gc_enumeration_get_entries(ARV_GC_ENUMERATION (node));

            std::map<std::string, int> var;

            for (iter = children; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented((ArvGcFeatureNode*) iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name((ArvDomNode*) iter->data), "EnumEntry") == 0)
                    {
                        // tcam_log(TCAM_LOG_DEBUG, "    Adding enum entry: '%s' %s", arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data));

                        var.emplace(arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), var.size());
                    }
                }
            }

            const char* current_value = arv_device_get_string_feature_value(arv_camera_get_device(camera), feature);

            if (!current_value)
            {
                tcam_error("The current value of %s is NULL. Ignoring", prop.name);
                return nullptr;
            }

            prop.value.i.value = var.at(current_value);
            prop.value.i.default_value = prop.value.i.value;

            tcam_log(TCAM_LOG_DEBUG, "Returning EnumerationProperty for %s", prop.name);

            return std::make_shared<PropertyEnumeration>(impl, prop, var, type);
        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_INTEGER)
        {
            prop.type = TCAM_PROPERTY_TYPE_INTEGER;

            tcam_log(TCAM_LOG_DEBUG, "Returning IntegerProperty for %s", prop.name);

            return  std::make_shared<PropertyInteger>(impl, prop, type);
        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_BOOLEAN)
        {
            prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;

            const GSList* children;
            const GSList* iter;

            children = arv_gc_enumeration_get_entries(ARV_GC_ENUMERATION (node));

            std::map<std::string, int> var;

            for (iter = children; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented((ArvGcFeatureNode*) iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name((ArvDomNode*) iter->data), "EnumEntry") == 0)
                    {
                        var.emplace(arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), var.size());
                    }
                }
            }

            if (var.size() != 2)
            {
                tcam_log(TCAM_LOG_ERROR,
                         "'%s' has more values that expected and can not be properly mapped.",
                         prop.name);
                return std::make_shared<PropertyBoolean>(impl, prop, type);
            }

            const char* current_value = arv_device_get_string_feature_value(arv_camera_get_device(camera), feature);

            if (strcmp(current_value, "On") == 0)
            {
                prop.value.b.value = true;
            }
            else
            {
                prop.value.b.value = false;
            }

            // prop.value.b.value = var.at(current_value);
            prop.value.b.default_value = prop.value.b.value;

            tcam_log(TCAM_LOG_DEBUG, "Returning BooleanProperty for %s", prop.name);

            return std::make_shared<PropertyBoolean>(impl, prop, type);
        }

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
        else if (type_to_use == TCAM_PROPERTY_TYPE_DOUBLE)
        {

            double min, max;

            arv_device_get_float_feature_bounds(arv_camera_get_device(camera), feature, &min, &max);

            prop.value.d.value = arv_device_get_float_feature_value(arv_camera_get_device(camera), feature);
            prop.value.d.min = min;
            prop.value.d.max = max;
            prop.value.d.default_value = prop.value.d.value;
            prop.value.d.step = 0.001;

            return std::make_shared<PropertyDouble>(impl, prop, type);
        }
    }
    else if (strcmp(node_type, "Boolean") == 0)
    {
        // struct camera_property ctrl = {};
        prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;

        // memcpy(prop.name, feature, sizeof(prop.name));
        prop.value.i.max = 1;
        prop.value.i.min = 0;
        prop.value.i.step = 1;

        // TODO: default value

        return std::make_shared<PropertyBoolean>(impl, prop, type);

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
            tcam_log(TCAM_LOG_DEBUG,"Unknown property conversion required");
        }
    }
    else
    {
        tcam_log(TCAM_LOG_WARNING, "Unknown Control '%s'", feature);
    }

    return nullptr;

}


struct aravis_fourcc
{
    uint32_t fourcc;
    uint32_t aravis;
};


static const aravis_fourcc arv_fourcc_conversion_table[] =
{
    { FOURCC_Y800, ARV_PIXEL_FORMAT_MONO_8 },
    { 0, ARV_PIXEL_FORMAT_MONO_8_SIGNED },
    { 0, ARV_PIXEL_FORMAT_MONO_10 },
    { 0, ARV_PIXEL_FORMAT_MONO_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_12 },
    { 0, ARV_PIXEL_FORMAT_MONO_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_14 },
    { FOURCC_Y16,    ARV_PIXEL_FORMAT_MONO_16 },
    { FOURCC_GRBG8,  ARV_PIXEL_FORMAT_BAYER_GR_8 },
    { FOURCC_RGGB8,  ARV_PIXEL_FORMAT_BAYER_RG_8 },
    { FOURCC_GBRG8,  ARV_PIXEL_FORMAT_BAYER_GB_8 },
    { FOURCC_BGGR8,  ARV_PIXEL_FORMAT_BAYER_BG_8 },
    { FOURCC_GRBG10, ARV_PIXEL_FORMAT_BAYER_GR_10 },
    { FOURCC_RGGB10, ARV_PIXEL_FORMAT_BAYER_RG_10 },
    { FOURCC_GBRG10, ARV_PIXEL_FORMAT_BAYER_GB_10 },
    { FOURCC_BGGR10, ARV_PIXEL_FORMAT_BAYER_BG_10 },
    { FOURCC_GRBG12, ARV_PIXEL_FORMAT_BAYER_GR_12 },
    { FOURCC_RGGB12, ARV_PIXEL_FORMAT_BAYER_RG_12 },
    { FOURCC_GBRG12, ARV_PIXEL_FORMAT_BAYER_GB_12 },
    { FOURCC_BGGR12, ARV_PIXEL_FORMAT_BAYER_BG_12 },
    { FOURCC_GRBG12_PACKED, ARV_PIXEL_FORMAT_BAYER_GR_12_PACKED },
    { FOURCC_RGGB12_PACKED, ARV_PIXEL_FORMAT_BAYER_RG_12_PACKED },
    { FOURCC_GBRG12_PACKED, ARV_PIXEL_FORMAT_BAYER_GB_12_PACKED },
    { FOURCC_BGGR12_PACKED, ARV_PIXEL_FORMAT_BAYER_BG_12_PACKED },
    { FOURCC_GRBG16, ARV_PIXEL_FORMAT_BAYER_GR_16 },
    { FOURCC_RGGB16, ARV_PIXEL_FORMAT_BAYER_RG_16 },
    { FOURCC_GBRG16, ARV_PIXEL_FORMAT_BAYER_GB_16 },
    { FOURCC_BGGR16, ARV_PIXEL_FORMAT_BAYER_BG_16 },
    { FOURCC_BGR24, 0x02180015},
    { 0, ARV_PIXEL_FORMAT_RGB_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGBA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGRA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_411_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_422_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_444_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_8_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_16_PLANAR },
    { FOURCC_IYU1, ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED },
};


uint32_t tcam::aravis2fourcc (uint32_t aravis)
{
    for (auto&& e : arv_fourcc_conversion_table)
    {
        if (e.aravis == aravis)
        {
            return e.fourcc;
        }
    }

    return 0;
}


uint32_t tcam::fourcc2aravis (uint32_t fourcc)
{
    for (const auto& e : arv_fourcc_conversion_table)
    {
        if (e.fourcc == fourcc)
        {
            return e.aravis;
        }
    }

    return 0;
}

static const std::string gige_daemon_lock_file = "/var/lock/gige-daemon.lock";


std::vector<DeviceInfo> tcam::get_gige_device_list ()
{
    bool is_running = is_process_running( get_pid_from_lockfile( gige_daemon_lock_file ) );

    if (!is_running)
    {
        tcam_log( TCAM_LOG_ERROR, "Could not find gige-daemon. Using internal methods" );
        return get_aravis_device_list();
    }

    key_t shmkey = ftok( "/tmp/tcam-gige-camera-list", 'G' );
    key_t sem_key = ftok( "/tmp/tcam-gige-semaphore", 'S' );

    int shmid = shmget( shmkey, sizeof( struct tcam_gige_device_list ), 0644 );
    if (shmid < 0)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to connect to gige-daemon. Using internal methods");
        auto vec = get_aravis_device_list();
        tcam_log(TCAM_LOG_ERROR, "Aravis gave us %d", vec.size());
        return vec;
    }

    semaphore sem_id = semaphore::create( sem_key );
    std::lock_guard<semaphore> lck( sem_id );

    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, 0);

    if (d == nullptr)
    {
        shmdt(d);
        return std::vector<DeviceInfo>();
    }

    std::vector<DeviceInfo> ret;

    ret.reserve(d->device_count);

    for (unsigned int i = 0; i < d->device_count; ++i)
    {
        ret.push_back(DeviceInfo(d->devices[i]));
    }

    shmdt( d );

    return ret;
}


unsigned int tcam::get_gige_device_count ()
{
    return get_gige_device_list().size();
}

unsigned int tcam::get_aravis_device_count ()
{
    arv_update_device_list();

    return arv_get_n_devices();
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
        tcam_device_info info = { TCAM_DEVICE_TYPE_ARAVIS,
                                  "", "", "", ""};
        std::string name = arv_get_device_id(i);
        memcpy(info.identifier, name.c_str(), name.size());

        ArvCamera* cam = arv_camera_new(name.c_str());

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
