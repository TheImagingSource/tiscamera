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

#include "../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_info_list.h"
#include "../error.h"
#include "../logging.h"
#include "AravisDevice.h"
#include "AravisPropertyBackend.h"
#include "aravis_property_impl.h"
#include "aravis_utils.h"


using namespace tcam::property;


namespace
{
enum class map_type
{
    pub, // public       added to AravisDevice::properties_
    priv, // private      added to AravisDevice::internal_properties_
    blacklist, // blocklisted  not-added
};

struct aravis_property_name_map
{
    constexpr aravis_property_name_map() = default;
    constexpr aravis_property_name_map(std::string_view n, map_type m) noexcept : name(n), type(m)
    {
    }
    constexpr aravis_property_name_map(std::string_view n,
                                       std::string_view new_n,
                                       map_type m = map_type::pub) noexcept
        : name(n), type(m), new_name(new_n)
    {
    }

    std::string_view name;

    map_type type = map_type::pub;

    std::string_view new_name;
};

// clang-format off
static const aravis_property_name_map aravis_property_name_mapping_list[] =
{
    { "TLParamsLocked", map_type::priv },
    // should not be accessible because they are used by aravis
    { "GevSCPSDoNotFragment", map_type::blacklist },
    { "GevSCPSPacketSize", map_type::blacklist },
    { "GevSCPSFireTestPacket", map_type::blacklist },
    { "GevTimestampTickFrequency", map_type::priv }, // public?
    { "GevSCPD", map_type::priv }, // public? Sets the inter-packet delay (in ticks) for the selected stream channel
    
    { "PayloadSize", map_type::blacklist }, // blacklist because of warning in aravis
    // 23G camera properties (for whatever reason)
    { "PayloadPerPacket", map_type::blacklist },
    { "PacketsPerFrame", map_type::blacklist },
    { "TotalPacketSize", map_type::blacklist },
    { "PacketTimeUS", map_type::blacklist },

    { "GevGVSPExtendedIDMode", map_type::priv },    // This should be controlled by aravis

    // ChunkMode stuff, currently just blacklist
    { "ChunkModeActive", map_type::priv },
    { "ChunkImage", map_type::blacklist },  // is Register
    { "ChunkBlockId", map_type::blacklist },
    { "ChunkIMX174FrameSet", map_type::blacklist },
    { "ChunkIMX174FrameId", map_type::blacklist },

    // GigEVision stuff?
    { "DeviceType", map_type::blacklist },
    { "DeviceSFNCVersionMajor", map_type::blacklist },
    { "DeviceSFNCVersionMinor", map_type::blacklist },
    { "DeviceSFNCVersionSubMinor", map_type::blacklist },
    { "DeviceTLType", map_type::blacklist },
    { "DeviceTLTypeMajor", map_type::blacklist },
    { "DeviceTLTypeMinor", map_type::blacklist },
    { "DeviceTLTypeSubMinor", map_type::blacklist },
    { "DeviceTLVersionMajor", map_type::blacklist },
    { "DeviceTLVersionMinor", map_type::blacklist },
    { "DeviceTLVersionSubMinor", map_type::blacklist },
    { "DeviceLinkSelector", map_type::blacklist },
    { "DeviceStreamChannelCount", map_type::blacklist },
    { "DeviceStreamChannelSelector", map_type::blacklist },
    { "DeviceStreamChannelType", map_type::blacklist },
    { "DeviceStreamChannelLink", map_type::blacklist },
    { "DeviceStreamChannelEndianness", map_type::blacklist },
    { "DeviceStreamChannelPacketSize", map_type::blacklist },
    { "DeviceEventChannelCount", map_type::blacklist },

    // These should be private
    { "DeviceReset", map_type::priv },
    { "DeviceFactoryReset", map_type::priv },

    // aravis stuff, should not be accessible
    { "AcquisitionStart", map_type::priv },
    { "AcquisitionStop", map_type::priv },
    { "AcquisitionMode", map_type::priv },
    { "Width", map_type::priv },
    { "Height", map_type::priv },
    { "FPS", map_type::priv },
    { "AcquisitionFrameRate", map_type::priv },
    { "PixelFormat", map_type::priv },

    // Binning etc.
    { "Binning", map_type::priv },
    { "BinningHorizontal", map_type::priv },
    { "BinningVertical", map_type::priv },
    { "SkippingHorizontal", map_type::priv },
    { "SkippingVertical", map_type::priv },
    { "DecimationHorizontal", map_type::priv },
    { "DecimationVertical", map_type::priv },

    { "LUTValueAll", map_type::blacklist }, // blacklisted because this is a unsupported 'Register' GenICam node

    // These generate aravis error messages, so just blacklist
    { "WidthMax", map_type::blacklist },
    { "HeightMax", map_type::blacklist },

    // rename stuff
    { "OffsetAuto", "OffsetAutoCenter", map_type::pub },
    { "IRCutFilterEnableElement", "IRCutFilterEnable", map_type::pub },
    { "ExposureAutoHighlighReduction", "ExposureAutoHighlightReduction", map_type::pub }, // This should already be there??

    // These controls get overridden by hand build implementations to either fix type issues or to flatten properties
    { "BalanceRatioSelector", map_type::priv },
    { "BalanceRatio", map_type::priv },
    { "BalanceRatioRaw", map_type::priv },
    { "FocusAuto", map_type::priv },
    { "IrisAuto", map_type::priv },

    // private/blacklisted because of potential problems with e.g. Width
    { "UserSetSelector", map_type::blacklist },
    { "UserSetLoad", map_type::blacklist },
    { "UserSetSave", map_type::blacklist },
    { "UserSetDefault", map_type::blacklist },

    // blacklisted due to problems with serialization
    { "DeviceUserID", map_type::blacklist },
    { "LUTEnable", map_type::blacklist },
    { "LUTIndex", map_type::blacklist },
    { "LUTSelector", map_type::blacklist },
    { "LUTValue", map_type::blacklist },
    // LUTValueAll is already blacklisted
};

static const aravis_property_name_map unlinked_property_map[] =
{
    { "TestBinningHorizontal", map_type::priv },
    { "TestBinningVertical", map_type::priv },
    { "TestDecimationHorizontal", map_type::priv },
    { "TestDecimationVertical", map_type::priv },
};

// clang-format on


auto find_map_info(std::string_view name) noexcept -> aravis_property_name_map
{
    auto f = std::find_if(std::begin(aravis_property_name_mapping_list),
                          std::end(aravis_property_name_mapping_list),
                          [name](const auto& v) { return v.name == name; });
    if (f != std::end(aravis_property_name_mapping_list))
    {
        return *f;
    }
    f = std::find_if(std::begin(unlinked_property_map),
                     std::end(unlinked_property_map),
                     [name](const auto& v) { return v.name == name; });
    if (f != std::end(unlinked_property_map))
    {
        return *f;
    }
    return {};
}

auto build_property_from_node(std::string_view name,
                              std::string_view category,
                              ArvGcNode* node,
                              const std::shared_ptr<tcam::aravis::AravisPropertyBackend>& backend,
                              const aravis_property_name_map& map_entry)
    -> std::shared_ptr<tcam::property::IPropertyBase>
{
    using namespace tcam::aravis;

    std::string_view prop_name = name;
    if (!map_entry.new_name.empty())
    {
        prop_name = map_entry.new_name;
    }

    std::shared_ptr<tcam::property::IPropertyBase> prop;
    if (ARV_IS_GC_ENUMERATION(node))
    {
        prop = std::make_shared<AravisPropertyEnumImpl>(prop_name, category, node, backend);
    }
    else if (ARV_IS_GC_FLOAT(node))
    {
        prop = std::make_shared<AravisPropertyDoubleImpl>(prop_name, category, node, backend);
    }
    else if (ARV_IS_GC_INTEGER(node))
    {
        prop = std::make_shared<AravisPropertyIntegerImpl>(prop_name, category, node, backend);
    }
    else if (ARV_IS_GC_BOOLEAN(node))
    {
        prop = std::make_shared<AravisPropertyBoolImpl>(prop_name, category, node, backend);
    }
    else if (ARV_IS_GC_COMMAND(node))
    {
        prop = std::make_shared<AravisPropertyCommandImpl>(prop_name, category, node, backend);
    }
    else if (ARV_IS_GC_STRING(node))
    {
        prop = std::make_shared<AravisPropertyStringImpl>(prop_name, category, node, backend);
    }
    else
    {
        SPDLOG_INFO("Property '{}' node-name '{}' not implemented.",
                    prop_name,
                    arv_dom_node_get_node_name(ARV_DOM_NODE(node)));
    }
    return prop;
}


struct node_cat_entry
{
    std::string category;
    std::string name;
    ArvGcFeatureNode* node;
};

void create_ordered_property_list(std::vector<node_cat_entry>& out_lst,
                                  ArvGc* document,
                                  const char* category,
                                  const char* name)
{
    ArvGcNode* node = arv_gc_get_node(document, name);
    if (!ARV_IS_GC_FEATURE_NODE(node))
        return;

    ArvGcFeatureNode* feature_node = ARV_GC_FEATURE_NODE(node);
    if (!arv_gc_feature_node_is_implemented(feature_node, NULL))
    {
        return;
    }

    if (ARV_IS_GC_CATEGORY(node))
    {
        auto features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));

