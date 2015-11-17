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
        .id = TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        .genicam_name = {"ExposureAutoHighlighReduction"},
    },
    {
        .id = TCAM_PROPERTY_AUTO_REFERENCE,
        .genicam_name = {"ExposureAutoReference"},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        .genicam_name = {"ExposureAutoUpperLimitAuto"},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        .genicam_name = {"ExposureAutoUpperLimit"},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        .genicam_name = {"ExposureAutoLowerLimit"},
    },
    {
        .id = TCAM_PROPERTY_GAIN,
        .genicam_name = {"Gain"},
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO,
        .genicam_name = {"GainAuto"},
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
        .id = TCAM_PROPERTY_TRIGGER_DENOISE,
        .genicam_name = {"TriggerDenoise"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MASK,
        .genicam_name = {"TriggerMask"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DEBOUNCER,
        .genicam_name = {"TriggerDebouncer"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DELAY,
        .genicam_name = {"TriggerDelay"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_SELECTOR,
        .genicam_name = {"TriggerSelector"},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_OPERATION,
        .genicam_name = {"TriggerOperation"},
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
        .genicam_name = {"BalanceWhiteAuto"},
    },
    {
        .id = TCAM_PROPERTY_WB_MODE,
        .genicam_name = {"BalanceWhiteMode"},
    },
    {
        .id = TCAM_PROPERTY_WB_TEMPERATURE,
        .genicam_name = {"BalanceWhiteTemperature"},
    },
    {
        .id = TCAM_PROPERTY_BALANCERATIO_SELECTOR,
        .genicam_name = {"BalanceRatioSelector"}
    },
    {
        .id = TCAM_PROPERTY_BALANCERATIO,
        .genicam_name = {"BalanceRatio"}
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_AUTO_PRESET,
        .genicam_name = {"BalanceWhiteAutoPreset"}
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        .genicam_name = {"BalanceWhiteTemperaturePreset"}
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
    {
        .id = TCAM_PROPERTY_STROBE_OPERATION,
        .genicam_name = {"StrobeOperation"},
    },
    {
        .id = TCAM_PROPERTY_STROBE_POLARITY,
        .genicam_name = {"StrobePolarity"},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DELAY,
        .genicam_name = {"StrobeDelay"},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DURATION,
        .genicam_name = {"StrobeDuration"},
    },
    {
        .id = TCAM_PROPERTY_BINNING,
        .genicam_name = {"Binning"},
    },
    {
        .id = TCAM_PROPERTY_BINNING_VERTICAL,
        .genicam_name = {"BinningHorizontal"},
    },
    {
        .id = TCAM_PROPERTY_BINNING_HORIZONTAL,
        .genicam_name = {"BinningVertical"},
    },
    {
        .id = TCAM_PROPERTY_BLACKLEVEL,
        .genicam_name = {"BlackLevel"},
    },
    {
        .id = TCAM_PROPERTY_CHUNK_MODE_ACTIVE,
        .genicam_name = {"ChunkModeActive"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_COUNT,
        .genicam_name = {"DeviceStreamChannelCount"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_SELECTOR,
        .genicam_name = {"DeviceStreamChannelSelector"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_TYPE,
        .genicam_name = {"DeviceStreamChannelType"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_LINK,
        .genicam_name = {"DeviceStreamChannelLink"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_ENDIANNESS,
        .genicam_name = {"DeviceStreamChannelEndianness"},
    },
    {
        .id = TCAM_PROPERTY_STREAM_CHANNEL_PACKET_SIZE,
        .genicam_name = {"DeviceStreamChannelPacketSize"},
    },
    {
        .id = TCAM_PROPERTY_EVENT_CHANNEL_COUNT,
        .genicam_name = {"DeviceStreamChannelPacketSize"},
    },
    {
        .id = TCAM_PROPERTY_PAYLOAD_SIZE,
        .genicam_name = {"PayloadSize"},
    },
    {
        .id = TCAM_PROPERTY_PAYLOAD_PER_FRAME,
        .genicam_name = {"PayloadPerFrame"},
    },
    {
        .id = TCAM_PROPERTY_PAYLOAD_PER_PACKET,
        .genicam_name = {"PayloadPerPacket"},
    },
    {
        .id = TCAM_PROPERTY_TOTAL_PACKET_SIZE,
        .genicam_name = {"TotalPacketSize"},
    },
    {
        .id = TCAM_PROPERTY_PACKETS_PER_FRAME,
        .genicam_name = {"PacketsPerFrame"},
    },
    {
        .id = TCAM_PROPERTY_PACKET_TIME_US,
        .genicam_name = {"PacketTimeUS"},
    },
    {
        .id = TCAM_PROPERTY_REVERSE_X,
        .genicam_name = {"ReverseX"},
    },
    {
        .id = TCAM_PROPERTY_REVERSE_Y,
        .genicam_name = {"ReverseY"},
    },
    {
        .id = TCAM_PROPERTY_CHUNK_IMX174_FRAME_ID,
        .genicam_name = {"ChunkIMX174FrameId"},
    },
    {
        .id = TCAM_PROPERTY_CHUNK_IMX174_FRAME_SET,
        .genicam_name = {"ChunkIMX174FrameSet"},
    },
    {
        .id = TCAM_PROPERTY_IMX174_WDR_SHUTTER2,
        .genicam_name = {"IMX174WDRShutter2"},
    },
    {
        .id = TCAM_PROPERTY_IMX174_HARDWARE_WDR_ENABLE,
        .genicam_name = {"IMX174HardwareWDREnable"},
    },
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


std::shared_ptr<Property> tcam::create_property (ArvCamera* camera,
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
        // generate id so that identfication of passed through properties is guaranteed
        prop.id = generate_unique_property_id();
    }
    else
    {
        type_to_use = ctrl_m.type_to_use;

        prop = create_empty_property(ctrl_m.id);
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


                        // TODO should enums be stored in separate vector
                        // var.emplace(remove_prefix(arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), "_"), var.size());
                        var.emplace(arv_gc_feature_node_get_name((ArvGcFeatureNode*) iter->data), var.size());
                    }
                }
            }

            const char* current_value = arv_device_get_string_feature_value(arv_camera_get_device(camera), feature);

            prop.value.i.value = var.at(current_value);
            prop.value.i.default_value = prop.value.i.value;

            return std::make_shared<PropertyEnumeration>(impl, prop, var, type);
        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_INTEGER)
        {
            prop.type = TCAM_PROPERTY_TYPE_INTEGER;



            return  std::make_shared<PropertyInteger>(impl, prop, type);
        }
        else if (type_to_use == TCAM_PROPERTY_TYPE_BOOLEAN)
        {
            prop.type == TCAM_PROPERTY_TYPE_BOOLEAN;

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

        // // TODO: default value

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
