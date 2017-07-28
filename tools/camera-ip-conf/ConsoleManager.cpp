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
#include "CameraDiscovery.h"
#include "Camera.h"
#include "utils.h"
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>

#include <thread>
#include <mutex>
#include <exception>

namespace tis
{

std::string getArgumentValue (const std::vector<std::string>& args,
                              const std::string& long_name,
                              const std::string& short_name)
{
    std::string retv;

    auto iter = find_if(args.begin(), args.end(),  [&long_name, &short_name] (std::string s)
                        {
                            if ((startsWith(s, long_name)) || (startsWith(s, short_name)))
                            {
                                return true;
                            }
                            return false;
                        });

    if (iter == args.end())
    {
        return retv;
    }
    if ((*iter).find("=") != std::string::npos)
    {
            std::string s = *iter;
            unsigned pos = s.find("=");

            retv = s.substr(pos+1);
    }
    else if (std::next(iter) != args.end())
    {
        retv = *(std::next(iter));
    }

    return retv;
}


camera_list getCameraList ()
{
    camera_list cameras;
    std::mutex cam_lock;

    std::function<void(std::shared_ptr<Camera>)> f = [&cameras, &cam_lock] (std::shared_ptr<Camera> camera)
    {
        std::lock_guard<std::mutex> mutex_lock(cam_lock);
        cameras.push_back(camera);
    };

    discoverCameras(f);

    return cameras;
}


std::shared_ptr<Camera> findCamera (const std::vector<std::string>& args)
{
    std::shared_ptr<Camera> retv;

    std::string serial = getArgumentValue(args, "--serial", "-s");
    std::string name   = getArgumentValue(args, "--name",   "-n");
    std::string mac    = getArgumentValue(args, "--mac",    "-m");

    if (serial.empty() && name.empty() && mac.empty())
    {
        throw std::invalid_argument("No camera identifier found.");
    }

    camera_list cameras = getCameraList();

    if (cameras.size() == 0)
    {
        throw std::runtime_error("\nNo cameras found.\n");
    }

    if (!serial.empty())
    {
        retv = getCameraFromList(cameras, serial, CAMERA_SERIAL);
    }
    else if (!name.empty())
    {
        retv = getCameraFromList(cameras, name, CAMERA_NAME);
    }
    else
    {
        retv = getCameraFromList(cameras, mac, CAMERA_MAC);
    }

    return retv;
}


void listCameras ()
{
    camera_list cameras = getCameraList();

    if (isRPFilterActive())
    {
        std::cout << "Your system has active rp_filter." << std::endl;
        std::cout << "This can prevent misconfigured cameras from being detected." << std::endl;
        std::cout << "To temporarily disable rp_filter execute:" << std::endl;
        std::cout << "\tsudo sysctl -w net.ipv4.conf.all.rp_filter=0" << std::endl;
    }

    if (cameras.size() == 0)
    {
        std::cout << "\nNo cameras found.\n" << std::endl;
        return;
    }

    // header for table
    std::cout << "\n" << std::left << std::setw(15) << "Model"
              << std::setw(3)  << " - "
              << std::setw(9)  << "Serial"
              << std::setw(3)  << " - "
              << std::setw(15) << "User Def. Name"
              << std::setw(3)  << " - "
              << std::setw(15) << "Current IP"
              << std::setw(3)  << " - "
              << std::setw(15) << "Current Netmask"
              << std::setw(3)  << " - "
              << std::setw(15) << "Current Gateway"
              << std::endl;

    for (const auto& cam : cameras)
    {
        std::cout << std::left << std::setw(15) << cam->getModelName()
                  << std::setw(3)  << " - "
                  << std::setw(9)  << cam->getSerialNumber()
                  << std::setw(3)  << " - "
                  << std::setw(15) << cam->getUserDefinedName()
                  << std::setw(3)  << " - "
                  << std::setw(15) << cam->getCurrentIP()
                  << std::setw(3)  << " - "
                  << std::setw(15) << cam->getCurrentSubnet()
                  << std::setw(3)  << " - "
                  << std::setw(15) << cam->getCurrentGateway()
                  << std::endl;
    }
    std::cout << std::endl;
}


void printCameraInformation (const std::vector<std::string>& args)
{
    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    bool reachable = false;
    if(!camera->isReachable())
    {
        std::cout << "\n========================================"
                  << "\n>  Camera is currently not reachable!  <"
                  << "\n> To enable full communication set IP  <"
                  << "\n>     configuration via forceip        <"
                  << "\n========================================" << std::endl;
    }
    else
    {
        reachable = true;
    }

    std::cout << "\nModel:    " << camera->getModelName()
              << "\nSerial:   " << camera->getSerialNumber()
              << "\nFirmware: " << camera->getFirmwareVersion()
              << "\nUserName: " << camera->getUserDefinedName()
              << "\n\nMAC Address:        " << camera->getMAC()
              << "\nCurrent IP:         " << camera->getCurrentIP()
              << "\nCurrent Subnet:     " << camera->getCurrentSubnet()
              << "\nCurrent Gateway:    " << camera->getCurrentGateway()
              << "\n\nDHCP is:   " << (camera->isDHCPactive() ? "enabled" : "disabled")
              << "\nStatic is: " << (camera->isStaticIPactive() ? "enabled" : "disabled") << std::endl;

    if (reachable)
    {
        std::cout << "\n\nPersistent IP:      " << camera->getPersistentIP()
                  << "\nPersistent Subnet:  " << camera->getPersistentSubnet()
                  << "\nPersistent Gateway: " << camera->getPersistentGateway() << "\n" <<  std::endl;
    }
}


void writeChanges (std::shared_ptr<Camera> camera,
                   const std::string ip,
                   const std::string subnet,
                   const std::string gateway)
{
    std::cout << "Writing changes...." << std::endl;
    if (!camera->setPersistentIP(ip))
    {
        throw std::runtime_error( "  Unable to set IP address.");
    }

    if (!camera->setPersistentSubnet(subnet))
    {
        throw std::runtime_error( "  Unable to set Subnetmask.");
    }

    if (!camera->setPersistentGateway(gateway))
    {
        throw std::runtime_error("  Unable to set Gateway.");
    }
}

int isAccessible (const std::vector<std::string>& args)
{
    auto camera = findCamera(args);

    return isAccessible(camera);
}

int isAccessible (std::shared_ptr<Camera> camera)
{
    int ret = false;

    if (camera == NULL)
    {
        ret = false;
    } else {
        if (camera->getControl())
        {
            ret = true;
            camera->abandonControl();
        }
    }

    if (ret == false)
    {
        std::cout << "Could not gain read/write access to the camera.\n"
        "Please make sure the camera is on the same network as the host computer \n"
        "and no other application is currently accessing the device." << std::endl;
    }

    return ret;
}


void setCamera (const std::vector<std::string>& args)
{
    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    if (!isAccessible(camera))
    {
        return;
    }

    bool write_changes = false;

    std::string ip = getArgumentValue(args, "ip", "");
    if (!ip.empty())
    {
        if (!isValidIpAddress(ip))
        {
            std::cout << "Please enter a valid IP address." << std::endl;
            return;
        }
        else
        {
            write_changes = true;
        }
    }
    else
    {
        ip = camera->getPersistentIP();
    }

    std::string subnet = getArgumentValue(args, "subnet", "");
    if (!subnet.empty())
    {
        if (!isValidIpAddress(subnet))
        {
            std::cout << "Please enter a valid subnet address." << std::endl;
            return;
        }
        else
        {
            write_changes = true;
        }
    }
    else
    {
        subnet = camera->getPersistentSubnet();
    }

    std::string gateway = getArgumentValue(args, "gateway", "");
    if (!gateway.empty())
    {
        if (!isValidIpAddress(gateway))
        {
            std::cout << "Please enter a valid gateway address." << std::endl;
            return;
        }
        else
        {
            write_changes = true;
        }
    }
    else
    {
        gateway = camera->getPersistentGateway();
    }

    if (write_changes)
    {
        try
        {
            writeChanges(camera, ip, subnet, gateway);
        }
        catch (std::runtime_error run)
        {
            throw run;
        }
    }

    // if one exists we have to check them both
    if (!getArgumentValue(args, "dhcp", "").empty() || !getArgumentValue(args, "static", "").empty())
    {
        bool dhcp = camera->isDHCPactive();
        bool staticIP = camera->isStaticIPactive();

        std::string d = getArgumentValue(args, "dhcp", "");
        if (!d.empty())
        {
            if (d.compare("on") == 0 || d.compare("off") == 0)
            {
                (d.compare("on") == 0) ? dhcp = true : dhcp = false;
            }
            else
            {
                throw std::invalid_argument("Unable to interpret dhcp argument as value: " + d);
            }
        }

        std::string s = getArgumentValue(args, "static", "");
        if (!s.empty())
        {
            if (s.compare("on") == 0 || s.compare("off") == 0)
            {
                (s.compare("on") == 0) ? staticIP = true : staticIP = false;
            }
            else
            {
                throw std::invalid_argument("Unable to interpret static ip argument as value: " + s);
            }
        }
        std::cout << "DHCP will be: " << ((dhcp) ? "on" : "off")
                  << "\nStatic IP to: " << ((staticIP) ? "on" : "off")
                  << "\n\nWriting IP configuration...." << std::endl;

        if (camera->setIPconfigState(dhcp, staticIP))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            throw std::runtime_error("  Error while setting IP configuration.");
        }
    }

