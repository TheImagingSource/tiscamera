

#include "Properties.h"
#include "tis_logging.h"
#include "tis_utils.h" // tis_xioctl

#include <algorithm> // find_if
#include <cstring>

using namespace tis_imaging;




PropertyString::PropertyString (std::shared_ptr<PropertyImpl> _impl, const camera_property& _property)
    : Property(_property)
{
    impl = _impl;
}


PropertyString::~PropertyString ()
{}


std::string PropertyString::getDefault () const
{
    return prop.value.s.default_value;
}


bool PropertyString::setValue (const std::string& _value)
{
    if (isReadOnly())
    {
        return false;
    }

    if (_value.size() > sizeof(this->prop.value.s.value))
        return false;

    memcpy(this->prop.value.s.value, _value.c_str(), _value.size());

    notifyImpl();

    return true;
}


std::string PropertyString::getValue () const
{
    return prop.value.s.value;
}






PropertyStringMap::PropertyStringMap (std::shared_ptr<PropertyImpl> _impl,
                                      const camera_property& _property,
                                      const std::map<std::string, int>& _values)
    : Property(_property), string_map(_values)
{
    impl = _impl;
}


PropertyStringMap::~PropertyStringMap ()
{}


std::vector<std::string> PropertyStringMap::getValues () const
{

}


std::string PropertyStringMap::getDefault () const
{
    return "";
}


bool PropertyStringMap::setValue (const std::string& _value)
{
    if (isReadOnly())
    {
        return false;
    }

    auto element = string_map.find(_value);

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





PropertySwitch::PropertySwitch (std::shared_ptr<PropertyImpl> _impl,
                                const camera_property& _property)
    : Property(_property)
{
    impl = _impl ;
}


PropertySwitch::~PropertySwitch ()
{}


bool PropertySwitch::getDefault () const
{
    return prop.value.i.default_value;
}


bool PropertySwitch::setValue (const bool&)
{
    if (isReadOnly())
    {
        return false;
    }

    notifyImpl();

    return true;
}


bool PropertySwitch::getValue () const
{
    return prop.value.i.value;
}






PropertyInteger::PropertyInteger (std::shared_ptr<PropertyImpl> _impl,
                                  const camera_property& _property)
    : Property (_property)
{
    impl = _impl;
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


int64_t PropertyInteger::getValue () const
{
    return this->prop.value.i.value;
}


bool PropertyInteger::setValue (const int64_t& _value)
{
    // if (isReadOnly())
    // return false;

    tis_value_int& i = this->prop.value.i;

    // if (i.min > _value || i.max < _value)
    // return false;

    i.value = _value;

    notifyImpl();

    return true;
}






PropertyDouble::PropertyDouble (std::shared_ptr<PropertyImpl> _impl,
                                const camera_property& _property)
    : Property(_property)
{
    impl = _impl;
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


bool PropertyDouble::setValue (const double& _value)
{
    if (isReadOnly())
    {
        return false;
    }
    
    tis_value_double& d = this->prop.value.d;

    if (d.min > _value || d.max < _value)
    {
        return false;
    }

    d.value = _value;

    notifyImpl();

    return false;
}





PropertyButton::PropertyButton (std::shared_ptr<PropertyImpl> _impl,
                                const camera_property& _property)
    : Property(_property)
{
    impl = _impl ;
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
