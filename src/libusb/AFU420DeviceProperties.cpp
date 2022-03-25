/*
 * Copyright 2017 The Imaging Source Europe GmbH
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
#include "../utils.h"
#include "AFU420Device.h"
#include "AFU420DeviceBackend.h"
#include "AFU420PropertyImpl.h"

#include <cmath>

using namespace tcam;
using namespace tcam::property;

bool AFU420Device::create_exposure()
{
    tcam_value_double d = {};
    d.min = 100.0;
    d.max = 30'000'000.0;
    d.step = 100.0;
    d.value = 100.0;
    d.default_value = 100.0;

    auto exp = std::make_shared<AFU420PropertyDoubleImpl>(
        "ExposureTime", d, tcam::afu420::AFU420Property::ExposureTime, m_backend);

    set_exposure(100.0);

    m_properties.push_back(exp);

    return true;
}


bool AFU420Device::create_gain()
{
    tcam_value_double d = {};
    d.min = 64.0;
    d.max = 520.0;
    d.step = 1.0;

    auto value = get_gain();

    if (value == 0)
    {
        d.value = 292;
        set_gain(292);
    }
    else
    {
        d.value = value;
    }
    d.default_value = 292.0;

    auto exp = std::make_shared<AFU420PropertyDoubleImpl>(
        "Gain", d, tcam::afu420::AFU420Property::Gain, m_backend);

    m_properties.push_back(exp);

    return true;
}


bool AFU420Device::create_focus()
{
    auto value = get_focus();

    tcam_value_int i = {};
    i.min = 0;
    i.max = 1023;
    i.step = 1;
    i.default_value = value;

    auto exp = std::make_shared<AFU420PropertyIntegerImpl>(
        "Focus", i, tcam::afu420::AFU420Property::Focus, m_backend);

    m_properties.push_back(exp);

    return true;
}


bool AFU420Device::create_iris()
{
    set_iris(m_iris);

    tcam_value_int i = {};

    i.min = 0;
    i.max = 1;
    i.step = 1;
    i.value = 0;
    i.default_value = 0;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
                           "Iris", i, tcam::afu420::AFU420Property::Iris, m_backend));

    return true;
}


bool AFU420Device::create_hdr()
{
    //auto prop = create_empty_property(TCAM_PROPERTY_HDR);
    // hdr sets the exposure divider for the dark lines in the hdr image.
    // e.g. a factor of 1 disables hdr
    // a factor of 16 means that the dark lines use a exposure value of exposure / 16

    tcam_value_int i = {};
    i.min = 1;
    i.max = 16;
    i.step = 1;
    i.value = 1;
    i.default_value = 1;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
        "HDR", i, tcam::afu420::AFU420Property::HDR, m_backend));

    return true;
}

#define COLOR_MAX (4.0-(1.0/256.0))


bool AFU420Device::create_color_gain()
{
    /*
      gain works in a weird way.
      the values lie between 0 and (4.0 - (1.0 / 256.0))

     */

    tcam_value_double ir = {};
    ir.min = 0;
    ir.max = COLOR_MAX;
    ir.step = 0.1;

    double value = 0;
    get_color_gain_factor(color_gain::ColorGainRed, value);

    ir.value = value;

    ir.default_value = 1.0;

    m_properties.push_back(std::make_shared<AFU420PropertyDoubleImpl>(
        "BalanceWhiteRed", ir, tcam::afu420::AFU420Property::WB_Red, m_backend));


    /// gain green

    tcam_value_double ig = {};
    ig.min = 0;
    ig.max = COLOR_MAX;
    ig.step = 0.1;

    value = 0;
    get_color_gain_factor(color_gain::ColorGainGreen1, value);

    ig.value = value;

    ig.default_value = 1.0;

    m_properties.push_back(std::make_shared<AFU420PropertyDoubleImpl>(
        "BalanceWhiteGreen", ig, tcam::afu420::AFU420Property::WB_Green, m_backend));

    /// gain blue

    tcam_value_double ib = {};
    ib.min = 0;
    ib.max = COLOR_MAX;
    ib.step = 0.1;

    value = 0;
    get_color_gain_factor(color_gain::ColorGainBlue, value);

    ib.value = value;

    ib.default_value = 1.0;

    m_properties.push_back(std::make_shared<AFU420PropertyDoubleImpl>(
        "BalanceWhiteBlue", ib, tcam::afu420::AFU420Property::WB_Blue, m_backend));

    return true;
}


