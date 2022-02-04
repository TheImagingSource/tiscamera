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
#include "aravis_utils.h"

namespace
{


static const std::pair<std::string_view, std::string_view> name_table[] = {
    { "OffsetAuto", "OffsetAutoCenter" },
    { "IRCutFilterEnableElement", "IRCutFilterEnable" },
    { "ExposureAutoHighlighReduction", "ExposureAutoHighlightReduction" },
};

std::string find_conversion_name(const std::string& name)
{
    for (const auto& entry : name_table)
    {
        if (entry.first == name)
        {
            return std::string(entry.second);
        }
    }
    return {};
}

} // namespace


namespace tcam
{

void AravisDevice::index_properties(const char* name)
{
    ArvGcNode* node = arv_gc_get_node(genicam_, name);

    if (ARV_IS_GC_FEATURE_NODE(node)
        && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), NULL)
        && arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL))
    {
        if (ARV_IS_GC_CATEGORY(node))
        {
            std::string node_name = arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(node));

            if (node_name == "DeviceControl" || node_name == "TransportLayerControl")
            {
                return;
            }

            auto features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));

            for (auto iter = features; iter != NULL; iter = iter->next)
            {
                index_properties((char*)iter->data);
            }
            return;
        }
    }

    std::string actual_name = arv_gc_feature_node_get_name((ArvGcFeatureNode*)node);

    std::string prop_name = actual_name;
    if (auto conv_name = find_conversion_name(prop_name); !conv_name.empty())
    {
        prop_name = conv_name;
    }

    std::shared_ptr<tcam::property::IPropertyBase> prop;
    if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Float") == 0)
    {
        prop = std::make_shared<tcam::property::AravisPropertyDoubleImpl>(
            prop_name, arv_camera_, node, backend_);
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Integer") == 0)
    {
        prop = std::make_shared<tcam::property::AravisPropertyIntegerImpl>(
            prop_name, arv_camera_, node, backend_);
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Boolean") == 0)
    {
        prop = std::make_shared<tcam::property::AravisPropertyBoolImpl>(
            prop_name, arv_camera_, node, backend_);
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Command") == 0)
    {
        prop =
            std::make_shared<tcam::property::AravisPropertyCommandImpl>(prop_name, node, backend_);
    }
    else if (strcmp(arv_dom_node_get_node_name(ARV_DOM_NODE(node)), "Enumeration") == 0)
    {
        prop = std::make_shared<tcam::property::AravisPropertyEnumImpl>(
            prop_name, arv_camera_, node, backend_);
    }
    else
    {
        SPDLOG_ERROR("Not implemented - {} - {}!!!!",
                     arv_dom_node_get_node_name(ARV_DOM_NODE(node)),
                     prop_name);
    }

    if (prop != nullptr)
    {
        if (is_private_setting(actual_name))
        {
            //SPDLOG_ERROR("Private setting {}", arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(node)));
            internal_properties_.push_back(prop);
        }
        else
        {
            properties_.push_back(prop);
        }
    }
}

} // namespace tcam
