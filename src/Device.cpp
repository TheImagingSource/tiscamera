


#include "Device.h"
#include "user_properties.h"

#include <algorithm>

using namespace tis_imaging;

Device::Device (const CaptureDevice& _device)
    : actual_device(openCaptureInterface(_device))
{

    for (auto& p : actual_device->create_properties(actual_device))
    {
        device_properties.push_back(p);
    }

    create_user_properties();

}

    
Device::Device (const Device&)
{

}


Device::~Device ()
{
    for (auto& p : device_properties)
    {
        p.reset();
    }
}


CaptureDevice Device::getDeviceDescription () const
{
    return actual_device->getDeviceDescription();
}


std::vector<Property> Device::getProperties () const
{
    std::vector<Property> props;

    for ( const auto& p : user_properties )
    {
        props.push_back(*p);
    }
    
    return props;
}


std::vector<std::shared_ptr<Property> > Device::create_properties(std::shared_ptr<PropertyImpl>)
{}


bool Device::isAvailable (const Property&)
{
    
}


bool Device::setProperty (const Property& _property)
{
    return actual_device->setProperty(_property);
}

    
bool Device::getProperty (Property& _property)
{
    return actual_device->getProperty(_property);
}

    
bool Device::setVideoFormat (const VideoFormat& _format)
{
    return actual_device->setVideoFormat(_format);
}

    
VideoFormat Device::getActiveVideoFormat () const
{
    return actual_device->getActiveVideoFormat();
}


std::vector<VideoFormatDescription> Device::getAvailableVideoFormats () const
{
    return actual_device->getAvailableVideoFormats();
}


void Device::create_pass_through_property (std::shared_ptr<Property> dev_prop)
{

    PROPERTY_TYPE type = dev_prop->getType();
        
    switch (type)
    {
        case PROPERTY_TYPE_STRING:
        {
            user_properties.push_back(std::make_shared<PropertyString>(dev_prop, dev_prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_STRING_TABLE:
        {
            user_properties.push_back(std::make_shared<PropertyStringMap>(dev_prop, dev_prop->getStruct(), (std::dynamic_pointer_cast<PropertyStringMap>(dev_prop)->getMapping() )));
            break;

        }
        case PROPERTY_TYPE_INTEGER:
        {
            user_properties.push_back(std::make_shared<PropertyInteger>(dev_prop, dev_prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_DOUBLE:
        {
            user_properties.push_back(std::make_shared<PropertyDouble>(dev_prop, dev_prop->getStruct()));
            break;
        }                
        case PROPERTY_TYPE_BUTTON:
        {
            user_properties.push_back(std::make_shared<PropertyButton>(dev_prop, dev_prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_BOOLEAN:
        {
            user_properties.push_back(std::make_shared<PropertySwitch>(dev_prop, dev_prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_BITMASK:
        {
            // user_properties.push_back(std::make_shared<PropertyBitmask>((PropertyBitmask&)p));
            break;
        }
        default:
        {
            exit(1);
        }
    }
}


void Device::create_new_user_property (std::shared_ptr<Property> _prop, const control_reference& _ref)
{
    PROPERTY_TYPE type = _ref.type_to_use;
        
    switch (type)
    {
        case PROPERTY_TYPE_STRING:
        {
            user_properties.push_back(std::make_shared<PropertyString>(_prop, _prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_STRING_TABLE:
        {
            user_properties.push_back(std::make_shared<PropertyStringMap>(_prop, _prop->getStruct(), (std::dynamic_pointer_cast<PropertyStringMap>(_prop)->getMapping() )));
            break;

        }
        case PROPERTY_TYPE_INTEGER:
        {
            user_properties.push_back(std::make_shared<PropertyInteger>(_prop, _prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_DOUBLE:
        {
            user_properties.push_back(std::make_shared<PropertyDouble>(_prop, _prop->getStruct()));
            break;
        }                
        case PROPERTY_TYPE_BUTTON:
        {
            user_properties.push_back(std::make_shared<PropertyButton>(_prop, _prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_BOOLEAN:
        {
            user_properties.push_back(std::make_shared<PropertySwitch>(_prop, _prop->getStruct()));
            break;
        }
        case PROPERTY_TYPE_BITMASK:
        {
            // user_properties.push_back(std::make_shared<PropertyBitmask>((PropertyBitmask&)p));
            break;
        }
        default:
        {
            exit(1);
        }
    }
}


bool Device::create_user_properties ()
{
    if (device_properties.empty())
    {
        return false;
    }


    // auto match_string = [] 
    
    std::string property_name;
    TIS_DEVICE_TYPE type = actual_device->getDeviceDescription().getDeviceType();
    auto find_user_property = [&property_name, &type] (const control_reference& ref)
        {
            if (type == TIS_DEVICE_TYPE_USB)
            {
                return (std::find(ref.v4l2_name.begin(),ref.v4l2_name.end(), property_name) != ref.v4l2_name.end());
            }
            else if (type == TIS_DEVICE_TYPE_GIGE)
            {
                return (std::find(ref.genicam_name.begin(), ref.genicam_name.end(), property_name) != ref.genicam_name.end()); 
            }
            else
            {
                return false;
            }
        };

    // auto f = std::find_if(std::begin(ctrl_reference_table), std::end(ctrl_reference_table), find_user_property);


    
    for (auto& p : device_properties)
    {
        // find appropriate user property definition
        property_name = p.lock()->getName();

        auto f = std::find_if(std::begin(ctrl_reference_table), std::end(ctrl_reference_table), find_user_property);

        if (f == std::end(ctrl_reference_table))
        {        
            // if non found simply pass through
            create_pass_through_property(p.lock());
            continue;
        }
        
        // else create property according to definition

        create_new_user_property (p.lock(), *f);
    }

    

    return true;;
}