bool AFU420Device::create_strobe()
{
    // std::map<int, std::string> enable_entries = {{{0, "Off"}, {1, "On"}}};
    // m_properties.push_back(std::make_shared<AFU420PropertyEnumImpl>("StrobeEnable", tcam::afu420::AFU420Property::StrobeEnable,
    //                                                                 enable_entries,
    //                                                                 m_backend));

    tcam_value_int i_sde = {};
    i_sde.min = 0;
    i_sde.max = 1700000;
    i_sde.step = 1;
    i_sde.value = get_strobe(strobe_parameter::first_strobe_delay);
    i_sde.default_value = i_sde.value;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
        "StrobeDelay", i_sde, tcam::afu420::AFU420Property::StrobeDelay, m_backend));

    tcam_value_int i_sdu = {};
    i_sdu.min = 10;
    i_sdu.max = 682000;
    i_sdu.step = 1;
    i_sdu.value = get_strobe(strobe_parameter::first_strobe_duration);
    i_sdu.default_value = i_sdu.value;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
        "StrobeDuration", i_sdu, tcam::afu420::AFU420Property::StrobeDuration, m_backend));


    tcam_value_int i_sd = {};
    i_sd.min = 0;
    i_sd.max = 1700000;
    i_sd.step = 1;
    i_sd.value = get_strobe(strobe_parameter::second_strobe_delay);
    i_sd.default_value = i_sd.value;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
        "StrobeDelaySecond", i_sd, tcam::afu420::AFU420Property::StrobeDelaySecond, m_backend));

    tcam_value_int i_sds = {};
    i_sds.min = 10;
    i_sds.max = 682000;
    i_sds.step = 1;
    i_sds.value = get_strobe(strobe_parameter::second_strobe_duration);
    i_sds.default_value = i_sds.value;

    m_properties.push_back(std::make_shared<AFU420PropertyIntegerImpl>(
        "StrobeDurationSecond",
        i_sds,
        tcam::afu420::AFU420Property::StrobeDurationSecond,
        m_backend));


    std::map<int, std::string> polarity_entries = { { { 0, "ActiveLow" }, { 1, "ActiveHigh" } } };
    m_properties.push_back(
        std::make_shared<AFU420PropertyEnumImpl>("StrobePolarity",
                                                 tcam::afu420::AFU420Property::StrobePolarity,
                                                 polarity_entries,
                                                 m_backend));


    std::map<int, std::string> mode_map;
    mode_map.emplace(1, "Single Strobe");
    mode_map.emplace(2, "Double Strobe");

    m_properties.push_back(std::make_shared<AFU420PropertyEnumImpl>(
        "StrobeMode", tcam::afu420::AFU420Property::StrobeMode, mode_map, m_backend));

    return true;
}


