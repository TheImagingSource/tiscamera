

#include "Properties.h"
#include "tis_logging.h"
#include "tis_utils.h" // tis_xioctl

#include <algorithm> // find_if
#include <cstring>

using namespace tis_imaging;




PropertyString::PropertyString (std::shared_ptr<PropertyImpl> prop_impl,
                                const camera_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyString::~PropertyString ()
{}


std::string PropertyString::getDefault () const
{
    return prop.value.s.default_value;
}


bool PropertyString::setValue (const std::string& new_value)
{
    if (isReadOnly())
    {
        return false;
    }

    if (new_value.size() > sizeof(this->prop.value.s.value))
        return false;

    memcpy(this->prop.value.s.value, new_value.c_str(), sizeof(this->prop.value.s.value));

    notifyImpl();

    return true;
}


std::string PropertyString::getValue () const
{
    return prop.value.s.value;
}






PropertyStringMap::PropertyStringMap (std::shared_ptr<PropertyImpl> prop_impl,
                                      const camera_property& prop,
                                      const std::map<std::string, int>& values,
                                      VALUE_TYPE t)
    : Property(prop, values, t)
{
    impl = prop_impl;
}


PropertyStringMap::~PropertyStringMap ()
{}


std::vector<std::string> PropertyStringMap::getValues () const
{
    std::vector<std::string> vec;

    for (auto m : string_map)
    {
        vec.push_back(std::get<0>(m));
    }

    return vec;
}


std::string PropertyStringMap::getDefault () const
{
    return "";
}


bool PropertyStringMap::setValue (const std::string& new_value)
{
    if (isReadOnly())
    {
        return false;
    }

    auto element = string_map.find(new_value);

    if (element == string_map.end())
    {
        return false;
    }

    notifyImpl();

    return false;
}


std::string PropertyStringMap::getValue () const
{
    return prop.value.s.value;
}


std::map<std::string, int> PropertyStringMap::getMapping () const
{
    return string_map;
}





PropertySwitch::PropertySwitch (std::shared_ptr<PropertyImpl> prop_impl,
                                const camera_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertySwitch::~PropertySwitch ()
{}


bool PropertySwitch::getDefault () const
{
    return prop.value.b.default_value;
}


bool PropertySwitch::setValue (bool value)
{
    if (isReadOnly())
    {
        return false;
    }

    prop.value.b.value = value;
    notifyImpl();

    return true;
}


bool PropertySwitch::getValue () const
{
    return prop.value.b.value;
}






PropertyInteger::PropertyInteger (std::shared_ptr<PropertyImpl> prop_impl,
                                  const camera_property& prop,
                                  VALUE_TYPE t)
    : Property (prop, t)
{
    impl = prop_impl;
}


PropertyInteger::~PropertyInteger ()
{}


int64_t PropertyInteger::getDefault () const
{
    return prop.value.i.default_value;
}


int64_t PropertyInteger::getMin () const
{
    return this->prop.value.i.min;
}


int64_t PropertyInteger::getMax () const
{
    return this->prop.value.i.max;
}


int64_t PropertyInteger::getStep () const
{
  return this->prop.value.i.step;
}


int64_t PropertyInteger::getValue () const
{
    return this->prop.value.i.value;
}


bool PropertyInteger::setValue (int64_t new_value)
{
    // if (isReadOnly())
    // return false;

    tis_value_int& i = this->prop.value.i;

    // if (i.min > _value || i.max < _value)
    // return false;

    i.value = new_value;

    notifyImpl();

    return true;
}






PropertyDouble::PropertyDouble (std::shared_ptr<PropertyImpl> prop_impl,
                                const camera_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyDouble::~PropertyDouble ()
{}


double PropertyDouble::getDefault () const
{
    return prop.value.d.default_value;
}


double PropertyDouble::getMin () const
{
    return this->prop.value.d.min;
}


double PropertyDouble::getMax () const
{
    return this->prop.value.d.max;
}


double PropertyDouble::getValue () const
{
    return this->prop.value.d.value;
}


bool PropertyDouble::setValue (double new_value)
{
    if (isReadOnly())
    {
        return false;
    }
    
    tis_value_double& d = this->prop.value.d;

    if (d.min > new_value || d.max < new_value)
    {
        return false;
    }

    d.value = new_value;

    notifyImpl();

    return false;
}





PropertyButton::PropertyButton (std::shared_ptr<PropertyImpl> prop_impl,
                                const camera_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyButton::~PropertyButton ()
{}


bool PropertyButton::activate ()
{
    if (isReadOnly())
        return false;

    notifyImpl();

    return true;
}
