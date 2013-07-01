///
/// @file main.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///
/// main file for the commandline client that allows IP configuration of GigE cameras
///

#include "ConsoleManager.h"

#include <string>
#include <vector>
#include <iostream>

using namespace tis;


/// @name printHelp
/// @brief prints complete overview over possible actions
void printHelp ()
{
    std::cout << std::endl << "tis_network - network module for GigE cameras" << std::endl
              << std::endl
              << "usage: tis_network [command]             - execute specified command" << std::endl
              << "   or: tis_network [command] -c <camera> - execute command for given camera" << std::endl
              << std::endl << std::endl;

    std::cout << "Available commands: " << std::endl
              << "    list     - list all available GigE cameras" << std::endl
              << "    info     - print all information for camera" << std::endl
              << "    set      - configure camera settings" << std::endl
              << "    forceip  - force ip onto camera" << std::endl
              << "    rescue   - broadcasts to MAC given settings" << std::endl
              << "    firmware - upload new firmware to camera" << std::endl
              << "    help     - print this text"
              << std::endl << std::endl;

    std::cout << std::endl
              << "Available parameter:" << std::endl
              << "    -c <camera-serianumber>  - specify camera to use" << std::endl
              << "    -h                       - same as help" << std::endl
              << "    -i                       - same as info" << std::endl
              << "    -l                       - same as list" << std::endl
              << "    ip=X.X.X.X               - specifiy persistent ip that camera shall use" << std::endl
              << "    subnet=X.X.X.X           - specifiy persistent subnetmask that camera shall use" << std::endl
              << "    gateway=X.X.X.X          - specifiy persistent gateway that camera shall use" << std::endl
              << "    dhcp=on/off              - toggle dhcp state" << std::endl
              << "    static=on/off            - toggle static ip state" << std::endl
              << "    name=\"xyz\"               - set name for camera; maximum 16 characters" << std::endl
              << "    " << std::endl
              << std::endl;

    std::cout << "Examples:" << std::endl
              << std::endl
              << "    tis_network set gateway=192.168.0.1 -c 46210199" << std::endl
              << "    tis_network forceip ip=192.168.0.100 subnet=255.255.255.0 gateway=192.168.0.1 -c 46210199" << std::endl
              << std::endl << std::endl;
}


void handleCommandlineArguments (const int argc, char* argv[])
{
    if (argc == 1)
    {
        printHelp();
        return;
    }

    // std::string makes things easier to handle
    // we don;t need the program name itself and just ignore it
    std::vector<std::string> args(argv+1, argv + argc);

    for (const auto& arg : args)
    {
        if ((arg.compare("help") == 0) || (arg.compare("-h") == 0))
        {
            printHelp();
            return;
        }
        else if ((arg.compare("list") == 0) || (arg.compare("-l") == 0))
        {
            listCameras();
            break;
        }
        else if ((arg.compare("info") == 0) || arg.compare("-i") == 0)
        {
            if (argc <= 2)
            {
                std::cout << "Not enough arguments." << std::endl;
                return;
            }
            printCameraInformation(args);
            break;
        }
        else if (arg.compare("set") == 0)
        {
            if (argc <= 2)
            {
                std::cout << "Not enough arguments." << std::endl;
                return;
            }
            setCamera(args);
            break;
        }
        else if (arg.compare("forceip") == 0)
        {
            forceIP(args);
            break;
        }
        else if (arg.compare("firmware") == 0)
        {
            // to be implemented
            std::cout << std::endl << "Not yet available." << std::endl << std::endl;
            break;
        }
        else if (arg.compare("rescue") == 0)
        {
            rescue(args);
            break;
        }
        else
        {
            std::cout << "Unknown parameter \"" << arg << "\"" << std::endl << std::endl;
            return;
        }
    }
}


int main (int argc, char* argv[])
{
    handleCommandlineArguments(argc, argv);

    return 0;
}