bool AFU420Device::create_offsets()
{
    tcam_value_int i_ox = {};

    i_ox.min = 0;
    i_ox.max = 7463; //m_uPixelMaxX - m_uPixelMinX;
    i_ox.step = 12;

    auto offset_x = std::make_shared<AFU420PropertyIntegerImpl>(
        "OffsetX", i_ox, tcam::afu420::AFU420Property::OffsetX, m_backend);

    m_properties.push_back(offset_x);

    tcam_value_int i_oy = {};

    i_oy.min = 0;
    i_oy.max = 5115; //m_uPixelMaxY - m_uPixelMinY;
    i_oy.step = 4;

    auto offset_y = std::make_shared<AFU420PropertyIntegerImpl>(
        "OffsetY", i_oy, tcam::afu420::AFU420Property::OffsetY, m_backend);

    m_properties.push_back(offset_y);

    std::map<int, std::string> offset_entries = { { { 0, "Off" }, { 1, "On" } } };

    auto offset_auto = std::make_shared<AFU420PropertyEnumImpl>(
        "OffsetAutoCenter", tcam::afu420::AFU420Property::OffsetAuto, offset_entries, m_backend);

    m_properties.push_back(offset_auto);

    std::vector<std::weak_ptr<tcam::property::PropertyLock>> can_be_locked;

    can_be_locked.push_back(std::dynamic_pointer_cast<tcam::property::PropertyLock>(offset_x));
    can_be_locked.push_back(std::dynamic_pointer_cast<tcam::property::PropertyLock>(offset_y));

    std::dynamic_pointer_cast<tcam::property::PropertyLock>(offset_auto)->set_dependent_properties(std::move(can_be_locked));

    return true;
}


bool AFU420Device::create_ois()
{
    std::map<int, std::string> map2;
    map2.emplace(1, "ON with still mode without pan-tilt");
    map2.emplace(2, "ON with movie mode without pan-tilt");
    map2.emplace(3, "ON with movie mode with pan-tilt");
    map2.emplace(4, "ON with center cervo");
    map2.emplace(5, "ON Circle Mode");
    map2.emplace(6, "OFF");

    m_properties.push_back(std::make_shared<AFU420PropertyEnumImpl>(
        "OISMode", tcam::afu420::AFU420Property::OISMode, map2, m_backend));


    int64_t x_pos = 0;
    int64_t y_pos = 0;
    // not implemented
    // get_ois_pos(x_pos, y_pos);

    tcam_value_int pos_x = {};

    pos_x.min = -90;
    pos_x.max = 90;
    pos_x.step = 1;
    pos_x.value = x_pos;
    pos_x.default_value = 0;

    m_properties.push_back(std::make_shared<tcam::property::AFU420PropertyIntegerImpl>(
        "OISPosX", pos_x, tcam::afu420::AFU420Property::OISPosX, m_backend));

    tcam_value_int pos_y = {};

    pos_y.min = -90;
    pos_y.max = 90;
    pos_y.step = 1;
    pos_y.value = y_pos;
    pos_y.default_value = 0;

    m_properties.push_back(std::make_shared<tcam::property::AFU420PropertyIntegerImpl>(
        "OISPosY", pos_y, tcam::afu420::AFU420Property::OISPosY, m_backend));

    return true;
}


void AFU420Device::create_sensor_dimensions()
{
    tcam_value_int width = {};

    width.min = max_sensor_dim_.width;
    width.max = max_sensor_dim_.width;
    width.step = 0;
    width.value = max_sensor_dim_.width;
    width.default_value = max_sensor_dim_.width;

    m_properties.push_back(std::make_shared<tcam::property::AFU420PropertyIntegerImpl>(
                               "SensorWidth", width, tcam::afu420::AFU420Property::SensorWidth, m_backend));

    tcam_value_int height = {}; 

    height.min = max_sensor_dim_.height;
    height.max = max_sensor_dim_.height;
    height.step = 0;
    height.value = max_sensor_dim_.height;
    height.default_value = max_sensor_dim_.height;

    m_properties.push_back(std::make_shared<tcam::property::AFU420PropertyIntegerImpl>(
                               "SensorHeight", height, tcam::afu420::AFU420Property::SensorHeight, m_backend));

}


void AFU420Device::create_properties()
{
    create_exposure();
    create_gain();
    // currently not active
    // leave in as someone might want this in the future
    //create_hdr();

    if (has_ois_unit())
    {
        create_focus();
        create_iris();

        //create_ois();
    }

    create_color_gain();
    //create_strobe();
    create_offsets();
    create_sensor_dimensions();
}


