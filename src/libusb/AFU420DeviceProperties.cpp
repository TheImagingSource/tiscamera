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

#include "AFU420Device.h"

#include "standard_properties.h"
#include "logging.h"

#include "utils.h"

#include <cmath>

using namespace tcam;


bool AFU420Device::create_exposure ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_EXPOSURE);

    prop.value.i.min = 100; //
    prop.value.i.max = 30000000; // 30 seconds
    prop.value.i.step = 100;

    //auto value = get_exposure();

    int value = 100;
    set_exposure(value);

    prop.value.i.value = value;
    prop.value.i.default_value = value;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});

    return true;
}


bool AFU420Device::create_gain ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_GAIN);

    prop.value.i.min = 64;
    prop.value.i.max = 520;
    prop.value.i.step = 1;

    auto value = get_gain();

    if (value == 0)
    {
        value = 292;
        set_gain(292);
    }

    prop.value.i.value = value;
    prop.value.i.default_value = value;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});

    return true;
}


bool AFU420Device::create_focus ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_FOCUS);

    prop.value.i.min = 0;
    prop.value.i.max = 1023;
    prop.value.i.step = 1;

    auto value = get_focus();

    prop.value.i.value = value;
    prop.value.i.default_value = value;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});

    return true;
}


bool AFU420Device::create_shutter ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_SHUTTER);

    prop.value.b.value = false;
    prop.value.b.default_value = false;

    auto property = std::make_shared<PropertyBoolean>(property_handler, prop, Property::BOOLEAN);

    property_handler->properties.push_back({property});

    return true;
}


bool AFU420Device::create_hdr ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_HDR);
    // hdr sets the exposure divider for the dark lines in the hdr image.
    // e.g. a factor of 1 disables hdr
    // a factor of 16 means that the dark lines use a exposure value of exposure / 16
    prop.value.i.min = 1;
    prop.value.i.max = 16;
    prop.value.i.step = 1;
    prop.value.i.value = 1;
    prop.value.i.default_value = 1;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});

    return true;
}


double color_gain_to_camera (double value)
{
    return map_value_ranges(0, 255, 0, (4.0 - (1.0 / 256.0)), value);
}


int camera_to_color_gain (double value)
{
    return map_value_ranges(0, (4.0 - (1.0 / 256.0)), 0, 255, value);
}


bool AFU420Device::create_color_gain ()
{
    /*
      gain works in a weird way.
      the values lie between 0 and (4.0 - (1.0 / 256.0))

      we map this to the more traditional 0 - 255

      this allows direct adjustments from the tcamwhitebalance gst module
     */

    auto prop = create_empty_property(TCAM_PROPERTY_GAIN_RED);

    prop.value.i.min = 0;
    prop.value.i.max = 255;
    prop.value.i.step = 1;

    double value = 0;
    bool ret = get_color_gain_factor(color_gain::ColorGainRed, value);

    prop.value.i.value = camera_to_color_gain(value);
    prop.value.i.default_value = 64;

    auto property = std::make_shared<PropertyDouble>(property_handler, prop, Property::FLOAT);

    property_handler->properties.push_back({property});

    /// gain green
    prop = create_empty_property(TCAM_PROPERTY_GAIN_GREEN);

    prop.value.i.min = 0;
    prop.value.i.max = 255;
    prop.value.i.step = 1;

    value = 0;
    ret = get_color_gain_factor(color_gain::ColorGainGreen1, value);

    prop.value.i.value = camera_to_color_gain(value);
    prop.value.i.default_value = 64;

    property = std::make_shared<PropertyDouble>(property_handler, prop, Property::FLOAT);

    property_handler->properties.push_back({property});


    /// gain blue
    prop = create_empty_property(TCAM_PROPERTY_GAIN_BLUE);

    prop.value.i.min = 0;
    prop.value.i.max = 255;
    prop.value.i.step = 1;

    value = 0;
    ret = get_color_gain_factor(color_gain::ColorGainBlue, value);

    prop.value.i.value = camera_to_color_gain(value);
    prop.value.i.default_value = 64;

    property = std::make_shared<PropertyDouble>(property_handler, prop, Property::FLOAT);

    property_handler->properties.push_back({property});

    return true;
}


