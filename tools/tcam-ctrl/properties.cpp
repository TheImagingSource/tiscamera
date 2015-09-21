/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "properties.h"

#include "utils.h"

#include <iostream>
#include <iomanip>

void print_properties (const std::vector<Property*>& properties)
{
    std::cout << "Found " << properties.size() << " propert(y/ies)" << std::endl;
    for (const auto& p : properties)
    {
        std::cout << std::left;
        switch (p->get_type())
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                PropertyInteger* i = (PropertyInteger*) p;
                std::cout << std::setw(20) << i->get_name()
                          << std::setw(10) << " (int)"<< std::right
                          << "min=" << std::setw(5)<< i->get_min()
                          << " max=" << std::setw(8) << i->get_max()
                          << " step="<< std::setw(2)  << i->get_step()
                          << " default=" << std::setw(5) << i->get_default()
                          << " value=" << i->get_value()
                          << std::endl;
                break;
            }
            case TCAM_PROPERTY_TYPE_DOUBLE:
            {
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING:
            case TCAM_PROPERTY_TYPE_STRING_TABLE:
            {

            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                PropertyBoolean* s = (PropertyBoolean*) p;

                std::cout << std::setw(20) << s->get_name()
                          << std::setw(10) << "(bool)"
                          << std::setw(31) << " "
                          << "default="<< std::setw(6);
                if (s->get_default())
                {
                    std::cout << "true";
                }
                else
                {
                    std::cout << "false";
                }
                std::cout << "value=";
                if (s->get_value())
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
            case TCAM_PROPERTY_TYPE_BUTTON:
            {
                std::cout << std::setw(20) << p->get_name()
                          << std::setw(10) << "(button)"
                          << std::endl;
                break;
            }
            case TCAM_PROPERTY_TYPE_UNKNOWN:
            default:
            {
                std::cerr << "Unknown property type " << p->get_name() << std::endl;
            }
        }

    }

}


static std::vector<std::string> ctrl_split_string (const std::string& to_split,
                                                   const std::string &delim)
{
    std::vector<std::string> vec;

    size_t beg = 0;
    size_t end = 0;

    while (end != std::string::npos)
    {
        end = to_split.find_first_of(delim, beg);

        std::string s = to_split.substr(beg, end - beg);

        vec.push_back(s);

        beg = end + delim.size();
    }

    return vec;
}

bool set_property (std::shared_ptr<CaptureDevice> dev, const std::string& new_prop)
{

    std::vector<std::string> prop_vec = ctrl_split_string(new_prop, "=");

    if (prop_vec.size() != 2)
    {
        std::cout << "Given property string is faulty!" << std::endl;
        return false;
    }

    std::string name = prop_vec.at(0);
    std::string value = prop_vec.at(1);

    auto properties = dev->get_available_properties();

    for (Property* p : properties)
    {
        if (p->get_name().compare(name) == 0)
        {
            std::cout << "Found property!" << std::endl;
            switch(p->get_type())
            {
                case TCAM_PROPERTY_TYPE_DOUBLE:
                {
                    PropertyDouble* prop_d = (PropertyDouble*) p;

                    return prop_d->set_value(std::stod(value));
                }
                case TCAM_PROPERTY_TYPE_STRING:
                {
                    PropertyString* prop_s = (PropertyString*) p;
                    return prop_s->set_value(value);
                }
                case TCAM_PROPERTY_TYPE_STRING_TABLE:
                {
                    PropertyStringMap* prop_m = (PropertyStringMap*) p;

                    return prop_m->set_value(value);
                }
                case TCAM_PROPERTY_TYPE_BUTTON:
                {
                    PropertyButton* button = (PropertyButton*) p;

                    return button->activate();
                }
                case TCAM_PROPERTY_TYPE_INTEGER:
                {
                    PropertyInteger* prop_i = (PropertyInteger*) p;

                    return prop_i->set_value(std::stoi(value));
                }
                case TCAM_PROPERTY_TYPE_BOOLEAN:
                {
                    PropertyBoolean* prop_s = (PropertyBoolean*) p;
                    if (value == "true" || value == "TRUE" || value == "1")
                    {
                        std::cout << "Setting " << name << " to TRUE"<< std::endl;

                        return prop_s->set_value(true);
                    }
                    else if (value == "false" || value == "FALSE" || value == "0")
                    {
                        std::cout << "Setting " << name << " to FALSE"<< std::endl;
                        return prop_s->set_value(false);
                    }
                    else
                    {
                        std::cout << "Could not interpret \"" << value << "\" as boolean." << std::endl;
                        return false;
                    }

                }
                case TCAM_PROPERTY_TYPE_UNKNOWN:
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
