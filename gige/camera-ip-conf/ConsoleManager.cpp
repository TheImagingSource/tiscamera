///
/// @file ConsoleManager.cpp
/// 
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "ConsoleManager.h"
#include "CameraDiscovery.h"
#include "Camera.h"
#include "utils.h"
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <thread>
#include <mutex>
#include <exception>

namespace tis
{

std::string getArgument(const std::vector<std::string>& args, const std::string& argument)
{
    for(const auto& arg : args)
    {
        if (tis::startsWith(arg,argument))
        {
            std::string s = arg;
            unsigned pos = s.find("=");

            s = s.substr(pos+1);

            return s;
        }
    }
    return "";
}


std::string getArgumentValue (const std::vector<std::string>& args, const std::string& long_name, const std::string& short_name)
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


std::string getSerialFromArgs(const std::vector<std::string>& args)
{
    for (unsigned int x = 0; x < args.size(); ++x)
    {
        if (args.at(x).compare("-c") == 0)
        {
            if (x+1 < args.size())
            {
                return args.at(x+1);
            }
        }
    }

    return "";
}


camera_list getCameraList ()
{
    camera_list cameras;
    std::mutex cam_lock;

    std::function<void(std::shared_ptr<Camera>)> f = [&cameras, &cam_lock] (std::shared_ptr<Camera> camera)
    {
        cam_lock.lock();
        cameras.push_back(camera);
        cam_lock.unlock();
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
        std::cout << "\nNo cameras found.\n" << std::endl;
        exit(1);
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

    if (cameras.size() == 0)
    {
        std::cout << "\nNo cameras found.\n" << std::endl;
        return;
    }

    // header for table
    std::cout << std::endl << std::setw(12) << "Model"
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
        std::cout << std::setw(12) << cam->getModelName() 
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


void writeChanges (std::shared_ptr<Camera> camera, const std::string ip, const std::string subnet, const std::string gateway)
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


void setCamera (const std::vector<std::string>& args)
{
    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    std::string ip = getArgument (args,"ip");
    if (!ip.empty())
    {
        if (!isValidIpAddress(ip))
        {
            std::cout << "Please enter a valid IP address." << std::endl;
            return;
        }

        std::cout << "Setting IP address...." << std::endl;
        if (camera->setPersistentIP(ip))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            std::cout << "  Unable to set IP address." << std::endl;
        }
    }
    

    std::string subnet = getArgument (args,"subnet");
    if (!subnet.empty())
    {
        if (!isValidIpAddress(subnet))
        {
            std::cout << "Please enter a valid subnet address." << std::endl;
            return;
        }

        std::cout << "Setting Subnetmask...." << std::endl;
        if (camera->setPersistentSubnet(subnet))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            std::cout << "  Unable to set Subnetmask." << std::endl;
        }
    }

    std::string gateway = getArgument (args,"gateway");
    if (!gateway.empty())
    {
        if (!isValidIpAddress(gateway))
        {
            std::cout << "Please enter a valid gateway address." << std::endl;
            return;
        }

    // if one exists we have to check them both
    if (!getArgumentValue(args, "dhcp", "").empty() || !getArgumentValue(args, "static", "").empty())
    {
        bool dhcp = camera->isDHCPactive();
        bool staticIP = camera->isStaticIPactive();

        std::string d = getArgumentValue(args, "dhcp", "");
        if (d.compare("on") == 0 || d.compare("off") == 0)
        {
            (d.compare("on") == 0) ? dhcp = true : dhcp = false;
        }
        else
        {
            throw std::invalid_argument("Unable to interpret dhcp argument as value: " + d);
        }

        std::string s = getArgumentValue(args, "static", "");
        if (s.compare("on") == 0 || s.compare("off") == 0)
        {
            (s.compare("on") == 0) ? staticIP = true : staticIP = false;
        }
        else
        {
            throw std::invalid_argument("Unable to interpret static ip argument as value: " + s);
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
            std::cout << "  Error while setting IP configuration." << std::endl;
            exit(1);
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
            std::cout << "  Unable to set name." << std::endl;
        }
    }
}


void forceIP (const std::vector<std::string>& args)
{
    std::string ip = getArgument (args,"ip");
    if (ip.empty() || !isValidIpAddress(ip))
    {
        std::cout << "Please specifiy a valid IP address." << std::endl;
        return;
    }

    std::string subnet = getArgument (args,"subnet");
    if (subnet.empty() || !isValidIpAddress(subnet))
    {
        std::cout << "Please specifiy a valid subnet mask." << std::endl;
        return;
    }

    std::string gateway = getArgument (args,"gateway");
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
    std::string firmware = getArgument (args, "firmware");
    if (firmware.empty())
    {
        std::cout << "Please specify a valid firmware file." << std::endl;
        return;
    }

    char actualpath [PATH_MAX+1];
    std::string cF (realpath(firmware.c_str(), actualpath));

    std::cout << "===========================" << firmware << " - " << cF << std::endl;

    auto camera = findCamera(args);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    auto func = [] (int progress)
        {
            std::cout << "\r";
            std::string progressSign = "[";

            int i = progress*10 / 100;
            for (int x = 0; x < i; ++x)
            {
                progressSign += "XX";
            }
            for (int x = 0; x < 20 - (i*2); ++x)
            {
                progressSign += " ";
            }


            progressSign += "] ";

            std::cout << progressSign;
            std::cout.flush();
        };


    std::cout << std::endl;
    if (camera->uploadFirmware(cF, func))
    {
        std::cout << "\nSuccessfully uploaded firmware. \
Please reconnect your camera to assure full functionality.\n";
    }
    else
    {
        std::cout << "\nERROR aborted upgrade.\n";
    }
    std::cout << std::endl;
}


void rescue (std::vector<std::string> args)
{
    std::string mac = getArgument (args,"mac");
    if (mac.empty())
    {
        std::cout << "Please specify the MAC address to use." << std::endl;
        return;
    }

    std::string ip = getArgument (args,"ip");
    if (ip.empty() || !isValidIpAddress(ip))
    {
        std::cout << "Please specify a valid IP address." << std::endl;
        return;
    }

    std::string subnet = getArgument (args,"subnet");
    if (subnet.empty() || !isValidIpAddress(subnet))
    {
        std::cout << "Please specify a valid subnet mask." << std::endl;
        return;
    }


    std::string gateway = getArgument (args,"gateway");
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
}

} /* namespace tis */
