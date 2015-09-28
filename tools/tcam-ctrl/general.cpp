

#include "general.h"

#include "tcam.h"

#include <unistd.h>

#include <memory>
#include <iostream>

using namespace tcam;


bool save_device_list (const std::string& filename)
{
    if (filename.empty())
    {
        return false;
    }

    std::vector<DeviceInfo> device_list = get_device_list();

    export_device_list(device_list, filename);

    return false;
}


bool save_device_settings (const std::string& serial, const std::string& filename)
{
    auto dev = open_device(serial);

    if (dev == nullptr)
    {
        std::cout << "WHAT" << std::endl;
    }

    dev->save_configuration(filename);

    return false;
}


bool load_device_settings (const std::string& serial, const std::string& filename)
{
    auto dev = open_device(serial);

    if (dev == nullptr)
    {
        std::cout << "WHAT" << std::endl;
    }

    dev->load_configuration(filename);

    return false;
}
