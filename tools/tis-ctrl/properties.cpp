
#include "properties.h"

#include <iostream>
#include <iomanip>

void print_properties (const std::vector<Property>& properties)
{
    
    for (const auto& p : properties)
    {
        std::cout << std::left;
        switch (p.getType())
        {
            case PROPERTY_TYPE_INTEGER:
            {
                PropertyInteger& i = (PropertyInteger&) p;
                std::cout << std::setw(20) << i.getName()
                          << std::setw(10) << " (int)"<< std::right
                          << "min=" << std::setw(5)<< i.getMin()
                          << " max=" << std::setw(8) << i.getMax()
                          << " step="<< std::setw(2)  << i.getStep()
                          << " default=" << std::setw(5) << i.getDefault()
                          << " value=" << i.getValue()
                          << std::endl;
                break;
            }
            case PROPERTY_TYPE_DOUBLE:
            {
                break;
            }
            case PROPERTY_TYPE_STRING:
            case PROPERTY_TYPE_STRING_TABLE:
            {

            }
            case PROPERTY_TYPE_BOOLEAN:
            {
                PropertySwitch& s = (PropertySwitch&) p;

                std::cout << std::setw(20) << s.getName()
                          << std::setw(10) << "(bool)"
                          << std::setw(31) << " " 
                          << "default="<< std::setw(5) << s.getDefault()
                          << "value=" << s.getValue()
                          << std::endl;
                break;
            }
            case PROPERTY_TYPE_BUTTON:
            {
                std::cout << std::setw(20) << p.getName()
                          << std::setw(10) << "(button)"
                          << std::endl;
                break;
            }
            case PROPERTY_TYPE_BITMASK:
            case PROPERTY_TYPE_UNKNOWN:
            default:
            {
                std::cerr << "Unknown property type " << p.getName() << std::endl;
            }
        }

    }

}


bool set_property (Grabber& g, const std::string& new_prop)
{

    if (!g.isDeviceOpen())
    {
        std::cout << "An open device is required for setting properties!"<< std::endl;
        return false;
    }

    std::string name;
    std::string value;
    
    auto properties = g.getAvailableProperties();

    for (Property& p : properties)
    {
        if (p.getName().compare(name) == 0)
        {
            switch(p.getType())
            {
                case PROPERTY_TYPE_DOUBLE:
                {
                    PropertyDouble& prop_d = (PropertyDouble&) p;

                    return prop_d.setValue(std::stod(value));
                }
                case PROPERTY_TYPE_STRING:
                {
                    PropertyString& prop_s = (PropertyString&) p;
                    return prop_s.setValue(value);
                }
                case PROPERTY_TYPE_STRING_TABLE:
                {
                    PropertyStringMap& prop_m = (PropertyStringMap&) p;

                    //prop_m.getValues();

                    return prop_m.setValue(value);
                }
                case PROPERTY_TYPE_BUTTON:
                {
                    PropertyButton& button = (PropertyButton&) p;

                    return button.activate();
                }
                case PROPERTY_TYPE_BITMASK:
                case PROPERTY_TYPE_INTEGER:
                {
                    PropertyInteger& prop_i = (PropertyInteger&) p;

                    return prop_i.setValue(std::stoi(value));
                }
                case PROPERTY_TYPE_BOOLEAN:
                {
                    
                }
                case PROPERTY_TYPE_UNKNOWN:
                default:
                {
                    std::cout << "Found property but am unable to determine correct property type."<< std::endl;
                    std::cout << "Aborting..." << std::endl;
                    return false;
                }
            }
        }
    }

    std::cout << "No property with name ' " << name <<"'"<< std::endl;
    
    return false;
}










