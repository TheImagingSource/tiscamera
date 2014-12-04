

#include "AutoPassFilter.h"
#include <tcam.h>

#include "logging.h"
#include "utils.h"
#include "standard_properties.h"

#include <algorithm>

using namespace tcam;


AutoPassPropertyHandler::AutoPassPropertyHandler (AutoPassFilter* f)
    : filter(f),
      prop_auto_exposure(nullptr),
      prop_auto_gain(nullptr),
      prop_auto_iris(nullptr),
      prop_wb(nullptr),
      prop_auto_wb(nullptr),
      prop_wb_r(nullptr),
      prop_wb_g(nullptr),
      prop_wb_b(nullptr)
{}


bool AutoPassPropertyHandler::setProperty (const Property& prop)
{
    if (prop.getID() == TCAM_PROPERTY_EXPOSURE_AUTO)
    {
        prop_auto_exposure->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_GAIN_AUTO)
    {
        prop_auto_gain->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_IRIS_AUTO)
    {
        prop_auto_iris->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB)
    {
        prop_wb->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_AUTO)
    {
        prop_auto_wb->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_RED)
    {
        prop_wb_r->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_GREEN)
    {
        prop_wb_g->setStruct(prop.getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_BLUE)
    {
        prop_wb_b->setStruct(prop.getStruct());
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "Property not supported by impl: %s(%d)", prop.getName().c_str(), prop.getID());
        return false;
    }
    tcam_log(TCAM_LOG_DEBUG, "Received change from %s", prop.getName().c_str());
    return true;
}


