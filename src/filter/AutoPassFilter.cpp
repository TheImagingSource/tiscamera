

#include "AutoPassFilter.h"
#include <tis.h>

#include <dutils_header.h>
#include <by8/by8_apply_whitebalance.h>

#include <algorithm>

//#include <standard_properties.h>

using namespace tis_imaging;


PropertyHandler::PropertyHandler ()
    : prop_auto_exposure(nullptr),
      prop_auto_gain(nullptr),
      prop_auto_iris(nullptr),
      prop_auto_wb(nullptr),
      prop_wb_r(nullptr),
      prop_wb_g(nullptr),
      prop_wb_b(nullptr)
{}


bool PropertyHandler::isAvailable (const Property&)
{
    return true;
}


bool PropertyHandler::setProperty (const Property& prop)
{
    if (prop.getName().compare("Exposure Auto") == 0)
    {
        prop_auto_exposure->setStruct(prop.getStruct());
    }
    else if (prop.getName().compare("Gain Auto") == 0)
    {
        prop_auto_gain->setStruct(prop.getStruct());
    }
    else if (prop.getName().compare("Iris Auto") == 0)
    {
        prop_auto_iris->setStruct(prop.getStruct());

    }
    else if (prop.getName().compare("Whitebalance Auto") == 0)
    {
        prop_auto_wb->setStruct(prop.getStruct());
    }
    else
    {
        tis_log(TIS_LOG_ERROR, "Property not supported by impl");
        return false;
    }
    tis_log(TIS_LOG_DEBUG, "Received change from %s", prop.getName().c_str());
    return true;
}


bool PropertyHandler::getProperty (Property&)
{
    return false;
}


AutoPassFilter::AutoPassFilter ()
    : valid(false),
      skipped_buffer(0),
      current_status(PIPELINE_UNDEFINED),
      exposure_max(0)
{
    description.name = "AutoPass";
    description.type = FILTER_TYPE_INTERPRET;
    description.input_fourcc = {0};
    description.output_fourcc = {0};
    state = {};
}


void AutoPassFilter::reset ()
{
    params = {};
    state = {};

    handler = std::make_shared<PropertyHandler>();
}


struct FilterDescription AutoPassFilter::getDescription () const
{
    return description;
}


bool AutoPassFilter::transform (MemoryBuffer& in, MemoryBuffer& out )
{
    return false;
}


void AutoPassFilter::update_params ()
{
    if (handler->prop_auto_exposure != nullptr)
    {
        auto exp = handler->property_exposure.lock();
        params.exposure.min = exp->getMin();

        if (exposure_max > 100)
            params.exposure.max = exposure_max;
        else
            params.exposure.max = 300;

        params.exposure.def = exp->getDefault();
        params.exposure.val = exp->getValue();
        params.exposure.do_auto = handler->prop_auto_exposure->getValue();
        params.exposure.flags = 0;
        params.exposure.granularity = 1;
    }

    if (handler->prop_auto_gain != nullptr)
    {
        auto gain = handler->property_gain.lock();
        params.gain.min = gain->getMin();
        params.gain.max = gain->getMax();
        params.gain.def = gain->getDefault();
        params.gain.val = handler->property_gain.lock()->getValue();
        params.gain.do_auto = handler->prop_auto_gain->getValue();
        params.gain.flags = 0;
        params.gain.steps_to_double_brightness = 1;
    }

    if (handler->prop_auto_iris != nullptr)
    {
        auto iris = handler->property_iris.lock();
        params.iris.min = iris->getMin();
        params.iris.max = iris->getMax();
        params.iris.def = iris->getDefault();
        params.iris.val = iris->getValue();
        params.iris.do_auto = handler->prop_auto_iris->getValue();
        params.iris.flags = 0;
        params.iris.camera_fps = 30.0;   // TODO: automate
        params.iris.is_pwm_iris = false; // TODO: automate
    }

    if (handler->prop_auto_wb != nullptr)
    {
        params.wb.r = 64;
        params.wb.g = 64;
        params.wb.b = 64;
        params.wb.auto_enabled = handler->prop_auto_wb->getValue();
        params.wb.one_push_enabled = false;
        params.wb.is_software_applied_wb = true;
        params.wb.temperature_mode = true;

    }


    params.exposure_reference.min = 1;
    params.exposure_reference.max = 255;
    params.exposure_reference.def = 128;
    params.exposure_reference.val = 128;
    params.exposure_reference.do_auto = params.exposure.do_auto;
    params.exposure_reference.flags = params.exposure.flags;

    params.enable_auto_ref = false;
}


bool AutoPassFilter::apply (std::shared_ptr<MemoryBuffer> buf)
{
    img::img_descriptor img = to_img_desc(*buf);
    if (skipped_buffer < 3)
    {
        skipped_buffer++;
    }
    else
    {
        skipped_buffer = 0;

        update_params();

        auto_alg::auto_pass_results res = auto_alg::auto_pass(img, params, state);

        wb_r = res.wb_r;
        wb_g = res.wb_g;
        wb_b = res.wb_b;

        if (params.exposure.do_auto == true)
        {
            set_exposure(res.exposure);
        }
        if (params.gain.do_auto == true)
        {
            set_gain(res.gain);
        }
        if (params.iris.do_auto == true)
        {
            set_iris(res.iris);
        }
    }
    by8_transform::apply_wb_to_bayer_img(img, wb_r, wb_g, wb_b, wb_g, 0);

    return true;
}