bool AFU420Device::create_strobe ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_STROBE_ENABLE);

    prop.value.b.value = false;
    prop.value.b.default_value = false;

    property_handler->properties.push_back({std::make_shared<PropertyBoolean>(property_handler, prop, Property::BOOLEAN)});

    prop = create_empty_property(TCAM_PROPERTY_STROBE_DELAY);
    prop.value.i.min = 0;
    prop.value.i.max = 1700000;
    prop.value.i.step = 1;
    prop.value.i.value = get_strobe(strobe_parameter::first_strobe_delay);
    prop.value.i.default_value = prop.value.i.value;
    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    prop = create_empty_property(TCAM_PROPERTY_STROBE_DURATION);
    prop.value.i.min = 10;
    prop.value.i.max = 682000;
    prop.value.i.step = 1;
    prop.value.i.value = get_strobe(strobe_parameter::first_strobe_duration);
    prop.value.i.default_value = prop.value.i.value;
    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    prop = create_empty_property(TCAM_PROPERTY_STROBE_DELAY_SECOND);
    prop.value.i.min = 0;
    prop.value.i.max = 1700000;
    prop.value.i.step = 1;
    prop.value.i.value = get_strobe(strobe_parameter::second_strobe_delay);
    prop.value.i.default_value = prop.value.i.value;

    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    prop = create_empty_property(TCAM_PROPERTY_STROBE_DURATION_SECOND);
    prop.value.i.min = 10;
    prop.value.i.max = 682000;
    prop.value.i.step = 1;
    prop.value.i.value = get_strobe(strobe_parameter::second_strobe_duration);
    prop.value.i.default_value = prop.value.i.value;
    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    prop = create_empty_property(TCAM_PROPERTY_STROBE_POLARITY);
    prop.value.b.value = false;
    prop.value.b.default_value = false;
    property_handler->properties.push_back({std::make_shared<PropertyBoolean>(property_handler, prop, Property::BOOLEAN)});

    prop = create_empty_property(TCAM_PROPERTY_STROBE_MODE);
    prop.value.i.min = 1;
    prop.value.i.max = 2;
    prop.value.i.step = 1;
    prop.value.i.value = get_strobe(strobe_parameter::mode);
    prop.value.i.default_value = 1;

    std::map<std::string, int> mode_map;
    mode_map.emplace("Single Strobe", 1);
    mode_map.emplace("Double Strobe", 2);

    property_handler->properties.push_back({std::make_shared<PropertyEnumeration>(property_handler, prop, mode_map, Property::ENUM)});

    return true;
}


bool AFU420Device::create_offsets ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_OFFSET_X);

    prop.value.i.min = 0;
    prop.value.i.max = 7463; //m_uPixelMaxX - m_uPixelMinX;
    prop.value.i.step = 12;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});





    prop = create_empty_property(TCAM_PROPERTY_OFFSET_Y);

    prop.value.i.min = 0;
    prop.value.i.max = 5115; //m_uPixelMaxY - m_uPixelMinY;
    prop.value.i.step = 4;

    property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});




    prop = create_empty_property(TCAM_PROPERTY_OFFSET_AUTO);

    prop.value.b.value = false;
    prop.value.b.default_value = false;

    auto property_auto = std::make_shared<PropertyBoolean>(property_handler, prop, Property::BOOLEAN);

    property_handler->properties.push_back({property_auto});
}


bool AFU420Device::create_binning ()
{
    auto ptr = create_binning_property(TCAM_PROPERTY_BINNING_HORIZONTAL,
                                       property_handler, 1, 8, 1, 1);

    if (ptr == nullptr)
    {
        tcam_error("Could not create binning property. Continuing without.");
    }
    else
    {
        property_handler->properties.push_back({ptr});
    }

    ptr = create_binning_property(TCAM_PROPERTY_BINNING_VERTICAL,
                                  property_handler, 1, 8, 1, 1);

    if (ptr == nullptr)
    {
        tcam_error("Could not create binning property. Continuing without.");
    }
    else
    {
        property_handler->properties.push_back({ptr});
    }

    return true;
}