    auto iter_name = find_if(args.begin(), args.end(), [] (std::string s)
                             {
                                 if (startsWith(s, "name"))
                                 {
                                     return true;
                                 }
                                 return false;
                             });
    // if name exists we have to evaluate to allow the setting of empty names
    if (iter_name != args.end())
    {
        std::string name = getArgumentValue(args, "name", "");
        std::cout << "Setting user defined name to \"" <<  name << "\" ...." << std::endl;
        if (camera->setUserDefinedName(name))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            throw std::runtime_error("  Unable to set name.");
        }
    }
    std::cout << std::endl;
    std::cout << "Configuration written to camera EEPROM.\n"
    "Power-cycle the camera to activate the new settings." << std::endl;
}


void forceIP (const std::vector<std::string>& args)
{
    std::string ip = getArgumentValue (args, "ip", "");
    if (ip.empty() || !isValidIpAddress(ip))
    {
        std::cout << "Please specifiy a valid IP address." << std::endl;
        return;
    }

    std::string subnet = getArgumentValue (args, "subnet", "");
    if (subnet.empty() || !isValidIpAddress(subnet))
    {
        std::cout << "Please specifiy a valid subnet mask." << std::endl;
        return;
    }

    std::string gateway = getArgumentValue (args, "gateway", "");
    if (gateway.empty() || !isValidIpAddress(gateway))
    {
        std::cout << "Please specifiy a gateway address." << std::endl;
        return;
    }

    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    std::cout << "\nEnforcing IP Configuration.\n\n"
              << "Serial Number:     " << camera->getSerialNumber()
              << "\nCurrent Settings:"
              << "\n    IP:      " << camera->getCurrentIP()
              << "\n    Subnet:  " << camera->getCurrentSubnet()
              << "\n    Gateway: " << camera->getCurrentGateway()
              << "\n\nDo you really want to enforce the following configuration?\n"
              << "\n    IP:     " << ip
              << "\n    Subnet: " << subnet
              << "\n    Gateway:" << gateway << std::endl;
    std::cout << "\n\nEnforce IP? [y/N] ";

    std::string really;
    std::cin >> really;
    if (really.compare("y") == 0 )
    {
        std::cout << "\nSending forceIP....\n" << std::endl;
        if (camera->forceIP(ip, subnet, gateway))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            std::cout << "  Failed to set address." << std::endl;
        }
    }
    else
    {
        std::cout << "\nAborted forceIP!\n" << std::endl;
    }
}


