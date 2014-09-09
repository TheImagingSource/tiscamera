

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

PropertyHandler::~PropertyHandler ()
{}



bool PropertyHandler::isAvailable (const Property&)
{
    return true;
}


bool PropertyHandler::setProperty (const Property&)
{
    return false;
}


bool PropertyHandler::getProperty (Property&)
{
    return false;
}



AutoPassFilter::AutoPassFilter ()
    : valid(false),
      current_status(PIPELINE_UNDEFINED),
      exposure_max(0)
{
    description.name = "AutoPass";
    description.type = FILTER_TYPE_INTERPRET;

    state = {};
}


AutoPassFilter::~AutoPassFilter ()
{}


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
        params.exposure.min = handler->property_exposure.lock()->getMin();
        params.exposure.max = exposure_max;
        params.exposure.def = handler->property_exposure.lock()->getDefault();
        params.exposure.val = handler->property_exposure.lock()->getValue();
 
        params.exposure.do_auto = true;
        params.exposure.flags = 0;
        params.exposure.granularity = 1;
    }

    if (handler->prop_auto_gain != nullptr)
    {
        params.gain.min = handler->property_gain.lock()->getMin();
        params.gain.max = handler->property_gain.lock()->getMax();
        params.gain.def = handler->property_gain.lock()->getDefault();
        params.gain.val = handler->property_gain.lock()->getValue();
        params.gain.do_auto = true;
        params.gain.flags = 0;
        params.gain.steps_to_double_brightness = 1;
    }

    if (handler->prop_auto_iris != nullptr)
    {
        params.iris.min = handler->property_iris.lock()->getMin();
        params.iris.max = handler->property_iris.lock()->getMax();
        params.iris.def = handler->property_iris.lock()->getDefault();
        params.iris.val = handler->property_iris.lock()->getValue();
        // params.iris.do_auto = handler->prop_auto_iris->getValue();
        params.iris.do_auto = false;
        params.iris.flags = 0;
        params.iris.camera_fps = 30.0;   // TODO: automate
        params.iris.is_pwm_iris = false; // TODO: automate
    }

    if (handler->prop_auto_wb != nullptr)
    {
        params.wb.r = 64;
        params.wb.g = 64;
        params.wb.b = 64;
        params.wb.auto_enabled = true;
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

    
void AutoPassFilter::update_state ()
{

}


bool AutoPassFilter::apply (std::shared_ptr<MemoryBuffer> buf)
{

    update_params();
    update_state();
    
    img::img_descriptor img = to_img_desc(*buf);

    tis_log(TIS_LOG_INFO, "Received image descriptor with type %d", img.type);

    
    auto_alg::auto_pass_results res = auto_alg::auto_pass(img, params, state);

    if (params.exposure.do_auto == true)
    {
        tis_log(TIS_LOG_INFO, "Setting exposure to  %d", res.exposure);
        set_exposure(res.exposure);
    }
    if (params.gain.do_auto == true)
    {
        tis_log(TIS_LOG_INFO, "Setting gain to  %d", res.gain);
        set_gain(res.gain);
    }
    if (params.iris.do_auto == true)
    {
        tis_log(TIS_LOG_INFO, "Setting iris to  %d", res.iris);
        set_iris(res.iris);
    }

    by8_transform::apply_wb_to_bayer_img(img, res.wb_r, res.wb_g, res.wb_b, res.wb_g, 0);
    
    return true;
}


bool AutoPassFilter::setStatus (const PIPELINE_STATUS& s)
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
        tis_log(TIS_LOG_ERROR, "Unable to find exposure property. Auto Exposure will be disabled.");
        // handler->property_exposure = nullptr;
        return;
    }
    else
    {
        handler->property_exposure = std::static_pointer_cast<PropertyInteger>(*exp);

        // property_exposure =*exp;
        // create auto_exposure property
        camera_property prop = {};

        strncpy(prop.name, "Exposure Auto", sizeof(prop.name));
        prop.type = PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags & (1 << PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_exposure = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);
    }

    
    s = "Gain";
    auto gain = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (gain == dev_properties.end())
    {
        tis_log(TIS_LOG_ERROR, "Unable to find exposure property. Module will be disabled.");
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
        prop.flags & (1 << PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_gain = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);
    }
    
    s = "Iris";
    auto iris = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (iris == dev_properties.end())
    {
        tis_log(TIS_LOG_ERROR, "Unable to find iris property.");
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
        prop.flags & (1 << PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_iris = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);

    }

    
    camera_property prop = {};

    strncpy(prop.name, "Whitebalance Auto", sizeof(prop.name));
    prop.type = PROPERTY_TYPE_BOOLEAN;
    prop.value.b.value = true;
    prop.flags & (1 << PROPERTY_FLAG_EXTERNAL);

    handler->prop_auto_wb = std::make_shared<PropertySwitch>(handler, prop, Property::BOOLEAN);

    params.wb.r = 60;
    params.wb.g = 60;
    params.wb.b = 60;
    params.wb.auto_enabled = true;
    params.wb.one_push_enabled = false;
    params.wb.is_software_applied_wb = true;
    params.wb.temperature_mode = false;

    update_params();
    
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
