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

#include "../logging.h"
#include "AravisDevice.h"

using namespace tcam;

void AravisDevice::determine_scaling_type()
{
    scale_.scale_type = ImageScalingType::Unknown;

    auto add_scaling_prop_if_av = [this](const std::string& name, ImageScalingType flag)
    {
        auto prop = tcam::property::find_property(internal_properties_, name);
        if (prop)
        {
            scale_.scale_type = flag;
            scale_.properties.push_back(prop);
            return true;
        }
        return false;
    };

    add_scaling_prop_if_av("Binning", ImageScalingType::Binning);
    add_scaling_prop_if_av("BinningHorizontal", ImageScalingType::Binning);
    add_scaling_prop_if_av("BinningVertical", ImageScalingType::Binning);

    ImageScalingType to_set = ImageScalingType::Skipping;

    // we already have binning
    // if something skipping related is found we have both
    if (scale_.scale_type == ImageScalingType::Binning)
    {
        to_set = ImageScalingType::BinningSkipping;
    }

    add_scaling_prop_if_av("DecimationHorizontal", to_set);
    add_scaling_prop_if_av("DecimationVertical", to_set);

    if (scale_.scale_type == ImageScalingType::Unknown)
    {
        scale_.scale_type = ImageScalingType::None;
    }
}


static bool image_scaling_comparator(const image_scaling& lhs,const image_scaling& other)
{
    auto us = lhs.binning_v + lhs.binning_h + lhs.skipping_v + lhs.skipping_h;
    auto them = other.binning_v + other.binning_h + other.skipping_v + other.skipping_h;
    if (us < them)
    {
        return true;
    }
    return false;
}

