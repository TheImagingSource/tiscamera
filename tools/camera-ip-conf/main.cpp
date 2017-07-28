/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

#include "ConsoleManager.h"

#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <libgen.h>

#include <libgen.h>

#include "version.h"

using namespace tis;


/// @name printHelp
/// @param name - program name
/// @brief prints complete overview over possible actions
void printHelp (char *execName)
{
    std::cout << "\n" << execName << " - configuration tool for The Imaging Source GigE cameras"
              << "\n\nusage: " << execName << " [command] [args]        - execute specified command\n\n"

              << "Available commands:\n"
              << "    list     - list all available GigE cameras\n"
              << "    info     - print all information for camera\n"
              << "    set      - permanently configure camera settings\n"
              << "    rescue   - temporarily set network addresses\n"
              << "    upload   - upload new firmware to camera\n"
              << "    help     - print this text\n"
              << std::endl;

    std::cout << "\nAvailable parameters:\n"
              << "    -h                       - same as help\n"
              << "    -i                       - same as info\n"
              << "    -l                       - same as list\n"
              << "    ip=X.X.X.X               - ip address to be set\n"
              << "    subnet=X.X.X.X           - subnetmask to be set\n"
              << "    gateway=X.X.X.X          - gateway to be set\n"
              << "    dhcp=on/off              - toggle dhcp state\n"
              << "    static=on/off            - toggle static ip state\n"
              << "    name=\"xyz\"             - set name for camera; maximum 15 characters\n"
              << "    firmware=firmware.zip    - file containing new firmware\n"
              << std::endl;

    std::cout << "Camera identification:\n"
              << "    --serial     -s          - serial number of camera\n"
              << "    --name       -n          - user defined name of camera\n"
              << "    --mac        -m          - MAC of camera\n"
              << std::endl;

    std::cout << "Examples:\n\n"
              << "  Temporarily set a fixed IP address, gateway and subnet mask for the camera\n"
              << "  with the serial number 27710767. The camera does not need to be on the same\n"
              << "  subnet:\n\n"
              << "    " << execName << " rescue ip=192.168.1.100 gateway=192.168.1.1 subnet=255.255.255.0 -s 27710767\n\n"
              << "  Permanently set a fixed IP adress on the same camera. Camera needs to be\n"
              << "  on the same subnet:\n\n"
              << "    " << execName << " set ip=192.168.1.100 gateway=192.168.1.1 subnet=255.255.255.0 -s 27710767\n"
              << std::endl;

    std::cout << "Version Information:\n\n"
              << "git revision: " << GIT_REVISION << std::endl;
    std::cout << "tcam lib version: " << TCAM_VERSION << std::endl;
}


void handleCommandlineArguments (const int argc, char* argv[])
{
    if (argc == 1)
    {
        printHelp(basename(argv[0]));
        return;
    }

    // std::string makes things easier to handle
    // we don;t need the program name itself and just ignore it
    std::vector<std::string> args(argv+1, argv + argc);

    try
    {
        for (const auto& arg : args)
        {
            if ((arg.compare("help") == 0) || (arg.compare("-h") == 0))
            {
                printHelp(basename(argv[0]));
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
                std::cout << "\n!!! Using the 'forceip' is DEPRECATED !!!"
                "\nThis command got replaced by the 'rescue' command and will be "
                "removed in the future!\n" << std::endl;
                rescue(args);
                break;
            }
            else if (arg.compare("upload") == 0)
            {
                upgradeFirmware(args);
                break;
            }
            else if (arg.compare("rescue") == 0)
            {
                rescue(args);
                break;
            }
            else
            {
                std::cout << "Unknown parameter \"" << arg << "\"\n" << std::endl;
                return;
            }
        }
    }
    catch (std::exception& exc)
    {
        std::cout << "\n" << exc.what()
                  << "\n" << std::endl;
        exit(1);
    }
}


int main (int argc, char* argv[])
{
    handleCommandlineArguments(argc, argv);

    return 0;
}
