/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include <CLI11.hpp>

#include "../../src/tcam-network/Camera.h"
#include "../../src/tcam-network/CameraDiscovery.h"

#include <iostream>
#include <mutex>
#include <memory>
#include <ostream>
#include <string>

namespace
{
enum class IP_Mode
{
    DHCP,
    Static,
    LinkLocal,
};

std::map<std::string, IP_Mode> ip_mode_map
{
    {
        "dhcp",
        IP_Mode::DHCP
    },
    {
        "static",
        IP_Mode::Static
    },
    {
        "linklocal",
        IP_Mode::LinkLocal
    }
};


tis::camera_list get_camera_list()
{
    tis::camera_list cameras;
    std::mutex cam_lock;

    std::function<void(std::shared_ptr<tis::Camera>)> f =
        [&cameras, &cam_lock]
        (std::shared_ptr<tis::Camera> camera)
    {
        std::lock_guard<std::mutex> mutex_lock(cam_lock);

        // filter cameras that are already known
        // this may happen when an interface
        // has multple addresses
        if (std::any_of(cameras.begin(), cameras.end(),
                        [&camera](const std::shared_ptr<tis::Camera> cam)
                        {
                            if (camera->getMAC() == cam->getMAC())
                            {
                                return true;
                            }
                            return false;
                        }))
        {
            return;
        }

        cameras.push_back(camera);
    };

    discoverCameras(f);

    return cameras;
}



void list_cameras(const std::string& format)
{
    tis::camera_list cameras = get_camera_list();

    if (tis::isRPFilterActive())
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

    std::cout << std::left;
    for (unsigned int i = 0; i < format.size(); ++i)
    {

        switch (format.at(i))
        {
            case 'm':
            {
                std::cout << std::setw(20) << "Model Name";
                break;
            }
            case 's':
            {
                std::cout << std::setw(16) << "Serial Number";
                break;
            }
            case 'u':
            {
                std::cout << std::setw(20) << "User Defined Name";
                break;
            }
            case 'i':
            {
                std::cout << std::setw(17) << "Current IP";
                break;
            }
            case 'n':
            {
                std::cout << std::setw(17) << "Current Netmask";
                break;
            }
            case 'g':
            {
                std::cout << std::setw(17) << "Current Gateway";
                break;
            }
            case 'I':
            {
                std::cout << std::setw(17) << "Persistent IP";
                break;
            }
            case 'N':
            {
                std::cout << std::setw(20) << "Persistent Netmask";
                break;
            }
            case 'G':
            {
                std::cout << std::setw(20) << "Persistent Gateway";
                break;
            }
            case 'f':
            {
                std::cout << std::setw(12) << "Interface";
                break;
            }
            case 'd':
            {
                std::cout << std::setw(10) << "DHCP";
                break;
            }
            case 'S':
            {
                std::cout << std::setw(10) << "Static IP";
                break;
            }
            case 'M':
            {
                std::cout << std::setw(18) << "MAC Address";
                break;
            }
            case 'r':
            {
                std::cout << std::setw(10) << "Reachable";
                break;
            }
            default:
            {
                break;
            }
        }

        if (i < format.length() - 1)
        {
            std::cout << " | ";
        }
    }
    std::cout << std::endl;

    for (const auto& cam : cameras)
    {
        for (unsigned int i = 0; i < format.size(); ++i)
        {
            // width is header number +3 to accomodate ` | `
            switch (format.at(i))
            {
                case 'm':
                {
                    std::cout << std::setw(23) << cam->getModelName();
                    break;
                }
                case 's':
                {
                    std::cout << std::setw(19) << cam->getSerialNumber();
                    break;
                }
                case 'u':
                {
                    std::cout << std::setw(23) << cam->getUserDefinedName();
                    break;
                }
                case 'i':
                {
                    std::cout << std::setw(20) << cam->getCurrentIP();
                    break;
                }
                case 'n':
                {
                    std::cout << std::setw(20) << cam->getCurrentSubnet();
                    break;
                }
                case 'g':
                {
                    std::cout << std::setw(20) << cam->getCurrentGateway();
                    break;
                }
                case 'I':
                {
                    std::cout << std::setw(20) << cam->getPersistentIP();
                    break;
                }
                case 'N':
                {
                    std::cout << std::setw(23) << cam->getPersistentSubnet();
                    break;
                }
                case 'G':
                {
                    std::cout << std::setw(23) << cam->getPersistentGateway();
                    break;
                }
                case 'f':
                {
                    std::cout << std::setw(15) << cam->getInterfaceName();
                    break;
                }
                case 'd':
                {
                    if (cam->isDHCPactive())
                    {
                        std::cout << std::setw(13) << "Yes";
                    }
                    else
                    {
                        std::cout << std::setw(13) << "No";
                    }
                    break;
                }
                case 'S':
                {
                    if (cam->isStaticIPactive())
                    {
                        std::cout << std::setw(13) << "Yes";
                    }
                    else
                    {
                        std::cout << std::setw(13) << "No";
                    }

                    break;
                }
                case 'M':
                {
                    std::cout << std::setw(21) << cam->getMAC();
                    break;
                }
                case 'r':
                {
                    if (cam->isReachable())
                    {
                        std::cout << std::setw(13) << "Yes";
                    }
                    else
                    {
                        std::cout << std::setw(13) << "No";
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        std::cout << std::endl;
    }
}


std::shared_ptr<tis::Camera> findCamera(const std::string& ident)
{
    tis::camera_list cameras = get_camera_list();

    if (cameras.size() == 0)
    {
        throw std::runtime_error("\nNo cameras found.\n");
    }

    std::shared_ptr<tis::Camera> retv = tis::getCameraFromList(cameras, ident, tis::CAMERA_SERIAL);

    if (!retv)
    {
        retv = tis::getCameraFromList(cameras, ident, tis::CAMERA_NAME);
    }
    if (!retv)
    {
        retv = tis::getCameraFromList(cameras, ident, tis::CAMERA_MAC);
    }
    if (!retv)
    {
        retv = tis::getCameraFromList(cameras, ident, tis::CAMERA_IP);
    }

    if (!retv)
    {
        std::cerr << "Unable to find camera with identifier \"" << ident << "\"" << std::endl;
    }

    return retv;
}


std::string get_camera_ident (const CLI::App& app)
{
    if (app.remaining_size() == 0)
    {
        std::cerr << "tcam-gigetool requires camera identifier!" << std::endl
                  << app.help();
    }
    else if (app.remaining_size() > 1)
    {
        std::cerr << "Too many arguments as potential camera identifier." << std::endl
                  << "Arguments are: ";
        std::string rem_str;

        for (const auto& e : app.remaining())
        {
            rem_str += e + " ";
        }
        std::cerr << rem_str << std::endl;

        return std::string();
    }

    return app.remaining().at(0);
}


int print_camera_information (const CLI::App& app)
{
    auto ident = get_camera_ident(app);

    if (ident.empty())
    {
        return 1;
    }

    auto camera = findCamera(ident);

    if (camera == NULL)
    {
        std::cout << "No camera found." << std::endl;
        return 1;
    }

    bool reachable = false;
    if (!camera->isReachable())
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
              << "\nSerial:   " << camera->getSerialNumber();
    if (reachable)
    {
        std::cout << "\nFirmware: " << camera->getFirmwareVersion()
                  << "\nUserName: " << camera->getUserDefinedName();
    }
    std::cout << "\n\nMAC Address:        " << camera->getMAC()
              << "\nCurrent IP:         " << camera->getCurrentIP()
              << "\nCurrent Subnet:     " << camera->getCurrentSubnet()
              << "\nCurrent Gateway:    " << camera->getCurrentGateway()
              << "\n\nDHCP is:   " << (camera->isDHCPactive() ? "enabled" : "disabled")
              << "\nStatic is: " << (camera->isStaticIPactive() ? "enabled" : "disabled")
              << std::endl;

    if (reachable)
    {
        std::cout << "\n\nPersistent IP:      " << camera->getPersistentIP()
                  << "\nPersistent Subnet:  " << camera->getPersistentSubnet()
                  << "\nPersistent Gateway: " << camera->getPersistentGateway() << "\n"
                  << std::endl;
    }
    return 0;
}



int set_camera_options(const CLI::App& app)
{
    auto ident = get_camera_ident(app);

    auto camera = findCamera(ident);

    if (!camera)
    {
        return 1;
    }

    auto name = app.get_option("--name");
    if (*name)
    {
        std::string name_str;
        name->results(name_str);
        camera->setUserDefinedName(name_str);
    }

    std::string ip_str;
    std::string nm_str;
    std::string gw_str;

    auto ip = app.get_option("--ip");
    if (*ip)
    {
        ip->results(ip_str);
    }

    auto netmask = app.get_option("--netmask");
    if (*netmask)
    {
        netmask->results(nm_str);
    }

    auto gateway = app.get_option("--gateway");
    if (*gateway)
    {
        gateway->results(gw_str);
    }

    // if no further settings are to be applied, exit
    if (!*ip && !*netmask && !*gateway)
    {
        return 0;
    }

    std::string err;
    if (tis::verifySettings(ip_str, nm_str, gw_str, err))
    {
        camera->setPersistentIP(ip_str);
        camera->setPersistentSubnet(nm_str);
        camera->setPersistentGateway(gw_str);
    }
    else
    {
        std::cerr << err << std::endl;
        return 1;
    }

    auto mode = app.get_option("--mode");
    if (*mode)
    {
        IP_Mode m;
        mode->results(m);

        switch(m)
        {
            case IP_Mode::DHCP:
            {
                camera->setIPconfigState(true, false);
                break;
            }
            case IP_Mode::Static:
            {
                camera->setIPconfigState(false, true);
                break;

            }
            case IP_Mode::LinkLocal:
            {
                camera->setIPconfigState(false, false);
                break;
            }
        }

    }
    return 0;
}


int execute_rescue(const CLI::App& app)
{

    auto ident = get_camera_ident(app);

    if (ident.empty())
    {
        return 1;
    }

    auto camera = findCamera(ident);

    std::string mac;

    if (!camera)
    {
        if (!tis::isValidMAC(ident))
        {
            std::cerr << "Given identifier could not be associated with a camera, nor is it a valid MAC address" << std::endl;
            return 1;
        }
        mac = ident;
    }
    else
    {
        mac = camera->getMAC();
    }

    std::string ip_str;
    std::string nm_str;
    std::string gw_str;

    auto ip = app.get_option("--ip");
    if (*ip)
    {
        ip->results(ip_str);
        std::cout << "Setting ip to: " << ip_str <<std::endl;
    }

    auto netmask = app.get_option("--netmask");
    if (*netmask)
    {
        netmask->results(nm_str);
        std::cout << "Setting netmask to: " << nm_str <<std::endl;
    }

    auto gateway = app.get_option("--gateway");
    if (*gateway)
    {
        gateway->results(gw_str);
        std::cout << "Setting gw to: " << gw_str <<std::endl;
    }

    auto send = [=] ()
    {
        try
        {
            tis::sendIpRecovery(mac, tis::ip2int(ip_str), tis::ip2int(nm_str), tis::ip2int(gw_str));
        }
        catch (...)
        {
            std::cerr << "An Error occured while sending the rescue packet." << std::endl;
        }

    };

    std::string err;
    if (tis::verifySettings(ip_str, nm_str, gw_str, err))
    {
        send();
    }
    else
    {
        auto yes = app.get_option("--yes");

        if (yes)
        {
            send();
        }
        else
        {
            std::cout << "Settings verification failed." << std::endl
                      << "Reason: " << err << std::endl
                      << "To prevent this warning add `--yes` to the command."
                      << "Do you really want to proceed(y/N)? ";

            std::string really;
            std::cin >> really;
            if (really.compare("y") != 0)
            {
                return 2;
            }
            else
            {
                send();
            }
        }
    }
    return 0;
}


int execute_upload (const CLI::App& app)
{
    auto ident = get_camera_ident(app);

    auto camera = findCamera(ident);

    if (!camera)
    {
        return 1;
    }

    if (!camera->isReachable())
    {
        std::cerr << "Camera is not reachable" << std::endl;
        return 1;
    }

    std::string firmware_file;

    auto firmware = app.get_option("--file");
    if (*firmware)
    {
        firmware->results(firmware_file);
    }

    std::string override_model_name = "";

    auto model_name = app.get_option("--overrideModelName");
    if (*model_name)
    {
        model_name->results(override_model_name);
    }

    bool assume_yes = false;

    auto yes = app.get_option("--yes");
    if (*yes)
    {
        yes->results(assume_yes);
    }

    if (!assume_yes)
    {
        std::cout << "\n!!! IMPORTANT NOTE !!!\n"
            "Do not interrupt the firmware update process.\n"
            "Do not disconnect the camera during the firmware update process.\n"
            "A failed firmware update may render the camera unusable.\n\n"
            "Start the update process [y/N]";

        std::string really;
        std::getline(std::cin, really);
        if (really.compare("y") != 0)
        {
            return 2;
        }
    }

    auto func = [](int progress, const std::string& s) {
        std::cout << "\r";

        std::cout << std::setw(5) << progress << "%";
        if (s != "")
        {
            std::cout << std::setw(40) << s;
        }
        std::cout.flush();
    };

    std::cout << std::endl;
    int ret = camera->uploadFirmware(firmware_file, override_model_name, func);
    if (ret >= 0)
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

    return 0;
}


void batch_rescue(tis::camera_list& cameras, const std::string& baseip)
{
    std::string netmask_str = "255.255.255.0";
    std::string gateway_str = "0.0.0.0";

    ip4_address_t ip = tis::ip2int(baseip);
    ip4_address_t netmask = tis::ip2int(netmask_str);
    ip4_address_t gateway = tis::ip2int(gateway_str);

    for (auto& camera : cameras)
    {
        tis::sendIpRecovery(camera->getMAC(), ip, netmask, gateway);

        // this increments the IP by 1
        ip = ntohl(ip);
        ip += 1;
        ip = htonl(ip);
    }
}


int execute_batch_upload (const CLI::App& app)
{
    std::string base_address;
    auto ba = app.get_option("--baseaddress");
    if (*ba)
    {
        ba->results(base_address);
    }

    std::string firmware_file;
    auto firmware = app.get_option("--file");
    if (*firmware)
    {
        firmware->results(firmware_file);
    }

    std::string interface;
    auto inter = app.get_option("--interface");
    {
        inter->results(interface);
    }

    // helper function
    // check if camera is on the correct interface
    // and if it is usable
    auto get_interface_cameras = [] (const std::string& itf)
    {
        auto all_cameras = get_camera_list();

        tis::camera_list cameras;

        for (auto cam : all_cameras)
        {
            if (cam->getInterfaceName() == itf)
            {
                if (cam->isReachable())
                {
                    if (cam->getIsBusy())
                    {
                        std::cerr << "The camera "
                                  << cam->getSerialNumber()
                                  << " is busy. Aborting."
                                  << std::endl;
                        return tis::camera_list();
                    }
                }
            }
            cameras.push_back(cam);
        }
        return cameras;
    };


    auto cameras = get_interface_cameras(interface);

    if (cameras.empty())
    {
        std::cout << "No cameras found." << std::endl;
        return 0;
    }

    if (app.get_option("--reconfigure"))
    {
        batch_rescue(cameras, base_address);
        sleep(1);
        cameras = get_interface_cameras(interface);
    }

    std::cout << "Uploading to " << cameras.size() << " cameras..." << std::endl;

    for (auto& camera : cameras)
    {
        std::string serial = camera->getSerialNumber();
        auto func = [&serial] (int progress, const std::string& s) {
            std::cout << "\r";

            std::cout << std::setw(10) << serial << std::setw(5) << progress << "%";
            if (s != "")
            {
                std::cout << std::setw(40) << s;
            }
            std::cout.flush();
        };

        std::cout << std::endl;
        int ret = camera->uploadFirmware(firmware_file, "", func);
        if (ret < 0)
        {
            std::cerr << "Error while uploading firmware to " << serial << std::endl;
        }
    }

    std::cout << "Done." << std::endl;
    return 0;
}


int execute_check_control(const CLI::App& app)
{
    auto ident = get_camera_ident(app);
    auto camera = findCamera(ident);
    if (!camera)
    {
        return 1;
    }

    if (!camera->isReachable())
    {
        std::cerr << "Camera is not reachable" << std::endl;
        return 1;
    }
    if (!camera->getIsBusy())
    {
        std::cout << "Camera is not controlled by anyone." << std::endl;
        return 0;
    }

    static constexpr int GEV_PRIMARY_APPLICATION_PORT_REGISTER = 0x0A04;
    static constexpr int GEV_PRIMARY_APPLICATION_IP_ADDRESS_REGISTER = 0x0A14;
    static constexpr int GEV_HEARTBEAT_TIMEOUT_REGISTER = 0x0938;
    uint32_t port;
    uint32_t address;
    uint32_t timeout;
    if (!camera->sendReadRegister(GEV_PRIMARY_APPLICATION_IP_ADDRESS_REGISTER, &address))
    {
        std::cerr << "Unable to read address register from device." << std::endl;
        return 2;
    }
    if (!camera->sendReadRegister(GEV_PRIMARY_APPLICATION_PORT_REGISTER, &port))
    {
        std::cerr << "Unable to read port register from device." << std::endl;
        return 2;
    }
    if (!camera->sendReadRegister(GEV_HEARTBEAT_TIMEOUT_REGISTER, &timeout))
    {
        std::cerr << "Unable to read timeout register from device." << std::endl;
        return 2;
    }

    // hacky fix
    // many firmware versions return a wrong value for the port information
    // the value might require an additional conversions
    // check if valid and fix otherwise
    if (port > 65535)
    {
        port = ntohs(ntohl(port));
    }

    std::cout << "Controlling IP: " << tis::int2ip(ntohl(address)) << ":" << port << std::endl
        << "Heartbeat Duration: " << timeout << " µs" << std::endl;
    return 0;
}


// all allowed characters
static const std::string list_format_chars = "msuingINGfdSMr";

// validator for possible --format characters
struct ListValidator : public CLI::Validator
{
    ListValidator()
    {
        name_ = "CAMERALIST";
        func_ = [] (const std::string &str)
        {
            for (unsigned int i = 0; i < str.length(); ++i)
            {
                if (list_format_chars.find(str.at(i)) == std::string::npos)
                {
                    auto ret = std::string("Character '");
                    ret += str.at(i);
                    ret += "' not supported";
                    return ret;
                }
            }

            return std::string();
        };
    }
};
const static ListValidator list_validator;
} // namespace

int main(int argc, char* argv[])
{
    //
    // the implementation of this tool
    // relies heavily on CLI11
    // we pass the CLI::App of the subcommand
    // to the associated function and let it
    // figure out the rest
    CLI::App app { "The Imaging Source Gigabit Ethernet camera configuration tool" };
    app.allow_extras();

    auto app_list = app.add_subcommand("list", "list connected cameras");

    // default string for --format
    std::string list_format = "msui";

    auto app_list_format = app_list->add_option("--format", list_format, "Format of the output list.\n"
                                                "Options:\n"
                                                "m - camera model\n"
                                                "s - camera serial\n"
                                                "u - user defined name\n"
                                                "i - current ip address\n"
                                                "n - current netmask\n"
                                                "g - current gateway\n"
                                                "I - persistent ip\n"
                                                "N - persistent netmask\n"
                                                "G - persistent gateway\n"
                                                "f - local network interface\n"
                                                "d - is dhcp enabled\n"
                                                "S - is static ip enabled\n"
                                                "M - mac address\n"
                                                "r - is reachable\n", true);

    app_list_format->check(list_validator);

    auto app_info = app.add_subcommand("info", "show information about a specific camera");

    auto app_set = app.add_subcommand("set", "set configuration for camera");
    auto set_ip = app_set->add_option("--ip", "IPv4 address to be set")->check(CLI::ValidIPV4);
    auto set_nm = app_set->add_option("--netmask", "IPv4 netmask to be set")->check(CLI::ValidIPV4);
    auto set_gw = app_set->add_option("--gateway", "IPv4 gateway address to be set")->check(CLI::ValidIPV4);
    app_set->add_option("--mode", "IP configuration mode")->transform(CLI::CheckedTransformer(ip_mode_map, CLI::ignore_case));
    app_set->add_option("--name", "user defined name for the camera");

    set_ip->needs(set_nm, set_gw);
    set_nm->needs(set_ip, set_gw);
    set_gw->needs(set_ip, set_nm);

    auto app_rescue = app.add_subcommand("rescue", "temporarily set IP configuration on the camera");
    app_rescue->add_option("--ip", "IPv4 address to be set")->check(CLI::ValidIPV4)->required();
    app_rescue->add_option("--netmask", "IPv4 netmask to be set")->check(CLI::ValidIPV4)->required();
    app_rescue->add_option("--gateway", "IPv4 gateway address to be set")->check(CLI::ValidIPV4)->required();
    app_rescue->add_flag("--yes", "Assume user says yes to actions");

    auto app_fw = app.add_subcommand("upload", "upload firmware to camera");
    app_fw->add_option("--file", "Firmware file to use")->check(CLI::ExistingFile)->required();
    // group hidden causes the option to not be listed in help
    // this option can break a camera, do not advertise it
    app_fw->add_option("-o,--overrideModelName", "Overwrite model name")->group("");
    app_fw->add_flag("--yes", "Assume user says yes to actions");

    auto app_batch_fw = app.add_subcommand("batchupload", "upload firmware to camera");
    app_batch_fw->add_option("--file", "Firmware file to use")->check(CLI::ExistingFile)->required();
    app_batch_fw->add_option("-b,--baseaddress", "Firmware file to use")->check(CLI::ExistingFile)->required();

    auto check_control = app.add_subcommand("check-control", "find IP of controlling PC");

    app.require_subcommand();
    // CLI11 uses "TEXT" as a filler for the option string arguments
    // replace it with "SERIAL" to make the help text more intuitive.
    app.get_formatter()->label("TEXT", "IDENTIFIER");
    // list does not have an identifier
    app_list->get_formatter()->label("TEXT", "list-fields");


    CLI11_PARSE(app, argc, argv);


    if (*app_list)
    {
        list_cameras(list_format);
        return 0;
    }

    if (*app_info)
    {
        return print_camera_information(*app_info);
    }
    else if (*app_set)
    {
        return set_camera_options(*app_set);
    }
    else if (*app_rescue)
    {
        return execute_rescue(*app_rescue);
    }
    else if (*app_fw)
    {
        return execute_upload(*app_fw);
    }
    else if (*app_batch_fw)
    {
        return execute_batch_upload(*app_batch_fw);
    }
    else if (*check_control)
    {
        return execute_check_control(*check_control);
    }

    return 0;
}