void upgradeFirmware (const std::vector<std::string>& args)
{
    std::string firmware = getArgumentValue(args, "firmware", "");
    if (firmware.empty())
    {
        std::cout << "Please specify a valid firmware file." << std::endl;
        return;
    }

    char actualpath [PATH_MAX+1];
    std::string cF (realpath(firmware.c_str(), actualpath));

    std::cout << "Updating the camera using the file: " << cF << std::endl;
    std::cout << "\n!!! IMPORTANT NOTE !!!\n"
    "Do not interrupt the firmware update process.\n"
    "Do not disconnect the camera during the firmware update process.\n"
    "A failed firmware update may render the camera unusable.\n\n"
    "Start the update process [y/N]";

    std::string really;
    std::cin >> really;
    if (really.compare("y") != 0 )
        return;

    std::string overrideModelName = getArgumentValue(args, "overrideModelName", "");

    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    if (!isAccessible(camera))
    {
        return;
    }

    auto func = [] (int progress,const std::string& s)
        {
            std::cout << "\r";

            std::cout << std::setw(5) << progress << "%";
            if (s != "")
            {
                std::cout << std::setw(40) << s;
            }
            std::cout.flush();
        };


    std::cout << std::endl;
    if (camera->uploadFirmware(cF, overrideModelName, func))
    {
        std::cout << "\n\nSuccessfully uploaded firmware. \
Please reconnect your camera to assure full functionality.\n";
    }
    else
    {
        std::cout << "\n\nERROR during upgrade. Firmware upload aborted.\n"
        "Do not disconnect the camera yet. Please retry the firmware update.\n"
        "Contact the technical support if this message keeps comming up.\n";
    }
    std::cout << std::endl;
}

