
#include "properties.h"
#include "formats.h"
#include "multimedia.h"

#include <tis.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace tcam;


void print_help ()
{
    std::cout << "Commandline camera manipulation utility." << std::endl
              << std::endl
              << "Options:" << std::endl
              << "\t-l - list cameras" << std::endl
              << "\t-p - list properties"  << std::endl
              << "\t-f - list video formats" << std::endl
              << std::endl
              << "Examples:" << std::endl
              << std::endl
              << "Set video format:" << std::endl
              << "\ttis-ctrl -s -f \"format=RGB32,width=1920,height=1080,framerate=15.0,binning=0\" 25410069" << std::endl
              << std::endl
              << "Set property" << std::endl
              << "\ttis-ctrl -s -p \"Auto Exposure=false\""
              << std::endl
              << std::endl;
}


void print_capture_devices (const std::vector<DeviceInfo>& devices)
{
  std::cout << "Available devices:" << std::endl;
  std::cout << "Model\t\tType\tSerial" << std::endl << std::endl;
    for (const auto& d : devices)
    {
      std::cout << d.getName() << "\t" << d.getDeviceTypeAsString() << "\t" << d.getSerial() << std::endl;
    }
    std::cout << std::endl;
}


enum modes
{
    LIST_PROPERTIES = 0,
    SET_PROPERTY,
    LIST_FORMATS,
    SET_FORMAT,
    LIST_DEVICES,
    SAVE_STREAM,
    SAVE_IMAGE,
};

enum INTERACTION
{
    GET = 0,
    SET,
};

int main (int argc, char *argv[])
{

    if (argc == 1)
    {
        print_help();
        return 0;
    }

    INTERACTION way = GET;
    std::string serial;
    std::string param;
    std::string filename;
    modes do_this;

    for (int i = 0; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            print_help();
            return 0;
        }
        else if (arg == "-l" || arg == "--list")
        {
            auto index = getDeviceIndex();
            sleep(3); // let camera detection do its magic
            std::vector<DeviceInfo> device_list = index->getDeviceList();
            print_capture_devices(device_list);
            return 0;
        }
        else if (arg == "-p" || arg == "--list-properties")
        {
            do_this = LIST_PROPERTIES;
            break;
        }
        else if (arg == "-s" || arg == "--set")
        {
            way = SET;
            do_this = SET_PROPERTY;
            int tmp_i = i;
            tmp_i++;
            if (tmp_i >= argc)
            {
                std::cout << "--set requires additional arguments to work properly!" << std::endl;
                return 1;
            }
            param = argv[tmp_i];
            i++;

            // break;
        }
        else if (arg == "-f" || arg == "--format")
        {
            do_this = LIST_FORMATS;
            // break;
        }
        else
        {
            serial = arg;
        }
    }

    if (serial.empty())
    {
        std::cout << "No serial given!" << std::endl;
        return 1;
    }


    auto device_index = getDeviceIndex();
    sleep(3);
    std::vector<DeviceInfo> device_list = device_index->getDeviceList();



    Grabber g;

    for (auto& d : device_list)
    {
        if (d.getSerial().compare(serial) == 0)
        {
            g.openDevice(d);

        }
    }

    if (!g.isDeviceOpen())
    {
        std::cout << "Unable to find device with serial '" << serial << "'" << std::endl;
        return 1;
    }

    switch (do_this)
    {
        case LIST_PROPERTIES:
        {
            print_properties(g.getAvailableProperties());
            break;
        }
        case LIST_FORMATS:
        {
            list_formats(g.getAvailableVideoFormats());
            break;
        }
        case SET_PROPERTY:
        {
            set_property(g, param);
            break;
        }
        case SET_FORMAT:
        {
            set_active_format(g, param);
            break;
        }
        case SAVE_STREAM:
        {
            //save_stream(g, filename);
        }
        case SAVE_IMAGE:
        {
            save_image(g, filename);
            break;
        }
        default:
        {
            std::cout << "Unknown command." << std::endl;
            break;
        }
    }

    return 0;
}