bool AutoPassPropertyHandler::getProperty (Property& prop)
{
    if (prop.getID() == TCAM_PROPERTY_EXPOSURE_AUTO)
    {
        prop.setStruct(prop_auto_exposure->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_GAIN_AUTO)
    {
        prop.setStruct(prop_auto_gain->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_IRIS_AUTO)
    {
        prop.setStruct(prop_auto_iris->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB)
    {
        prop.setStruct(prop_wb->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_AUTO)
    {
        prop.setStruct(prop_auto_wb->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_RED)
    {
        prop.setStruct(prop_wb_r->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_GREEN)
    {
        prop.setStruct(prop_wb_g->getStruct());
    }
    else if (prop.getID() == TCAM_PROPERTY_WB_BLUE)
    {
        prop.setStruct(prop_wb_b->getStruct());
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "Property not supported by impl");
        return false;
    }
    tcam_log(TCAM_LOG_DEBUG, "Updated value for %s", prop.getName().c_str());
    return true;
}


AutoPassFilter::AutoPassFilter ()
    : valid(false),
      skipped_buffer(0),
      current_status(TCAM_PIPELINE_UNDEFINED),
      wb_r(default_color_value),
      wb_g(default_color_value),
      wb_b(default_color_value),
      exposure_max(0),
      state(nullptr)
{
    description.name = "AutoPass";
    description.type = FILTER_TYPE_INTERPRET;
    description.input_fourcc = {0};
    description.output_fourcc = {0};

    handler = std::make_shared<AutoPassPropertyHandler>(this);
}


void AutoPassFilter::reset ()
{
    params = {};
    wb_r = default_color_value;
    wb_g = default_color_value;
    wb_b = default_color_value;

    handler = std::make_shared<AutoPassPropertyHandler>(this);
}


struct FilterDescription AutoPassFilter::getDescription () const
{
    return description;
}


bool AutoPassFilter::transform (MemoryBuffer& /* in */, MemoryBuffer& /* out */ )
{
    return false;
}


void AutoPassFilter::update_params ()
{
    if (handler->prop_auto_exposure != nullptr)
    {
        auto exp = handler->property_exposure.lock();
        params.exposure.value.min = exp->getMin();
        params.exposure.value.max = exposure_max;

        params.exposure.value.default_value = exp->getDefault();
        params.exposure.value.value = exp->getValue();
        params.exposure.do_auto = handler->prop_auto_exposure->getValue();
    }

    if (handler->prop_auto_gain != nullptr)
    {
        auto gain = handler->property_gain.lock();
        params.gain.value.min = gain->getMin();
        params.gain.value.max = gain->getMax();
        params.gain.value.default_value = gain->getDefault();
        params.gain.value.value = handler->property_gain.lock()->getValue();
        params.gain.do_auto = handler->prop_auto_gain->getValue();
        params.gain.steps_to_double_brightness = 1;
    }

    if (handler->prop_auto_iris != nullptr)
    {
        auto iris = handler->property_iris.lock();
        params.iris.value.min = iris->getMin();
        params.iris.value.max = iris->getMax();
        params.iris.value.default_value = iris->getDefault();
        params.iris.value.value = iris->getValue();
        params.iris.do_auto = handler->prop_auto_iris->getValue();
    }

    if (handler->prop_auto_wb != nullptr)
    {
        // use default values if disabled
        if (((PropertyBoolean&)*(handler->prop_auto_wb)).getValue())
        {
            params.wb.r = wb_r;
            params.wb.g = wb_g;
            params.wb.b = wb_b;
        }
        else
        {
            params.wb.r = ((PropertyBoolean&)*(handler->prop_wb_r)).getValue();
            params.wb.g = ((PropertyBoolean&)*(handler->prop_wb_g)).getValue();
            params.wb.b = ((PropertyBoolean&)*(handler->prop_wb_b)).getValue();
        }

        params.wb.auto_enabled = handler->prop_auto_wb->getValue();
        params.wb.one_push_enabled = false;
        params.wb.is_software_applied_wb = true;
        params.wb.temperature_mode = false;

    }

    params.exposure_reference.value.min = 1;
    params.exposure_reference.value.max = 255;
    params.exposure_reference.value.default_value = 128;
    params.exposure_reference.value.value = 128;
    params.exposure_reference.do_auto = params.exposure.do_auto;

    params.enable_auto_ref = false;
}


bool AutoPassFilter::apply (std::shared_ptr<MemoryBuffer> buf)
{
    auto img = buf->getImageBuffer();

    if (skipped_buffer < 3)
    {
        skipped_buffer++;
    }
    else
    {
        skipped_buffer = 0;

        update_params();

        auto res = auto_pass(&img, &params, state);

        if (handler->prop_auto_wb->getValue())
        {
            wb_r = res.wb_r;
            wb_g = res.wb_g;
            wb_b = res.wb_b;
        }
        else
        {
            res.wb_r = handler->prop_wb_r->getValue();
            res.wb_g = handler->prop_wb_g->getValue();
            res.wb_b = handler->prop_wb_b->getValue();
            wb_r = res.wb_r;
            wb_g = res.wb_g;
            wb_b = res.wb_b;
        }

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

    if (handler->prop_wb->getValue())
        apply_wb_to_bayer_img(&img, wb_r, wb_g, wb_b, wb_g);

    return true;
}


bool AutoPassFilter::setStatus (TCAM_PIPELINE_STATUS s)
{
    if (current_status == s)
    {
        return true;
    }

    current_status = s;

    if (current_status == TCAM_PIPELINE_PLAYING)
    {
        if (state == nullptr)
        {
            context.reserve(get_auto_pass_context_size());
            state = create_auto_pass_context(context.data(),&init_params);

            if (state == nullptr)
                tcam_log(TCAM_LOG_ERROR, "Unable to determine initial state for algorithm library");
        }

        reinit_auto_pass_context(state);
    }
    else if (current_status == TCAM_PIPELINE_STOPPED)
    {
        if (state != nullptr)
            destroy_auto_pass_context(state);
    }

    return true;
}


TCAM_PIPELINE_STATUS AutoPassFilter::getStatus () const
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

    exposure_max = calculate_exposure_max();

    tcam_log(TCAM_LOG_INFO, "Exposure maximum will be %d", exposure_max);

    return true;
}


bool AutoPassFilter::setVideoFormat (const VideoFormat& in, const VideoFormat& out)
{
    if (in != out)
    {
        return false;
    }

    input_format = in;

    exposure_max = calculate_exposure_max();

    tcam_log(TCAM_LOG_INFO, "Exposure maximum will be %d", exposure_max);

    return true;
}


void AutoPassFilter::setDeviceProperties (std::vector<std::shared_ptr<Property>> dev_properties)
{
    reset();

    TCAM_PROPERTY_ID id;
    auto f = [&id] (std::shared_ptr<Property> p)
        {
            return id ==p->getID();
        };


    id = TCAM_PROPERTY_EXPOSURE;
    auto exp = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (exp == dev_properties.end())
    {
        tcam_log(TCAM_LOG_INFO, "Unable to find exposure property. Auto Exposure will be disabled.");
        return;
    }
    else
    {
        handler->property_exposure = std::static_pointer_cast<PropertyInteger>(*exp);

        tcam_camera_property prop = {};
        prop.id = TCAM_PROPERTY_EXPOSURE_AUTO;
        strncpy(prop.name, "Exposure Auto", sizeof(prop.name));
        prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags = set_bit(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_exposure = std::make_shared<PropertyBoolean>(handler, prop, Property::BOOLEAN);
    }


    id = TCAM_PROPERTY_GAIN;
    auto gain = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (gain == dev_properties.end())
    {
        tcam_log(TCAM_LOG_INFO, "Unable to find exposure property. Module will be disabled.");
        return;
    }
    else
    {
        handler->property_gain = std::static_pointer_cast<PropertyInteger>(*gain);

        // property_gain = *gain;
        // create auto_gain property
        tcam_camera_property prop = {};
        prop.id = TCAM_PROPERTY_GAIN_AUTO;
        strncpy(prop.name, "Gain Auto", sizeof(prop.name));
        prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.value.b.default_value = true;
        prop.flags = set_bit(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_gain = std::make_shared<PropertyBoolean>(handler, prop, Property::BOOLEAN);
    }

    id = TCAM_PROPERTY_IRIS;
    auto iris = std::find_if(dev_properties.begin(), dev_properties.end(), f);

    if (iris == dev_properties.end())
    {
        tcam_log(TCAM_LOG_INFO, "Unable to find iris property.");
        // return;
        //property_iris = nullptr;
    }
    else
    {
        tcam_log(TCAM_LOG_INFO, "Found iris property.");

        handler->property_iris = std::static_pointer_cast<PropertyInteger>(*iris);

        // property_iris = *iris;
        // create auto_iris property

        tcam_camera_property prop = {};
        prop.id = TCAM_PROPERTY_IRIS_AUTO;
        strncpy(prop.name, "Iris Auto", sizeof(prop.name));
        prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;
        prop.value.b.value = true;
        prop.flags = set_bit(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

        handler->prop_auto_iris = std::make_shared<PropertyBoolean>(handler, prop, Property::BOOLEAN);

    }

    // TODO check for device whitebalance

    tcam_camera_property prop = {};
    prop.id = TCAM_PROPERTY_WB;
    strncpy(prop.name, "Whitebalance", sizeof(prop.name));
    prop.type = TCAM_PROPERTY_TYPE_BOOLEAN;
    prop.value.b.value = true;
    prop.value.b.default_value = true;
    prop.flags = set_bit(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

    handler->prop_wb = std::make_shared<PropertyBoolean>(handler, prop, Property::BOOLEAN);



    tcam_camera_property prop_auto = create_empty_property(TCAM_PROPERTY_WB_AUTO);
    prop_auto.value.b.value = true;
    prop_auto.value.b.default_value = true;
    prop_auto.flags = set_bit(prop_auto.flags, TCAM_PROPERTY_FLAG_EXTERNAL);

    handler->prop_auto_wb = std::make_shared<PropertyBoolean>(handler, prop_auto, Property::BOOLEAN);


    tcam_camera_property prop_wbr = create_empty_property(TCAM_PROPERTY_WB_RED);
    prop_wbr.value.i.min = 0;
    prop_wbr.value.i.max = 255;
    prop_wbr.value.i.value = default_color_value;
    prop_wbr.value.i.default_value = default_color_value;
    prop_wbr.flags = set_bit(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);
    handler->prop_wb_r = std::make_shared<PropertyInteger>(handler, prop_wbr, Property::INTEGER);

    tcam_camera_property prop_wbg = create_empty_property(TCAM_PROPERTY_WB_GREEN);
    prop_wbg.value.i.min = 0;
    prop_wbg.value.i.max = 255;
    prop_wbg.value.i.value = default_color_value;
    prop_wbg.value.i.default_value = default_color_value;
    prop_wbg.flags = set_bit(prop_wbg.flags, TCAM_PROPERTY_FLAG_EXTERNAL);
    handler->prop_wb_g= std::make_shared<PropertyInteger>(handler, prop_wbg, Property::INTEGER);

    tcam_camera_property prop_wbb = create_empty_property(TCAM_PROPERTY_WB_BLUE);
    prop_wbb.value.i.min = 0;
    prop_wbb.value.i.max = 255;
    prop_wbb.value.i.value = default_color_value;
    prop_wbb.value.i.default_value = default_color_value;
    prop_wbb.flags = set_bit(prop_wbb.flags, TCAM_PROPERTY_FLAG_EXTERNAL);
    handler->prop_wb_b= std::make_shared<PropertyInteger>(handler, prop_wbb, Property::INTEGER);


    params.wb.r = default_color_value;
    params.wb.g = default_color_value;
    params.wb.b = default_color_value;
    params.wb.auto_enabled = true;
    params.wb.one_push_enabled = false;
    params.wb.is_software_applied_wb = true;
    params.wb.temperature_mode = false;

    valid = true;

    init_params.auto_apply_distance = true;
    init_params.add_software_onepush_focus = false;
    init_params.is_software_applied_wb = true;

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
    if (handler->prop_wb != nullptr)
    {
        vec.push_back(handler->prop_wb);
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


static void set_int_property (std::weak_ptr<PropertyInteger> ptr, int val)
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


unsigned int AutoPassFilter::calculate_exposure_max ()
{
    if (handler->property_exposure.expired())
    {
        return 0;
    }
    int exp_max = handler->property_exposure.lock()->getMax();
    double fps = input_format.getFramerate();

    return exp_max / 10000 * fps;
}
