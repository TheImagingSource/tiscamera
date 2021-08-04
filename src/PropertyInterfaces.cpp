

#include "PropertyInterfaces.h"

#include <algorithm>

std::shared_ptr<tcam::property::IPropertyBase> tcam::property::find_property(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& properties,
    const std::string_view& name)
{
    auto iter = std::find_if(properties.begin(),
                             properties.end(),
                             [&name](const std::shared_ptr<tcam::property::IPropertyBase>& ptr) {
                                 if (ptr->get_name() == name)
                                 {
                                     return true;
                                 }
                                 return false;
                             });
    if (iter == properties.end())
    {
        return nullptr;
    }
    return (*iter);
}