bool AutoPassFilter::setStatus (PIPELINE_STATUS s)
{
    if (current_status == s)
    {
        return true;
    }

    current_status = s;

    return true;
}


PIPELINE_STATUS AutoPassFilter::getStatus () const
{
    return current_status;
}


void AutoPassFilter::getVideoFormat (VideoFormat& in, VideoFormat& out) const
{
    in = input_format;
    out = input_format;
}


bool AutoPassFilter::setVideoFormat (const VideoFormat& f)
{
    input_format = f;

    // exposure_max = handler->property_exposure.lock()->getValue() / 10000;//;

    if (handler->property_exposure.expired())
    {
        return true;
    }

    int exp_max = handler->property_exposure.lock()->getMax();
    double fps = input_format.getFramerate();

    exposure_max = exp_max / 10000 / fps;

    return true;
}


bool AutoPassFilter::setVideoFormat (const VideoFormat& in, const VideoFormat& out)
{
    if (in != out)
    {
        return false;
    }

    input_format = in;

    return true;
}


void AutoPassFilter::setDeviceProperties (std::vector<std::shared_ptr<Property>> dev_properties)
{
    reset();

    std::string s;
    auto f = [&s] (std::shared_ptr<Property> p)
        {
            return s.compare(p->getName()) == 0;
        };

    s = "Exposure";
    auto exp = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (exp == dev_properties.end())
    {
        tis_log(TIS_LOG_INFO, "Unable to find exposure property. Auto Exposure will be disabled.");
        return;
    }
    else
    {
        handler->property_exposure = std::static_pointer_cast<PropertyInteger>(*exp);

        camera_property prop = {};

        strncpy(prop.name, "Exposure Auto", sizeof(prop.name));
        prop.type = PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags = set_bit(prop.flags, PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_exposure = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);
    }


    s = "Gain";
    auto gain = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (gain == dev_properties.end())
    {
        tis_log(TIS_LOG_INFO, "Unable to find exposure property. Module will be disabled.");
        return;
    }
    else
    {
        handler->property_gain = std::static_pointer_cast<PropertyInteger>(*gain);

        // property_gain = *gain;
        // create auto_gain property
        camera_property prop = {};

        strncpy(prop.name, "Gain Auto", sizeof(prop.name));
        prop.type = PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags = set_bit(prop.flags, PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_gain = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);
    }

    s = "Iris";
    auto iris = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (iris == dev_properties.end())
    {
        tis_log(TIS_LOG_INFO, "Unable to find iris property.");
        // return;
        //property_iris = nullptr;
    }
    else
    {
        tis_log(TIS_LOG_INFO, "Found iris property.");

        handler->property_iris = std::static_pointer_cast<PropertyInteger>(*iris);

        // property_iris = *iris;
        // create auto_iris property

        camera_property prop = {};

        strncpy(prop.name, "Iris Auto", sizeof(prop.name));
        prop.type = PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags = set_bit(prop.flags, PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_iris = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);

    }


    camera_property prop = {};

    strncpy(prop.name, "Whitebalance Auto", sizeof(prop.name));
    prop.type = PROPERTY_TYPE_BOOLEAN;
    prop.value.b.value = true;
    prop.flags = set_bit(prop.flags, PROPERTY_FLAG_EXTERNAL);

    handler->prop_auto_wb = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);

    params.wb.r = 60;
    params.wb.g = 60;
    params.wb.b = 60;
    params.wb.auto_enabled = true;
    params.wb.one_push_enabled = false;
    params.wb.is_software_applied_wb = true;
    params.wb.temperature_mode = false;

    valid = true;
}


std::vector<std::shared_ptr<Property>> AutoPassFilter::getFilterProperties ()
{
    std::vector<std::shared_ptr<Property>> vec;

    if (handler->prop_auto_exposure != nullptr)
    {
        vec.push_back(handler->prop_auto_exposure);
    }
    if (handler->prop_auto_gain != nullptr)
    {
        vec.push_back(handler->prop_auto_gain);
    }
    if (handler->prop_auto_iris != nullptr)
    {
        vec.push_back(handler->prop_auto_iris);
    }

    if (handler->prop_auto_wb != nullptr)
    {
        vec.push_back(handler->prop_auto_wb);
    }
    if (handler->prop_wb_r != nullptr)
    {
        vec.push_back(handler->prop_wb_r);
    }
    if (handler->prop_wb_g != nullptr)
    {
        vec.push_back(handler->prop_wb_g);
    }
    if (handler->prop_wb_b != nullptr)
    {
        vec.push_back(handler->prop_wb_b);
    }

    return vec;
}


void set_int_property (std::weak_ptr<PropertyInteger> ptr, int val)
{
    if (ptr.expired())
    {
        return;
    }

    auto p = ptr.lock();

    p->setValue(val);
}


void AutoPassFilter::set_gain (int gain)
{
    set_int_property(handler->property_gain, gain);
}


void AutoPassFilter::set_exposure (int exposure)
{
    set_int_property(handler->property_exposure, exposure);
}


void AutoPassFilter::set_iris (int iris)
{
    set_int_property(handler->property_iris, iris);
}


// FilterBase* create ()
FB* create ()
{
    return (FB*)new AutoPassFilter();
}


// void close (FilterBase* filter)
void close (FB* filter)
{
    delete reinterpret_cast<AutoPassFilter*>(filter);
}
