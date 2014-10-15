
#include "properties.h"
#include "formats.h"
#include "multimedia.h"

#include <tis.h>

#include <iostream>
#include <iomanip>

using namespace tis_imaging;


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


void print_capture_devices (const std::vector<CaptureDevice>& devices)
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
    
};


int main (int argc, char *argv[])
{
    std::string serial = "25410069";
    modes do_this;

    if (argc == 1)
    {
        print_help();
        return 0;
    }
    
    for (unsigned int i = 0; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            print_help();
            return 0;
        }
        else if (arg == "-l" || arg == "--list")
        {
            std::vector<CaptureDevice> device_list = getAvailableCaptureDevices();
            print_capture_devices(device_list);
            break;
        }
        else if (arg == "-p" || arg == "--list-properties")
        {
            do_this = LIST_PROPERTIES;
            break;
        }
        else if (arg == "-s" || arg == "--set")
        {
            do_this = SET_PROPERTY;
            break;
        }
        else if (arg == "-f" || arg == "--format")
        {
            do_this = LIST_FORMATS;
            break;
        }
    }

    

    std::vector<CaptureDevice> device_list = getAvailableCaptureDevices();

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
    }

    return 0;
}