std::string string_format(const std::string &fmt, ...) {
    int n, size=1;
    std::string str;
    va_list ap;

    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf(&str[0], size, fmt.c_str(), ap);
        va_end(ap);

        if (n > -1 && n < size)
            return str;
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
}

std::string parseHexMac(std::string hexmac)
{
    if (hexmac.length() != 13)
        return "";

    std::string mac = hexmac.substr(0,2);
    for (int i=2; i < 12; i+=2)
        mac += ":" + hexmac.substr(i,2);
    return mac;
}

std::string serialToMac(std::string serial)
{
    if(serial.length() != 8)
        return "";
    std::string tmp = serial.substr(2,2);
    tmp = tmp.substr(1,1) + tmp.substr(0,1);
    int macPart = std::stoi(serial.substr(4,4)) + (10000 * (std::stoi(tmp) - 9)) + (10000 * 30 * (std::stoi(serial.substr(0,2)) - 1));
    std::string prefix = "000748";
    std::string macString = prefix + std::string(string_format("%06x", macPart));
    return parseHexMac(macString);
}

void rescue (std::vector<std::string> args)
{
    std::string mac = getArgumentValue(args, "--mac", "-m");
    if (mac.empty())
    {
        std::string serial = getArgumentValue(args, "--serial", "-s");
        if (serial.empty())
        {
            std::cout << "Please specify a MAC address or serial number." << std::endl;
            return;
        }
        mac = serialToMac(serial);
        std::cout << "Using MAC address " << mac << "." << std::endl;
    }

    if (!isValidMAC(mac))
    {
        std::cout << "Not a valid MAC address. Format is XX:XX:XX:XX:XX:XX" << std::endl;
        return;
    }

    std::string ip = getArgumentValue(args, "ip", "");
    if (ip.empty() || !isValidIpAddress(ip))
    {
        std::cout << "Please specify a valid IP address." << std::endl;
        return;
    }

    std::string subnet = getArgumentValue(args, "subnet", "");
    if (subnet.empty() || !isValidIpAddress(subnet))
    {
        std::cout << "Please specify a valid subnet mask." << std::endl;
        return;
    }


    std::string gateway = getArgumentValue (args,"gateway", "");
    if (gateway.empty() || !isValidIpAddress(gateway))
    {
        std::cout << "Please specify a gateway address." << std::endl;
        return;
    }

    try
    {
        sendIpRecovery(mac, ip2int(ip), ip2int(subnet), ip2int(gateway));
    }
    catch (...)
    {
        std::cout << "An Error occured while sending the rescue packet." << std::endl;
    }

    std::cout << "Rescue packet sent. Camera configuration will be lost on next power cycle.\n"
    "Use 'set' command to make permanent configuration changes." << std::endl;
}

} /* namespace tis */