bool AFU420Device::create_ois ()
{
    tcam_device_property prop = create_empty_property(TCAM_PROPERTY_OIS_MODE);

    prop.value.i.min = 1;
    prop.value.i.max = 6;
    prop.value.i.step = 1;
    prop.value.i.value = get_ois_mode();
    prop.value.i.default_value = 6;

    std::map<std::string, int> map;
    map.emplace("OFF", 6);
    map.emplace("ON with still mode without pan-tilt", 1);
    map.emplace("ON with movie mode without pan-tilt", 2);
    map.emplace("ON with movie mode with pan-tilt", 3);
    map.emplace("ON with center cervo", 4);
    map.emplace("ON Circle Mode", 5);

    auto ois_mode = std::make_shared<PropertyEnumeration>(property_handler, prop, map, Property::ENUM);

    property_handler->properties.push_back({ois_mode});

    int64_t x_pos, y_pos;
    get_ois_pos(x_pos, y_pos);

    prop = create_empty_property(TCAM_PROPERTY_OIS_POS_X);
    prop.value.i.min = -90;
    prop.value.i.max = 90;
    prop.value.i.step = 1;
    prop.value.i.value = x_pos;
    prop.value.i.default_value = 0;
    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    prop = create_empty_property(TCAM_PROPERTY_OIS_POS_Y);
    prop.value.i.min = -90;
    prop.value.i.max = 90;
    prop.value.i.step = 1;
    prop.value.i.value = y_pos;
    prop.value.i.default_value = 0;
    property_handler->properties.push_back({std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER)});


    return true;
}


void AFU420Device::create_properties ()
{
    create_exposure();
    create_gain();
    create_hdr();

    if (has_ois_unit())
    {
        create_focus();
        create_shutter();

        //create_ois();
    }

    create_color_gain();
    //create_strobe();
    create_binning();
    create_offsets();
}