        auto category_display_name = arv_gc_feature_node_get_display_name(feature_node);
        if (!category_display_name)
        {
            category_display_name = name;
        }

        for (auto iter = features; iter != NULL; iter = iter->next)
        {
            create_ordered_property_list(
                out_lst, document, category_display_name, (char*)iter->data);
        }
        return;
    }

    out_lst.push_back(node_cat_entry { category, name, feature_node });
}

} // namespace

bool tcam::aravis::is_private_setting(std::string_view name)
{
    auto info = find_map_info(name);
    return info.type != map_type::pub;
}

void tcam::AravisDevice::create_property_list_from_genicam_categories()
{
    std::vector<node_cat_entry> lst;
    create_ordered_property_list(lst, genicam_, nullptr, "Root");

    auto find_node = [&](std::string_view name) -> bool
    {
        return std::any_of(
            lst.begin(), lst.end(), [name](const node_cat_entry& e) { return e.name == name; });
    };

    for (auto&& entry : lst)
    {
        std::string_view prop_name = arv_gc_feature_node_get_name(entry.node);
        auto map_info = find_map_info(prop_name);
        if (map_info.type == map_type::blacklist)
        {
            continue;
        }
        if (!map_info.new_name.empty())
        {
            if (find_node(
                    map_info.new_name)) // if node with new name is already present skip this node
            {
                continue;
            }
        }

        auto prop = build_property_from_node(
            prop_name, entry.category, ARV_GC_NODE(entry.node), backend_, map_info);
        if (prop != nullptr)
        {
            if (map_info.type == map_type::priv)
            {
                internal_properties_.push_back(prop);
            }
            else
            {
                properties_.push_back(prop);
            }
        }
    }
}

