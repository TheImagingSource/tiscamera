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
                            if ((s.compare(long_name) == 0) || (s.compare(short_name) == 0))
                            {
                                return true;
                            }
                            return false;
                        });

    if (iter != args.end() && std::next(iter) != args.end())
    {
        retv = *++iter;
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


void listCameras ()
{
    camera_list cameras;

    std::function<void(std::shared_ptr<Camera>)> f = [&cameras] (std::shared_ptr<Camera> camera) 
    {
        std::mutex cam_lock;
        cam_lock.lock();
        cameras.push_back(camera);
        cam_lock.unlock();
    };

    discoverCameras(f);

    if (cameras.size() == 0)
    {
        std::cout << std::endl << "No cameras found." << std::endl << std::endl;
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
    std::string serial = getSerialFromArgs(args);
    
    if (serial.empty())
    {
        std::cout << std::endl << "No serial number given." << std::endl << std::endl;
        return;
    }

    camera_list cameras;
    std::function<void(std::shared_ptr<Camera>)> f = [&cameras] (std::shared_ptr<Camera> camera) 
    {
        std::mutex cam_lock;
        cam_lock.lock();
        cameras.push_back(camera);
        cam_lock.unlock();
    };

    discoverCameras(f);

    auto camera = getCameraFromList(cameras, serial);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    bool reachable = false;
    if(!camera->isReachable())
    {
        std::cout << std::endl 
                  << "========================================" << std::endl
                  << ">  Camera is currently not reachable!  <" << std::endl
                  << "> To enable full communication set IP  <" << std::endl
                  << ">     configuration via forceip        <" << std::endl
                  << "========================================" << std::endl;
    }
    else
    {
        reachable = true;
    }

    std::cout << std::endl;
    std::cout << "Model:    " << camera->getModelName() << std::endl
              << "Serial:   " << camera->getSerialNumber() << std::endl
              << "Firmware: " << camera->getFirmwareVersion() << std::endl
              << "UserName: " << camera->getUserDefinedName() << std::endl
              << std::endl
              << "MAC Address:        " << camera->getMAC() << std::endl
              << "Current IP:         " << camera->getCurrentIP() << std::endl
              << "Current Subnet:     " << camera->getCurrentSubnet() << std::endl 
              << "Current Gateway:    " << camera->getCurrentGateway() << std::endl
              << std::endl
              << "DHCP is:   ";

    if (camera->isDHCPactive())
    {
        std::cout << "enabled" << std::endl;
    }
    else
    {
        std::cout << "disabled" << std::endl;
    }

    std::cout << "Static is: ";
    if (camera->isStaticIPactive())
    {
        std::cout << "enabled" << std::endl;
    }
    else
    {
        std::cout << "disabled" << std::endl;
    }

    if (reachable)
    {
        std::cout << std::endl
                  << "Persistent IP:      " << camera->getPersistentIP() << std::endl
                  << "Persistent Subnet:  " << camera->getPersistentSubnet() << std::endl
                  << "Persistent Gateway: " << camera->getPersistentGateway() << std::endl <<  std::endl;
    }
}


void setCamera (const std::vector<std::string>& args)
{
    std::string serial = getSerialFromArgs(args);

    if (serial.empty())
    {
        std::cout << std::endl << "No serial number given! Please specifiy!" << std::endl << std::endl;
        return;
    }

    camera_list cameras;
    std::function<void(std::shared_ptr<Camera>)> f = [&cameras] (std::shared_ptr<Camera> camera) 
        {
            std::mutex cam_lock;
            cam_lock.lock();
            cameras.push_back(camera);
            cam_lock.unlock();
        };

    discoverCameras(f);

    auto camera = getCameraFromList(cameras, serial);

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

        std::cout << "Setting Gateway...." << std::endl;
        if (camera->setPersistentGateway(gateway))
        {
            std::cout << "  Done." << std::endl;
        }
        else
        {
            std::cout << "  Unable to set Gateway." << std::endl;
        }
    }

    std::string dhcp = getArgument (args,"dhcp");
    if (!dhcp.empty())
    {
        bool check = camera->isDHCPactive();
       
        if (dhcp.compare("on") == 0)
        {
            if (check)
            {
                std::cout << "DHCP is already on." << std::endl;
            }
            else
            {
                std::cout << "Enabling DHCP...." << std::endl;
                
                if (camera->setDHCPstate(true))
                {
                    std::cout << "  Done." << std::endl; 
                }
                else
                {
                    std::cout << "  Unable to set DHCP state." << std::endl;
                }
            }
        }
        else if (dhcp.compare("off") == 0 )
        {
            std::cout << "Disabling DHCP...." << std::endl;
            if (camera->setDHCPstate(false))
            {
                std::cout << "  Done." << std::endl; 
            }
            else
            {
                std::cout << "  Unable to set DHCP state." << std::endl;
            }
        }
        else
        {
            std::cout << "Unknown setting \"" << dhcp << "\"" << std::endl;
        }
    }

    std::string staticIP = getArgument (args,"static");
    if (!staticIP.empty())
    {
        bool check = camera->isStaticIPactive();
       
        if (staticIP.compare("on") == 0)
        {
            if (check)
            {
                std::cout << "Static IP is already on." << std::endl;
            }
            else
            {
                std::cout << "Enabling static IP...." << std::endl;
                if (camera->setStaticIPstate(true))
                {
                    std::cout << "  Done." << std::endl; 
                }
                else
                {
                    std::cout << "  Unable to set static IP state." << std::endl;
                }
            }
        }
        else if (staticIP.compare("off") == 0 )
        {
            std::cout << "Disabling static IP...." << std::endl;
            if (camera->setStaticIPstate(false))
            {
                std::cout << "  Done." << std::endl; 
            }
            else
            {
                std::cout << "  Unable to set static IP state." << std::endl;
            }
        }
        else
        {
            std::cout << std::endl << "Unknown setting \"" << dhcp << "\"" << std::endl << std::endl;
        }
    }

    std::string name = getArgument (args,"name");
    if (!name.empty())
    {
        std::cout << "Setting user defined name...." << std::endl;
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
    std::string serial = getSerialFromArgs(args);

    if (serial.empty())
    {
        std::cout << std::endl << "No serial number given! Please specify!" << std::endl << std::endl;
        return;
    }

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

    camera_list cameras;
    std::function<void(std::shared_ptr<Camera>)> f = [&cameras] (std::shared_ptr<Camera> camera) 
    {
        std::mutex cam_lock;
        cam_lock.lock();
        cameras.push_back(camera);
        cam_lock.unlock();
    };

    discoverCameras(f);

    auto camera = getCameraFromList(cameras, serial);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "Do you really want to enforce the following configuration? " << std::endl << std::endl
              << "Serial Number: " << camera->getSerialNumber() << std::endl
              << "IP:     " << ip << std::endl
              << "Subnet: " << subnet << std::endl
              << "Gateway:" << gateway << std::endl << std::endl;
    std::cout << "Enforce IP? [y/N] ";

    std::string really;
    std::cin >> really;
    if (really.compare("y") == 0 )
    {
        std::cout << std::endl << "Sending forceIP...." << std::endl << std::endl;
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
        std::cout << std::endl <<  "Aborted forceIP!" << std::endl << std::endl;
    }
}


void upgradeFirmware (const std::vector<std::string>& args)
{
    std::string serial = getSerialFromArgs(args);
    if (serial.empty())
    {
        std::cout << std::endl << "No serial number given! Please specify!" << std::endl << std::endl;
        return;
    }

    std::string firmware = getArgument (args, "firmware");
    if (firmware.empty())
    {
        std::cout << "Please specify a valid firmware file." << std::endl;
        return;
    }

    char actualpath [PATH_MAX+1];
    std::string cF (realpath(firmware.c_str(), actualpath));

    std::cout << "===========================" << firmware << " - " << cF << std::endl;

    camera_list cameras;
    std::function<void(std::shared_ptr<Camera>)> f = [&cameras] (std::shared_ptr<Camera> camera)
    {
        std::mutex cam_lock;
        cam_lock.lock();
        cameras.push_back(camera);
        cam_lock.unlock();
    };

    discoverCameras(f);

    auto camera = getCameraFromList(cameras, serial);

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
        std::cout << std::endl << "Successfully uploaded firmware. \
Please reconnect your camera to assure full functionality." << std::endl;
    }
    else
    {
        std::cout << std::endl << "ERROR aborted upgrade. " << std::endl;
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