int64_t AFU420Device::get_exposure()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_GET_EXPOSURE);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to read property 'Exposure. LibUsb returned {}", ret);
    }
    return value;
}


bool AFU420Device::set_exposure(int64_t exposure)
{
    uint16_t value = exposure;

    int ret = control_write(BASIC_PC_TO_USB_EXPOSURE, value);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to write property 'Exposure'. LibUsb returned {}", ret);
        return false;
    }

    return true;
}


int64_t AFU420Device::get_gain()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_PC_TO_USB_GAIN);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to read property 'Gain'. LibUsb returned {}", ret);
    }
    else
    {
        SPDLOG_DEBUG("Gain returned value: {}", value / 100);
    }
    return value / 100;
}


bool AFU420Device::set_gain(int64_t gain)
{
    uint16_t value = gain;

    int ret = control_write(BASIC_PC_TO_USB_GAIN, value);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to write property 'Gain'. LibUsb returned {}", ret);
        return false;
    }
    return true;
}


int64_t AFU420Device::get_focus()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_FOCUS);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to read property 'Focus'. LibUsb returned {}", ret);
        return ret;
    }
    return value;
}


bool AFU420Device::set_focus(int64_t focus)
{
    uint16_t value = focus;

    int ret = control_write(BASIC_PC_TO_USB_FOCUS, value);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to write property 'Focus'. LibUsb returned {}", ret);
        return false;
    }
    return true;
}


bool AFU420Device::set_iris(bool open)
{
    unsigned short ushValue = open ? 0xFFFF : 0x0;
    int ret = control_write(BASIC_PC_TO_USB_SHUTTER, ushValue);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not write Iris flag.");
        return false;
    }
    return true;
}


int64_t AFU420Device::get_hdr()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_HDR);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not read hdr. Libusb returned {}", ret);
    }

    return value;
}


bool AFU420Device::set_hdr(int64_t hdr)
{
    if (hdr == 1)
    {
        hdr = 0;
    }

    uint16_t exposure_ratio = uint16_t(hdr);
    uint16_t on_off = hdr <= 1 ? 0 : 1;

    int ret = control_write(BASIC_PC_TO_USB_HDR, on_off, exposure_ratio);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not write hdr value. Libusb returned {}", ret);
        return false;
    }
    return true;
}


bool AFU420Device::get_color_gain_factor(color_gain eColor, double& dValue)
{
    unsigned short ushColor = 0;
    switch (eColor)
    {
        case color_gain::ColorGainRed:
            ushColor = 1;
            break;
        case color_gain::ColorGainGreen1:
            ushColor = 0;
            break;
        case color_gain::ColorGainGreen2:
            ushColor = 3;
            break;
        case color_gain::ColorGainBlue:
            ushColor = 2;
            break;
        default:
            return false;
            break;
    }

    unsigned short ushSelectedGain = 0;
    int hr = control_read(ushSelectedGain, ADVANCED_USB_TO_PC_COLOR_GAIN, 0, ushColor);
    if (hr < 0)
    {
        dValue = 0.0;
        return false;
    }

    double dLowerPart = (double)(ushSelectedGain & 0xFF) / 256.0;
    double dUpperPart = (double)((ushSelectedGain >> 8) & 0xFF);

    dValue = dLowerPart + dUpperPart;

    return true;
}


