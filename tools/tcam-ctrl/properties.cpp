
#include "properties.h"

#include "utils.h"

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
                          << "value=";
                if (s.getValue())
                {
                    std::cout << "true";
                }
                else
                {
                    std::cout << "false";
                }
                std::cout << std::endl;
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

    std::vector<std::string> prop_vec = tcam::split_string(new_prop, "=");

    if (prop_vec.size() != 2)
    {
        std::cout << "Given property string is faulty!" << std::endl;
        return false;
    }

    std::string name = prop_vec.at(0);
    std::string value = prop_vec.at(1);

    auto properties = g.getAvailableProperties();

    for (Property& p : properties)
    {
        if (p.getName().compare(name) == 0)
        {
            std::cout << "Found property!" << std::endl;
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
                    PropertySwitch& prop_s = (PropertySwitch&) p;
                    if (value == "true" || value == "TRUE" || value == "1")
                    {
                        std::cout << "Setting " << name << " to TRUE"<< std::endl;

                        return prop_s.setValue(true);
                    }
                    else if (value == "false" || value == "FALSE" || value == "0")
                    {
                        std::cout << "Setting " << name << " to FALSE"<< std::endl;
                        return prop_s.setValue(false);
                    }
                    else
                    {
                        std::cout << "Could not interpret \"" << value << "\" as boolean." << std::endl;
                        return false;
                    }

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