bool AFU420Device::update_property (tcam::AFU420Device::property_description &desc)
{
    switch(desc.property->get_ID())
    {
        case TCAM_PROPERTY_EXPOSURE:
        {
            auto value = get_exposure();
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_GAIN:
        {
            return false; // gain not readable
            auto value = get_gain();
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_FOCUS:
        {
            auto value = get_focus();
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_SHUTTER:
        {
            desc.property->set_value(get_shutter());
            return true;
        }
        case TCAM_PROPERTY_GAIN_RED:
        {
            auto value = 0.0;
            get_color_gain_factor(color_gain::ColorGainRed, value);
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_GAIN_GREEN:
        {
            auto value = 0.0;
            get_color_gain_factor(color_gain::ColorGainGreen1, value);
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_GAIN_BLUE:
        {
            auto value = 0.0;
            get_color_gain_factor(color_gain::ColorGainBlue, value);
            desc.property->set_value(value);
            return true;
        }
        case TCAM_PROPERTY_BINNING_HORIZONTAL:
        {
            desc.property->set_value((int64_t)active_resolution_conf_.x_addr_start);
            return true;
        }
        case TCAM_PROPERTY_BINNING_VERTICAL:
        {
            desc.property->set_value((int64_t)active_resolution_conf_.y_addr_start);
            return true;
        }
        case TCAM_PROPERTY_STROBE_ENABLE:
        {
            break;
        }
        case TCAM_PROPERTY_STROBE_DELAY:
        {
            uint32_t value = get_strobe(strobe_parameter::first_strobe_delay);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_STROBE_DELAY_SECOND:
        {
            uint32_t value = get_strobe(strobe_parameter::second_strobe_delay);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_STROBE_DURATION:
        {
            uint32_t value = get_strobe(strobe_parameter::first_strobe_duration);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_STROBE_DURATION_SECOND:
        {
            uint32_t value = get_strobe(strobe_parameter::second_strobe_duration);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_STROBE_POLARITY:
        {
            uint32_t value = get_strobe(strobe_parameter::polarity);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_STROBE_MODE:
        {
            uint32_t value = get_strobe(strobe_parameter::mode);
            desc.property->set_value((int64_t)value);
            return true;
        }
        case TCAM_PROPERTY_OIS_MODE:
        {
            desc.property->set_value((int64_t)get_ois_mode());
            return true;
        }
        case TCAM_PROPERTY_OIS_POS_X:
        {
            int64_t x_pos, y_pos;
            get_ois_pos(x_pos, y_pos);
            desc.property->set_value(x_pos);
            return true;
        }
        case TCAM_PROPERTY_OIS_POS_Y:
        {
            int64_t x_pos, y_pos;
            get_ois_pos(x_pos, y_pos);
            desc.property->set_value(y_pos);
            return true;
        }
        default:
        {
            tcam_warning("Property does not belong to this device");
            break;
        }
    }

    return false;
}


int64_t AFU420Device::get_exposure ()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_GET_EXPOSURE);

    if (ret < 0)
    {
        tcam_error("Unable to read property 'Exposure. LibUsb returned %d", ret);
    }
    else
    {
        tcam_debug("exposure returned value: %u", value);
    }
    return value / 1000;
}


bool AFU420Device::set_exposure (int64_t exposure)
{
    uint16_t value = exposure * 1000;

    int ret = control_write(BASIC_PC_TO_USB_EXPOSURE, value);

    if (ret < 0)
    {
        tcam_error("Unable to write property 'Exposure'. LibUsb returned %d", ret);
        return false;
    }
    else
    {
        //tcam_debug("Exposure returned value: %u", value);
    }
    return true;
}


int64_t AFU420Device::get_gain ()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_PC_TO_USB_GAIN);

    if (ret < 0)
    {
        tcam_error("Unable to read property 'Gain'. LibUsb returned %d", ret);
    }
    else
    {
        tcam_debug("Gain returned value: %u", value / 100);
    }
    return value / 100;
}


bool AFU420Device::set_gain (int64_t gain)
{
    uint16_t value = gain;

    int ret = control_write(BASIC_PC_TO_USB_GAIN, value);

    if (ret < 0)
    {
        tcam_error("Unable to write property 'Gain'. LibUsb returned %d", ret);
        return false;
    }
    else
    {
        //tcam_debug("Gain value: %u written", value);
    }
    return true;
}


int64_t AFU420Device::get_focus ()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_FOCUS);

    if (ret < 0)
    {
        tcam_error("Unable to read property 'Focus'. LibUsb returned %d", ret);
        return ret;
    }
    else
    {
        tcam_debug("Focus returned value: %u", value);
    }
    return value;
}


bool AFU420Device::set_focus (int64_t focus)
{
    uint16_t value = focus;

    int ret = control_write(BASIC_PC_TO_USB_FOCUS, value);

    if (ret < 0)
    {
        tcam_error("Unable to write property 'Focus'. LibUsb returned %d", ret);
        return false;
    }
    else
    {
        tcam_debug("Gain value: %u written", value);
    }
    return true;
}


bool AFU420Device::set_shutter (bool open)
{
    unsigned short ushValue = open ? 0xFFFF : 0x0;
    int ret = control_write(BASIC_PC_TO_USB_SHUTTER, ushValue);

    if (ret < 0)
    {
        tcam_error("Could not write Shutter flag.");
        return false;
    }
    return true;
}


bool AFU420Device::get_shutter ()
{
    unsigned short ushValue = 0x0;
    int ret = control_read(ushValue, BASIC_PC_TO_USB_SHUTTER);

    if (ret < 0)
    {
        tcam_error("Could not write Shutter flag.");
        return false;
    }

    if (ushValue == 0xFFFF)
    {
        return true;
    }
    else
    {
        return false;
    }
}


int64_t AFU420Device::get_hdr ()
{
    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_HDR);

    if (ret < 0)
    {
        tcam_error("Could not read hdr. Libusb returned %d", ret);
    }

    return value;
}


bool AFU420Device::set_hdr (int64_t hdr)
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
        tcam_error("Could not write hdr value. Libusb returned %d", ret);
        return false;
    }
    return true;
}


bool AFU420Device::get_color_gain_factor (color_gain eColor, double& dValue)
{
    unsigned short ushColor = 0;
    switch (eColor)
    {
        case color_gain::ColorGainRed:        ushColor = 1; break;
        case color_gain::ColorGainGreen1:     ushColor = 0; break;
        case color_gain::ColorGainGreen2:     ushColor = 3; break;
        case color_gain::ColorGainBlue:       ushColor = 2; break;
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


bool AFU420Device::set_color_gain_factor (color_gain eColor, int value)
{
    double dValue = color_gain_to_camera(value);

    if (!((dValue >= 0.0) && (dValue <= (4.0 - (1.0 / 256.0)))))
    {
        tcam_error("color gain is out of bounds %f", dValue);
        return false;
    }

    uint16_t ushValue = uint16_t(dValue);
    dValue -= (double)ushValue;

    ushValue <<= 8;

    ushValue |= (unsigned short)round(dValue * 256.0);

    unsigned short ushColor = 0;
    switch( eColor )
    {
        case color_gain::ColorGainRed:        ushColor = 1; break;;
        case color_gain::ColorGainGreen1:     ushColor = 0; break;
        case color_gain::ColorGainGreen2:     ushColor = 3; break;
        case color_gain::ColorGainBlue:       ushColor = 2; break;
        default:
            return false;
    }

    int ret = control_write(ADVANCED_PC_TO_USB_COLOR_GAIN, ushValue, ushColor);

    if (ret < 0)
    {
        tcam_error("Could not read color gain value. Libsub returned %d", ret);
        return false;
    }

	return true;
}


int AFU420Device::read_strobe (strobe_data& strobe)
{
    int ret = usb_device_->control_transfer(DEVICE_TO_HOST,
                                            BASIC_PC_TO_USB_FLASH_STROBE, 0, 5,
                                            (unsigned char*) &strobe,sizeof(strobe));

    if (ret < 0)
    {
        tcam_error("Could not read strobe. Libusb returned %d", ret);
    }
    return ret;
}


int64_t AFU420Device::get_strobe (strobe_parameter param)
{
    uint32_t value;
    int ret = 0;
	if (param == strobe_parameter::polarity)
    {
		ret = control_read(value, BASIC_PC_TO_USB_FLASH_STROBE, 0, 5);
	}

	strobe_data tmp = {};
	ret = read_strobe(tmp);
	if (ret < 0)
    {
        return -1;
	}

    switch (param)
    {
        case strobe_parameter::mode:                    value = tmp.mode; break;
        case strobe_parameter::first_strobe_delay:      value = tmp.delay_control; break;
        case strobe_parameter::first_strobe_duration:   value = tmp.width_high_ctrl; break;
        case strobe_parameter::second_strobe_delay:     value = tmp.width_low_ctrl; break;
        case strobe_parameter::second_strobe_duration:  value = tmp.width2_high_ctrl; break;
        default: return -1;
    };
    return value;
}


bool AFU420Device::set_strobe (strobe_parameter param, int64_t strobe)
{
    uint16_t value = strobe;
    int ret = 0;
    if (param == strobe_parameter::mode)
    {
        int ret = usb_device_->control_transfer(HOST_TO_DEVICE,
                                                BASIC_PC_TO_USB_FLASH_STROBE,
                                                value, 0,
                                                value);
    }
    else
    {
        int ret = usb_device_->control_transfer(HOST_TO_DEVICE,
                                                BASIC_PC_TO_USB_FLASH_STROBE,
                                                0, (uint16_t)param,
                                                value);
    }

    if (ret < 0)
    {
        tcam_error("Could not write strobe. Libusb returned %d", ret);
        return false;
    }

    return true;
}


int64_t AFU420Device::get_ois_mode ()
{
    uint16_t mode = 0;

    int ret = control_read(mode, ADVANCED_PC_TO_USB_OIS_MODE);

    if (ret < 0)
    {
        tcam_error("Could not read ois mode. Libusb returned %d", ret);
        return ret;
    }
    return mode;
}


bool AFU420Device::set_ois_mode (int64_t mode)
{
    int ret = control_write(ADVANCED_PC_TO_USB_OIS_MODE, (uint16_t)mode);

    if (ret < 0)
    {
        tcam_error("Could not write ois mode. Libusb returned %d", ret);

        return false;
    }
    return true;
}


bool AFU420Device::get_ois_pos (int64_t& x_pos, int64_t& y_pos)
{
    return false;
}


bool AFU420Device::set_ois_pos (const int64_t& x_pos, const int64_t& y_pos)
{
    int ret = control_write(ADVANCED_PC_TO_USB_OIS_POS, (uint16_t)x_pos, (uint16_t)y_pos);

    if (ret < 0)
    {
        tcam_error("Could not write OIS position. Libusb returned %d", ret);
        return false;
    }

    return true;
}