static void add_property_after(
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& prop_list,
    std::string_view name,
    std::shared_ptr<tcam::property::IPropertyBase> prop)
{
    auto f = std::find_if(prop_list.begin(),
                          prop_list.end(),
                          [name](const auto& p) { return p->get_name() == name; });
    if (f != prop_list.end())
    {
        prop_list.insert(f + 1, prop);
    }
    else
    {
        prop_list.push_back(prop);
    }
}

void tcam::AravisDevice::generate_properties_from_genicam()
{
    create_property_list_from_genicam_categories();

    // this adds things like TestBinningVertical to the private properties
    for (auto&& e : unlinked_property_map)
    {
        ArvGcNode* node = arv_gc_get_node(genicam_, std::string(e.name).c_str());
        if (node == nullptr)
        {
            continue;
        }

        auto prop = build_property_from_node(e.name, "internal", node, backend_, e);
        if (prop != nullptr)
        {
            if (e.type == map_type::priv)
            {
                internal_properties_.push_back(prop);
            }
            else
            {
                properties_.push_back(prop);
            }
        }
    }

    auto balance_ratio_selector =
        find_cam_property<tcam::property::IPropertyEnum>("BalanceRatioSelector");
    if (balance_ratio_selector)
    {
        auto balance_ratio_float =
            find_cam_property<tcam::property::IPropertyFloat>("BalanceRatio");
        if (balance_ratio_float)
        {
            properties_.push_back(std::make_shared<tcam::aravis::balance_ratio_to_wb_channel>(
                balance_ratio_selector,
                balance_ratio_float,
                "Red",
                &tcamprop1::prop_list::BalanceWhiteRed,
                backend_));
            properties_.push_back(std::make_shared<tcam::aravis::balance_ratio_to_wb_channel>(
                balance_ratio_selector,
                balance_ratio_float,
                "Green",
                &tcamprop1::prop_list::BalanceWhiteGreen,
                backend_));
            properties_.push_back(std::make_shared<tcam::aravis::balance_ratio_to_wb_channel>(
                balance_ratio_selector,
                balance_ratio_float,
                "Blue",
                &tcamprop1::prop_list::BalanceWhiteBlue,
                backend_));
        }
        else
        {
            auto balance_ratio_raw =
                find_cam_property<tcam::property::IPropertyInteger>("BalanceRatioRaw");

            if (balance_ratio_raw)
            {
                properties_.push_back(
                    std::make_shared<tcam::aravis::balance_ratio_raw_to_wb_channel>(
                        balance_ratio_selector,
                        balance_ratio_raw,
                        "Red",
                        &tcamprop1::prop_list::BalanceWhiteRed,
                        backend_));
                properties_.push_back(
                    std::make_shared<tcam::aravis::balance_ratio_raw_to_wb_channel>(
                        balance_ratio_selector,
                        balance_ratio_raw,
                        "Green",
                        &tcamprop1::prop_list::BalanceWhiteGreen,
                        backend_));
                properties_.push_back(
                    std::make_shared<tcam::aravis::balance_ratio_raw_to_wb_channel>(
                        balance_ratio_selector,
                        balance_ratio_raw,
                        "Blue",
                        &tcamprop1::prop_list::BalanceWhiteBlue,
                        backend_));
            }
        }
    }

    auto focus_auto = find_cam_property<tcam::property::IPropertyCommand>("FocusAuto");
    if (focus_auto)
    {
        add_property_after(
            properties_,
            "Focus",
            std::make_shared<tcam::aravis::focus_auto_enum_override>(focus_auto, backend_));
    }
    auto iris_auto = find_cam_property<tcam::property::IPropertyBool>("IrisAuto");
    if (iris_auto)
    {
        add_property_after(
            properties_,
            "Iris",
            std::make_shared<tcam::aravis::iris_auto_enum_override>(iris_auto, backend_));
    }
}
