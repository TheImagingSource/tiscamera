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

using namespace tcam;


void AFU420Device::create_property (struct property_description desc)
{

    // auto def = create_empty_property(desc.definition.id);

    // uint16_t value = 0;

    // int ret = control_read(value, desc.prop_out);

    // if (ret < 0)
    // {
    //     //tcam_error("Unable to read property '%s'. LibUsb returned %d",
    //     //         desc.property->get_name().c_str(), ret);

    //     tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    // }
    // else
    // {
    //     tcam_debug("read '%s' value: %u", property_id2string(desc.definition.id).c_str(), value);
    // }

    // //uint32_t value = property_read(desc);

    // double modifier = 1;

    // if (desc.definition.id == TCAM_PROPERTY_EXPOSURE)
    // {
    //     modifier = 0.001;
    // }

    // def.value = desc.definition.value;
    // def.value.i.value = value * modifier;
    // def.value.i.default_value = value * modifier;

    // desc.property = std::make_shared<PropertyInteger>(property_handler, def, Property::INTEGER);

    // property_handler->properties.push_back(desc);
}


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
}


bool AFU420Device::create_focus ()
{
    auto prop = create_empty_property(TCAM_PROPERTY_FOCUS);

    prop.value.i.min = 0; //
    prop.value.i.max = 1023; // 30 seconds
    prop.value.i.step = 1;

    auto value = get_focus();

    prop.value.i.value = value;
    prop.value.i.default_value = value;

    auto property = std::make_shared<PropertyInteger>(property_handler, prop, Property::INTEGER);

    property_handler->properties.push_back({property});
}


void AFU420Device::create_properties ()
{

    property_handler->properties = std::vector<property_description>();

    tcam_device_property prop = {};

    // prop.id = TCAM_PROPERTY_EXPOSURE;
    // prop.type = TCAM_PROPERTY_TYPE_INTEGER;

    // create_property({nullptr, BASIC_PC_TO_USB_EXPOSURE, BASIC_USB_TO_PC_GET_EXPOSURE, 0, 0, prop});

    create_exposure();
    create_gain();
    create_focus();
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
    }

    return false;
}


int64_t AFU420Device::get_exposure ()
{

    uint16_t value = 0;

    int ret = control_read(value, BASIC_USB_TO_PC_GET_EXPOSURE);

    if (ret < 0)
    {
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("exposure !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("gain !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    else
    {
        tcam_debug("Gain returned value: %u", value);
    }
    return value;
}


bool AFU420Device::set_gain (int64_t gain)
{
    uint16_t value = gain;

    int ret = control_write(BASIC_PC_TO_USB_GAIN, value);

    if (ret < 0)
    {
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
        //tcam_error("Unable to read property '%s'. LibUsb returned %d",
        //         desc.property->get_name().c_str(), ret);

        tcam_error("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        return false;
    }
    else
    {
        tcam_debug("Gain value: %u written", value);
    }
    return true;
}
