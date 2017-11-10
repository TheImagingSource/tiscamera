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

    prop.value.i.min = 10; //
    prop.value.i.max = 300000; // 30 seconds
    prop.value.i.step = 1;

    auto value = get_exposure();

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
    auto prop = create_empty_property(TCAM_PROPERTY_FOCUS);

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
{}


bool AFU420Device::create_offsets ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_OFFSET_X);

    prop.value.i.min = 0;
    prop.value.i.max = m_uPixelMaxX - m_uPixelMinX;
    prop.value.i.step = 12;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});





    prop = create_empty_property(TCAM_PROPERTY_OFFSET_Y);

    prop.value.i.min = 0;
    prop.value.i.max = m_uPixelMaxY - m_uPixelMinY;
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


void AFU420Device::create_properties ()
{
    create_exposure();
    create_gain();

    create_hdr();

    // roi x
    // roi y

    // OIS_mode

    if (has_ois_unit())
    {
        create_focus();
        create_shutter();


    }

    create_color_gain();
    // strobe
    create_binning();
    create_offsets();
    // offset x
    // offset y
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
        tcam_debug("Gain returned value: %u", value);
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
        tcam_debug("Gain value: %u written", value);
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