void AravisDevice::generate_scaling_information()
{
    determine_scaling_type();

    auto legal_value = [](const int i)
    {
        // only accept valid values
        if (i == 1 || i == 2 || i == 4 || i == 8)
        {
            return true;
        }
        return false;
    };

    if (scale_.scale_type == ImageScalingType::Binning)
    {
        auto binning_h = tcam::property::find_property(scale_.properties, "BinningHorizontal");
        auto binning_v = tcam::property::find_property(scale_.properties, "BinningVertical");

        if (binning_h && binning_v)
        {
            auto bh = dynamic_cast<tcam::property::IPropertyInteger*>(binning_h.get());
            auto bv = dynamic_cast<tcam::property::IPropertyInteger*>(binning_v.get());

            for (int i = bh->get_range().min; i <= bh->get_range().max; i++)
            {
                // only accept valid values
                if (!legal_value(i))
                {
                    continue;
                }

                for (int v = bv->get_range().min; v <= bv->get_range().max; v++)
                {
                    // only accept valid values
                    if (!legal_value(v))
                    {
                        continue;
                    }

                    image_scaling new_scale = {};

                    new_scale.binning_h = i;
                    new_scale.binning_v = v;

                    // SPDLOG_INFO("New binning: {}x{}", i, i);
                    //SPDLOG_ERROR("new", bh->);

                    scale_.scaling_info_list.push_back(new_scale);
                }
            }
        }
        else
        {
            auto binning_base = tcam::property::find_property(scale_.properties, "Binning");

            if (binning_base)
            {
                auto binning = dynamic_cast<tcam::property::IPropertyEnum*>(binning_base.get());

                auto entries = binning->get_entries();

                for (const auto& entry : entries)
                {
                    if (entry == "Off")
                    {
                        scale_.scaling_info_list.push_back({ 1, 1 });
                    }
                    else if (entry == "2x")
                    {
                        scale_.scaling_info_list.push_back({ 2, 2 });
                    }
                    else if (entry == "4x")
                    {
                        scale_.scaling_info_list.push_back({ 4, 4 });
                    }
                    else
                    {
                        SPDLOG_WARN("Binning entry '{}' not implemented.", entry);
                    }
                }

                // ensure scaling info are always in the order 1x1, 2x2, 4x4
                std::sort(scale_.scaling_info_list.begin(), scale_.scaling_info_list.end(),&image_scaling_comparator);
            }
        }
    }

    if (scale_.scale_type == ImageScalingType::Skipping)
    {
        auto skipping_h = tcam::property::find_property(scale_.properties, "DecimationHorizontal");
        auto skipping_v = tcam::property::find_property(scale_.properties, "DecimationVertical");

        auto bh = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_h.get());
        auto bv = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_v.get());

        for (int i = bh->get_range().min; i <= bh->get_range().max; i++)
        {
            // only accept valid values
            if (!legal_value(i))
            {
                continue;
            }

            for (int v = bv->get_range().min; v <= bv->get_range().max; v++)
            {
                // only accept valid values
                if (!legal_value(v))
                {
                    continue;
                }

                image_scaling new_scale = {};

                new_scale.skipping_h = i;
                new_scale.skipping_v = v;

                scale_.scaling_info_list.push_back(new_scale);
            }
        }
    }

    if (scale_.scale_type == ImageScalingType::BinningSkipping)
    {
        auto get_value = [this](const std::string& name)
        {
            GError* err = nullptr;
            auto ret = arv_device_get_integer_feature_value(
                arv_camera_get_device(this->arv_camera_), name.c_str(), &err);

            if (err)
            {
                SPDLOG_ERROR("Received error: {}", err->message);
                g_clear_error(&err);
            }

            return ret;
        };

        auto set_value = [this](const std::string& name, int value)
        {
            GError* err = nullptr;
            arv_device_set_integer_feature_value(
                arv_camera_get_device(this->arv_camera_), name.c_str(), value, &err);

            if (err)
            {
                SPDLOG_ERROR("Received error: {}", err->message);
                g_clear_error(&err);
            }
        };

        auto binning_h = tcam::property::find_property(scale_.properties, "BinningHorizontal");
        auto binning_v = tcam::property::find_property(scale_.properties, "BinningVertical");
        // auto skipping_h = tcam::property::find_property(m_scale.m_properties, "SkippingHorizontal");
        // auto skipping_v = tcam::property::find_property(m_scale.m_properties, "SkippingVertical");
        auto skipping_h = tcam::property::find_property(scale_.properties, "DecimationHorizontal");
        auto skipping_v = tcam::property::find_property(scale_.properties, "DecimationVertical");

        auto sh = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_h.get());
        auto sv = dynamic_cast<tcam::property::IPropertyInteger*>(skipping_v.get());

        auto bh = dynamic_cast<tcam::property::IPropertyInteger*>(binning_h.get());
        auto bv = dynamic_cast<tcam::property::IPropertyInteger*>(binning_v.get());

        auto scale_is_known = [=](const tcam::image_scaling& s)
        {
            auto find = std::find_if(scale_.scaling_info_list.begin(),
                                     scale_.scaling_info_list.end(),
                                     [s](const tcam::image_scaling& vs) { return (s == vs); });

            if (find == scale_.scaling_info_list.end())
            {
                return false;
            }
            return true;
        };

        auto is_valid = [] (int value) -> bool
        {
            if (value == 1
            || value == 2
            || value == 4
            || value == 8)
            {
                return true;
            }
            return false;
        };

        // try all possible combinations
        for (int bin_h = bh->get_range().min; bin_h <= bh->get_range().max; bin_h++)
        {
            if (!is_valid(bin_h)) continue;
            for (int bin_v = bv->get_range().min; bin_v <= bv->get_range().max; bin_v++)
            {
                if (!is_valid(bin_v)) continue;
                for (int sk_h = sh->get_range().min; sk_h <= sh->get_range().max; sk_h++)
                {
                    if (!is_valid(sk_h)) continue;
                    for (int sk_v = sv->get_range().min; sk_v <= sv->get_range().max; sk_v++)
                    {
                        if (!is_valid(sk_v)) continue;
                        tcam::image_scaling is;

                        set_value("TestBinningHorizontal", bin_h);
                        set_value("TestBinningVertical", bin_v);

                        set_value("TestDecimationHorizontal", sk_h);
                        set_value("TestDecimationVertical", sk_v);

                        is.binning_h = get_value("TestBinningHorizontal");
                        is.binning_v = get_value("TestBinningVertical");

                        is.skipping_h = get_value("TestDecimationHorizontal");
                        is.skipping_v = get_value("TestDecimationVertical");

                        // SPDLOG_ERROR("testing: {}x{} {}x{}", bin_h, bin_v, sk_h, sk_v);
                        // SPDLOG_ERROR("scale: {}x{} {}x{}", is.binning_h, is.binning_v, is.skipping_h, is.skipping_v);

                        // no need for duplicates
                        // setting skipping 1x2 or 2x1 might both result in 2x2
                        if (scale_is_known(is))
                        {
                            continue;
                        }

                        scale_.scaling_info_list.push_back(is);
                    }
                }
            }
        }
    }
}


