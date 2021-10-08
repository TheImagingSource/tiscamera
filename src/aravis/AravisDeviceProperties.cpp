/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "../error.h"
#include "../logging.h"
#include "AravisDevice.h"
#include "aravis_property_impl.h"

namespace
{


static const std::pair<std::string_view, std::string_view> name_table[] = {
    { "OffsetAutoCenter", "OffsetAuto" },
    { "IRCutFilterEnableElement", "IRCutFilterEnable" },
};

outcome::result<std::string> find_conversion_name(const std::string& name)
{
    for (const auto& entry : name_table)
    {
        if (entry.first == name)
        {
            return std::string(entry.second);
        }
    }
    return tcam::status::PropertyDoesNotExist;
}

} // namespace


namespace tcam
{

void AravisDevice::index_properties(const char* name)
{
    ArvGcNode* node = arv_gc_get_node(genicam, name);

    if (ARV_IS_GC_FEATURE_NODE(node)
        && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), NULL)
        && arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL))
    {

        if (ARV_IS_GC_CATEGORY(node))
        {
            const GSList* features;
            const GSList* iter;

            std::string node_name = arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(node));

            if (node_name == "DeviceControl" || node_name == "TransportLayerControl"
                || node_name == "ImageFormatControl")
            {
                return;
            }

            features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));

            for (iter = features; iter != NULL; iter = iter->next)
            {
                index_properties((char*)iter->data);
            }
            return;
        }
    }

    static std::vector<std::string> private_settings = { "TLParamsLocked",
                                                         "GevSCPSDoNotFragment",
                                                         "GevTimestampTickFrequency",
                                                         "GevTimeSCPD",
                                                         "GevSCPD",
                                                         "PayloadSize",
                                                         "PayloadPerFrame",
                                                         "PayloadPerPacket",
                                                         "TotalPacketSize",
                                                         "PacketsPerFrame",
                                                         "PacketTimeUS",
                                                         "GevSCPSPacketSize",
                                                         "GevSCPSFireTestPacket",
                                                         "DeviceVendorName",
                                                         "DeviceType",
                                                         "DeviceModelType",
                                                         "DeviceVersion",
                                                         "DeviceSerialNumber",
                                                         "DeviceUserID",
                                                         "DeviceSFNCVersionMajor",
                                                         "DeviceSFNCVersionMinor",
                                                         "DeviceTLType",
                                                         "DeviceTLTypeMajor",
                                                         "DeviceTLTypeMinor",
                                                         "DeviceTLTypeSubMinor",
                                                         "DeviceLinkSelector",
                                                         "WidthMax",
                                                         "HeightMax",
                                                         "ChunkModeActive",
                                                         "ChunkImage",
                                                         "ChunkBlockId",
                                                         "ActionDeviceKey",
                                                         "ActionSelector",
                                                         "ActionGroupMask",
                                                         "ActionGroupKey",
                                                         "UserSetSelector",
                                                         "UserSetLoad",
                                                         "UserSetSave",
                                                         "UserSetDefault",
                                                         "DeviceScanType",
                                                         "StringReg",
                                                         "DeviceModelName",
                                                         "DeviceSFNCVersionSubMinor",
                                                         "MaskedIntReg",
                                                         "DeviceTLVersionMajor",
                                                         "MaskedIntReg",
                                                         "DeviceTLVersionMinor",
                                                         "DeviceTLVersionSubMinor",
                                                         "DeviceLinkHeartbeatTimeout",
                                                         "DeviceStreamChannelCount",
                                                         "DeviceStreamChannelSelector",
                                                         "DeviceStreamChannelType",
                                                         "DeviceStreamChannelLink",
                                                         "DeviceStreamChannelEndianness",
                                                         "DeviceStreamChannelPacketSize",
                                                         "DeviceEventChannelCount",
                                                         "DeviceTemperatureConverter",
                                                         "IMX174HardwareWDRShutterMode",
                                                         "IMX174HardwareWDREnable",
                                                         "IMX174WDRShutter2",
                                                         "ChunkIMX174FrameSet",
                                                         "ChunkIMX174FrameId",
                                                         "SensorPixelHeight",
                                                         "SensorPixelWidth",
                                                         "AcquisitionStart",
                                                         "AcquisitionStop",
                                                         "AcquisitionMode",
                                                         // "Binning",
                                                         "SensorWidth",
                                                         "SensorHeight",
                                                         "Width",
                                                         "Height",
                                                         "FPS",
                                                         "AcquisitionFrameRate",
                                                         "PixelFormat",
                                                         "Binning",
                                                         "BinningHorizontal",
                                                         "BinningVertical",
                                                         "SkippingHorizontal",
                                                         "SkippingVertical",
                                                         "DecimationHorizontal",
                                                         "DecimationVertical",
    };

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>>* container = &m_properties;

    if (std::find(private_settings.begin(),
                  private_settings.end(),
                  arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(node)))
        != private_settings.end())
    {
        //SPDLOG_ERROR("Private setting {}", arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(node)));
        //return;
        container = &m_internal_properties;
    }

    std::string prop_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);
    auto conv_name = find_conversion_name(prop_name);

    if (conv_name)
    {
        prop_name = conv_name.value();
    }

    if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Float") == 0)
    {
        container->push_back(std::make_shared<tcam::property::AravisPropertyDoubleImpl>(
            prop_name, arv_camera, node, m_backend));
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Integer") == 0)
    {
        container->push_back(std::make_shared<tcam::property::AravisPropertyIntegerImpl>(
            prop_name, arv_camera, node, m_backend));
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Boolean") == 0)
    {
        container->push_back(std::make_shared<tcam::property::AravisPropertyBoolImpl>(
            prop_name, arv_camera, node, m_backend));
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Command") == 0)
    {
        container->push_back(std::make_shared<tcam::property::AravisPropertyCommandImpl>(
            prop_name, node, m_backend));
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Enumeration") == 0)
    {
        container->push_back(std::make_shared<tcam::property::AravisPropertyEnumImpl>(
            prop_name, arv_camera, node, m_backend));
    }
    else
    {
        SPDLOG_ERROR("Not implemented - {} - {}!!!!", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), prop_name);
    }
    //m_properties.push_back();
}

} // namespace tcam
