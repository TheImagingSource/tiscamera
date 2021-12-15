

#include "PropertyInterfaces.h"

#include <algorithm>

std::shared_ptr<tcam::property::IPropertyBase> tcam::property::find_property(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& properties,
    std::string_view name)
{
    auto iter = std::find_if(properties.begin(),
                             properties.end(),
                             [&name](const auto& ptr) { return ptr->get_name() == name; });
    if (iter == properties.end())
    {
        return nullptr;
    }
    return (*iter);
}