bool AravisDevice::set_scaling(const image_scaling& scale)
{
    if (scale_.scale_type == ImageScalingType::Binning
        || scale_.scale_type == ImageScalingType::BinningSkipping)
    {
        auto binning_base = tcam::property::find_property(scale_.properties, "Binning");
        if (binning_base)
        {
            SPDLOG_TRACE("Setting binning to: {}x{}", scale.binning_h, scale.binning_v);

            auto binning = dynamic_cast<tcam::property::IPropertyEnum*>(binning_base.get());

            std::string value;

            // this type of binning should only appear
            // in legacy cameras that only offer Binning
            if (scale.binning_h == 1)
            {
                value = "Off";
            }
            else if (scale.binning_h == 2)
            {
                value = "2x";
            }
            else if (scale.binning_h == 4)
            {
                value = "4x";
            }
            else
            {
                SPDLOG_ERROR("Binning type not implemented");
            }

            auto ret = binning->set_value(value);

            if (!ret)
            {
                SPDLOG_ERROR("Unable to set binning: {}", ret.as_failure().error().message());

                return false;
            }
        }
        else
        {
            auto bin_hb = tcam::property::find_property(scale_.properties, "BinningHorizontal");
            auto bin_vb = tcam::property::find_property(scale_.properties, "BinningVertical");

            auto bin_h = dynamic_cast<tcam::property::IPropertyInteger*>(bin_hb.get());
            auto bin_v = dynamic_cast<tcam::property::IPropertyInteger*>(bin_vb.get());

            SPDLOG_TRACE("Setting binning to: {}x{}", scale.binning_h, scale.binning_v);

            auto ret = bin_h->set_value(scale.binning_h);

            if (!ret)
            {
                SPDLOG_ERROR("Unable to set binning: {}", ret.as_failure().error().message());
                return false;
            }

            ret = bin_v->set_value(scale.binning_v);

            if (!ret)
            {
                SPDLOG_ERROR("Unable to set binning: {}", ret.as_failure().error().message());
                return false;
            }
        }
    }

    if (scale_.scale_type == ImageScalingType::Skipping
        || scale_.scale_type == ImageScalingType::BinningSkipping)
    {
        auto sk_hb = tcam::property::find_property(scale_.properties, "DecimationHorizontal");
        auto sk_vb = tcam::property::find_property(scale_.properties, "DecimationVertical");

        auto sk_h = dynamic_cast<tcam::property::IPropertyInteger*>(sk_hb.get());
        auto sk_v = dynamic_cast<tcam::property::IPropertyInteger*>(sk_vb.get());

        SPDLOG_TRACE("Setting skipping to: {}x{}", scale.skipping_h, scale.skipping_v);
        auto ret = sk_h->set_value(scale.skipping_h);

        if (!ret)
        {
            SPDLOG_ERROR("Unable to set binning: {}", ret.as_failure().error().message());
            return false;
        }

        ret = sk_v->set_value(scale.skipping_v);

        if (!ret)
        {
            SPDLOG_ERROR("Unable to set binning: {}", ret.as_failure().error().message());
            return false;
        }
    }

    return true;
}


image_scaling AravisDevice::get_current_scaling()
{
    assert(scale_.scale_type != ImageScalingType::Unknown);

    image_scaling ret = {};

    if (scale_.scale_type == ImageScalingType::Binning
        || scale_.scale_type == ImageScalingType::BinningSkipping)
    {
        auto binning_base = tcam::property::find_property(scale_.properties, "Binning");

        if (binning_base)
        {
            auto binning = dynamic_cast<tcam::property::IPropertyEnum*>(binning_base.get());

            auto value = binning->get_value();

            if (!value)
            {
                SPDLOG_ERROR("Unable to retrieve value for Binning: {}",
                             value.as_failure().error().message());
                return {};
            }

            if (value.value() == "Off")
            {
                ret.binning_h = 1;
                ret.binning_v = 1;
            }
            else if (value.value() == "2x")
            {
                ret.binning_h = 2;
                ret.binning_v = 2;
            }
            else if (value.value() == "4x")
            {
                ret.binning_h = 4;
                ret.binning_v = 4;
            }
            else
            {
                SPDLOG_WARN("Binning entry '{}' not implemented.", value.value());
                return {};
            }
        }
        else
        {
            auto bin_hb = tcam::property::find_property(scale_.properties, "BinningHorizontal");
            auto bin_vb = tcam::property::find_property(scale_.properties, "BinningVertical");

            auto bin_h = dynamic_cast<tcam::property::IPropertyInteger*>(bin_hb.get());
            auto bin_v = dynamic_cast<tcam::property::IPropertyInteger*>(bin_vb.get());

            auto res = bin_h->get_value();

            if (!res)
            {
                SPDLOG_ERROR("Unable to retrieve value for BinningHorizontal: {}",
                             res.as_failure().error().message());
                return {};
            }
            ret.binning_h = res.value();
            res = bin_v->get_value();
            if (!res)
            {
                SPDLOG_ERROR("Unable to retrieve value for BinningVertical: {}",
                             res.as_failure().error().message());
                return {};
            }
            ret.binning_v = res.value();
        }
    }

    if (scale_.scale_type == ImageScalingType::Skipping
        || scale_.scale_type == ImageScalingType::BinningSkipping)
    {
        auto sk_h = tcam::property::find_property<tcam::property::IPropertyInteger>(scale_.properties, "DecimationHorizontal");
        auto sk_v = tcam::property::find_property<tcam::property::IPropertyInteger>(scale_.properties, "DecimationVertical");

        if (auto res = sk_h->get_value(); !res)
        {
            SPDLOG_ERROR("Unable to retrieve value for SkippingHorizontal: {}",
                         res.as_failure().error().message());
            return {};
        }
        else
        {
            ret.skipping_h = res.value();
        }

        if (auto res = sk_v->get_value(); !res)
        {
            SPDLOG_ERROR("Unable to retrieve value for SkippingVertical: {}",
                         res.as_failure().error().message());
            return {};
        }
        else
        {
            ret.skipping_v = res.value();
        }
    }

    return ret;
}
