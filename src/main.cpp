
#include "tis.h"

// #include "camera_index.h"
// #include "camera_index.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <vector>


using namespace tis_imaging;


// Property createProp ()
// {

//     tis_value_int i = {0, 100, 1, 5, 50};
    
//     camera_property cp = {"Bla", PROPERTY_TYPE_INTEGER, {i}, false, false, 0};
    
//     return PropertyInteger(cp);
// }


std::string fourcc2string (const uint32_t& fourcc)
{

    
    union _bla
    {
        uint32_t i;
        char c[4];
    } bla;

    bla.i = fourcc;

    std::string s = bla.c;
    
    // std::string s ( (char*)&fourcc);
    // s += "\0";
    return s;
}

int main (int /* argc */, char* /* argv */ [])
{

    Grabber g;

    auto dev = getAvailableCaptureDevices();

    std::cout << "Number of Cameras: " << dev.size() << std::endl;

    for (const auto& d : dev)
    {
        std::cout << "====================" << std::endl << std::endl;
        std::cout << d.getName() << " - " << d.getSerial() <<std::endl;

        // if (d.getSerial().compare("20410002") == 0)
        if (d.getSerial().compare("28210147") == 0)
        {
            if (!g.openDevice(d))
            {
                std::cout << "Error while opening device." << std::endl;
            }
        }
    }

    
    //g.openDevice(dev.at(0));

    if (g.isDeviceOpen())
    {
        std::cout << "Got an active device." << std::endl;

        auto d = g.getDevice();

        std::cout << d.getName() <<  std::endl;

        
    }
    else
    {
        std::cout << "Device not opened!" << std::endl;
        exit(1);
    }

    // auto frmts = g.getAvailableVideoFormats();

    // for (const auto& f : frmts)
    // {
    //     auto desc = f.getFormatDescription();

    //     std::cout << desc.description << " - " << desc.fourcc << std::endl;

    //     // std::cout << f.getFrameRates().size()<< std::endl;
    //     for (const auto& d : f.getFrameRates())
    //     {
    //         std::cout << "    " << d << std::endl;
    //     }
    // }

    // const auto& f = frmts.at(0).getFormatDescription();
    
    // struct video_format vid_frmt = {f.fourcc,
    //                                 f.min_size.width,
    //                                 f.min_size.height,
    //                                 f.binning,
    //                                 frmts.at(0).getFrameRates().at(1)};
    
    // VideoFormat form(vid_frmt);

    // std::cout << "Setting: "
    //           << f.min_size.width << "x" 
    //           << f.min_size.height << " - "
    //           << frmts.at(0).getFrameRates().at(0)
    //           << std::endl;
    
    // g.setVideoFormat(form);
    // g.closeDevice();

    std::cout << std::endl << "PROPERTIES" << std::endl  << std::endl;
    
    auto properties = g.getAvailableProperties();

    std::cout << "Iterating properties (" << properties.size() << "):" << std::endl;
    for (auto& p : properties)
    {
        if (p == nullptr)
        {
            std::cout << "WAT"  << std::endl;
            continue;
        }
        
        std::cout << std::endl << "  " << p.getName() << std::endl;
        std::cout << "    " << propertyType2String( p.getType()) << std::endl;

        if (p.getType() == PROPERTY_TYPE_INTEGER)
        {
            // std::shared_ptr<PropertyInteger> pi = std::static_pointer_cast<PropertyInteger>(p);
            // auto pi = dynamic_cast<PropertyInteger&>(p);
            auto pi = (PropertyInteger&)p;

            
            std::cout << "    Range:   " << pi.getMin() << " - " <<  pi.getMax() << std::endl;
            std::cout << "    Current: " << pi.getValue() << std::endl;
            std::cout << "    Default: " << pi.getDefault()  << std::endl;

            // pi.setValue(pi.getValue()+1);
        }
        
    }

    // auto prop = createProp();

    // PropertyInteger pi = (PropertyInteger&)(prop);

    // std::cout << pi.getName() << " - " << pi.getMax() << std::endl;
    
    g.closeDevice();

    return 0;
}