bool AFU420Device::set_color_gain_factor(color_gain eColor, double value)
{
    double dValue = value;

    if (!((dValue >= 0.0) && (dValue <= (4.0 - (1.0 / 256.0)))))
    {
        SPDLOG_ERROR("color gain is out of bounds {}", dValue);
        return false;
    }

    uint16_t ushValue = uint16_t(dValue);
    dValue -= (double)ushValue;

    ushValue <<= 8;

    ushValue |= (unsigned short)round(dValue * 256.0);

    unsigned short ushColor = 0;
    switch (eColor)
    {
        case color_gain::ColorGainRed:
            ushColor = 1;
            break;
            ;
        case color_gain::ColorGainGreen1:
            ushColor = 0;
            break;
        case color_gain::ColorGainGreen2:
            ushColor = 3;
            break;
        case color_gain::ColorGainBlue:
            ushColor = 2;
            break;
        default:
            return false;
    }

    int ret = control_write(ADVANCED_PC_TO_USB_COLOR_GAIN, ushValue, ushColor);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not set color gain value. Libsub returned {}", ret);
        return false;
    }

    return true;
}


int AFU420Device::read_strobe(strobe_data& strobe)
{
    int ret = usb_device_->control_transfer(DEVICE_TO_HOST,
                                            BASIC_PC_TO_USB_FLASH_STROBE,
                                            0,
                                            5,
                                            (unsigned char*)&strobe,
                                            sizeof(strobe));

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not read strobe. Libusb returned {}", ret);
    }
    return ret;
}


int64_t AFU420Device::get_strobe(strobe_parameter param)
{
    uint32_t value;
    int ret = 0;
    if (param == strobe_parameter::polarity)
    {
        ret = control_read(value, BASIC_PC_TO_USB_FLASH_STROBE, 0, 5);
    }

    if (ret < 0)
    {
        return -1;
    }

    strobe_data tmp = {};
    ret = read_strobe(tmp);
    if (ret < 0)
    {
        return -1;
    }

    switch (param)
    {
        case strobe_parameter::mode:
            value = tmp.mode;
            break;
        case strobe_parameter::first_strobe_delay:
            value = tmp.delay_control;
            break;
        case strobe_parameter::first_strobe_duration:
            value = tmp.width_high_ctrl;
            break;
        case strobe_parameter::second_strobe_delay:
            value = tmp.width_low_ctrl;
            break;
        case strobe_parameter::second_strobe_duration:
            value = tmp.width2_high_ctrl;
            break;
        default:
            return -1;
    };
    return value;
}


bool AFU420Device::set_strobe(strobe_parameter param, int64_t strobe)
{
    uint16_t value = strobe;
    int ret = 0;
    if (param == strobe_parameter::mode)
    {
        ret = usb_device_->control_transfer(
            HOST_TO_DEVICE, BASIC_PC_TO_USB_FLASH_STROBE, value, 0, value);
    }
    else
    {
        ret = usb_device_->control_transfer(
            HOST_TO_DEVICE, BASIC_PC_TO_USB_FLASH_STROBE, 0, (uint16_t)param, value);
    }

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not write strobe. Libusb returned {}", ret);
        return false;
    }

    return true;
}


int64_t AFU420Device::get_ois_mode()
{
    uint16_t mode = 0;

    int ret = control_read(mode, ADVANCED_PC_TO_USB_OIS_MODE);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not read ois mode. Libusb returned {}", ret);
        return ret;
    }
    return mode;
}


bool AFU420Device::set_ois_mode(int64_t mode)
{
    int ret = control_write(ADVANCED_PC_TO_USB_OIS_MODE, (uint16_t)mode);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not write ois mode. Libusb returned {}", ret);

        return false;
    }
    return true;
}


bool AFU420Device::get_ois_pos(int64_t& x_pos __attribute__((unused)),
                               int64_t& y_pos __attribute__((unused)))
{
    return false;
}


bool AFU420Device::set_ois_pos(const int64_t& x_pos, const int64_t& y_pos)
{
    int ret = control_write(ADVANCED_PC_TO_USB_OIS_POS, (uint16_t)x_pos, (uint16_t)y_pos);

    if (ret < 0)
    {
        SPDLOG_ERROR("Could not write OIS position. Libusb returned {}", ret);
        return false;
    }

    return true;
}
