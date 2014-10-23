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

using namespace tis;


/// @name printHelp
/// @brief prints complete overview over possible actions
void printHelp ()
{
    std::cout << "\ntis_network - network module for GigE cameras"
              << "\n\nusage: tis_network [command]             - execute specified command"
              << "\n   or: tis_network [command] -c <camera> - execute command for given camera\n\n\n"

              << "Available commands:\n"
              << "    list     - list all available GigE cameras\n"
              << "    info     - print all information for camera\n"
              << "    set      - configure camera settings\n"
              << "    forceip  - force ip onto camera\n"
              << "    rescue   - broadcasts to MAC given settings\n"
              << "    upload   - upload new firmware to camera\n"
              << "    help     - print this text\n"
              << std::endl;

    std::cout << "\nAvailable parameter:\n"
              << "    -h                       - same as help\n"
              << "    -i                       - same as info\n"
              << "    -l                       - same as list\n"
              << "    ip=X.X.X.X               - specifiy persistent ip that camera shall use\n"
              << "    subnet=X.X.X.X           - specifiy persistent subnetmask that camera shall use\n"
              << "    gateway=X.X.X.X          - specifiy persistent gateway that camera shall use\n"
              << "    dhcp=on/off              - toggle dhcp state\n"
              << "    static=on/off            - toggle static ip state\n"
              << "    name=\"xyz\"               - set name for camera; maximum 15 characters\n"
              << "    firmware=firmware.zip    - file containing new firmware\n"
              << std::endl;

    std::cout << "Camera identification is possible via:\n"
              << "    --serial     -s          - serial number of camera\n"
              << "    --name       -n          - user defined name of camera\n"
              << "    --mac        -m          - MAC of camera\n"
              << std::endl;

    std::cout << "Examples:\n\n"

              << "    tis_network set gateway=192.168.0.1 -s 46210199\n"
              << "    tis_network forceip ip=192.168.0.100 subnet=255.255.255.0 gateway=192.168.0.1 -s 46210199\n\n"
              << std::endl;
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

    try
    {
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
