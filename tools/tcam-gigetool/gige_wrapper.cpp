#include "CameraDiscovery.h"
#include "Camera.h"
#include "utils.h"
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <assert.h>

#include <thread>
#include <mutex>
#include <exception>

#include "gige_wrapper.h"

#define MAX_CAMERAS 64

#define COPY_STRING(x,y) {strncpy(x, y, sizeof(x)-1);}

using namespace tis;

struct g_camera_info
{
    camera_list cameras;
};

struct g_camera_info *g_camera_info = NULL;

void init(void)
{
    if (g_camera_info)
        return;
    g_camera_info = new(struct g_camera_info);
}

void copy_to_tcam(struct tcam_camera *tcam, std::shared_ptr<Camera> camera, int get_persistent_values)
{
    memset((void*)tcam, 0, sizeof(tcam));
    COPY_STRING(tcam->model_name, camera->getModelName().c_str());
    COPY_STRING(tcam->serial_number, camera->getSerialNumber().c_str());
    COPY_STRING(tcam->current_ip, camera->getCurrentIP().c_str());
    COPY_STRING(tcam->current_netmask, camera->getCurrentSubnet().c_str());
    COPY_STRING(tcam->current_gateway, camera->getCurrentGateway().c_str());
    COPY_STRING(tcam->interface_name, camera->getNetworkInterfaceName().c_str());
    COPY_STRING(tcam->mac_address, camera->getMAC().c_str());
    tcam->is_reachable = camera->isReachable();
    if (tcam->is_reachable && get_persistent_values){
        COPY_STRING(tcam->persistent_ip, camera->getPersistentIP().c_str());
        COPY_STRING(tcam->persistent_netmask, camera->getPersistentSubnet().c_str());
        COPY_STRING(tcam->persistent_gateway, camera->getPersistentGateway().c_str());
        COPY_STRING(tcam->user_defined_name, camera->getUserDefinedName().c_str());
        COPY_STRING(tcam->firmware_version, camera->getFirmwareVersion().c_str());
        tcam->is_static_ip = camera->isStaticIPactive();
        tcam->is_dhcp_enabled = camera->isDHCPactive();
        tcam->is_busy = camera->getIsBusy();
    }
}

int get_camera_list(discover_callback_t callback, int get_persistent_values)
{
    if (!g_camera_info)
        abort();
    camera_list cameras;
    std::mutex cam_lock;

    std::function<void(std::shared_ptr<Camera>)> f = [&cam_lock, &cameras] (std::shared_ptr<Camera> camera)
    {
        std::lock_guard<std::mutex> mutex_lock(cam_lock);
        cameras.push_back(camera);
    };

    discoverCameras(f);
    for (const auto& camera : cameras)
    {
        struct tcam_camera tcam = {0};
        copy_to_tcam(&tcam, camera, get_persistent_values);
        callback(tcam);
    }

    g_camera_info->cameras = cameras;

    return 0;
}

int get_camera_details(char *identifier, struct tcam_camera *tcam)
{
    assert(g_camera_info);
    std::shared_ptr<Camera> camera;

    camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_SERIAL);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_NAME);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_MAC);

    if (!camera)
        return NO_DEVICE;

    copy_to_tcam(tcam, camera, 1);

    return 0;
}

int set_persistent_parameter_s(char *identifier, char *key, char *value)
{
    assert(g_camera_info);
    std::shared_ptr<Camera> camera;
    int ret = 0;

    camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_SERIAL);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_NAME);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_MAC);

    if (!camera)
        return NO_DEVICE;

    if(!strcmp(key, "ip"))
    {
        ret = camera->setPersistentIP(value);
    }
    else if (!strcmp(key, "gateway"))
    {
        ret = camera->setPersistentGateway(value);
    }
    else if (!strcmp(key, "netmask"))
    {
        ret = camera->setPersistentSubnet(value);
    }
    else if (!strcmp(key, "name"))
    {
        ret = camera->setUserDefinedName(value);
    }
    else
    {
        return INVALID_PARAMETER;
    }

    return ret != 0 ? SUCCESS : FAILURE;
}

int set_persistent_parameter_i(char *identifier, char *key, int value)
{
    assert(g_camera_info);
    std::shared_ptr<Camera> camera;
    int ret = 0;

    camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_SERIAL);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_NAME);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_MAC);

    if (!camera)
        return NO_DEVICE;

    if(!strcmp(key, "dhcp"))
    {
        ret = camera->setDHCPstate(value != 0);
    }
    else if (!strcmp(key, "static"))
    {
        ret = camera->setStaticIPstate(value != 0);
    }
    else{
        return INVALID_PARAMETER;
    }

    return ret != 0 ? SUCCESS : FAILURE;
}

int upload_firmware(char *identifier, char *path, upload_callback_t callback)
{
    assert(g_camera_info);
    std::shared_ptr<Camera> camera;

    camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_SERIAL);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_NAME);
    if (!camera)
        camera = getCameraFromList(g_camera_info->cameras, identifier, CAMERA_MAC);

    if (!camera)
        return NO_DEVICE;

    auto func = [callback] (int progress,const std::string& s)
    {
        callback(s.c_str(), progress);
    };

    int ret = (int)camera->uploadFirmware(path, "", func);

    return ret;
}

std::string string_format(const std::string &fmt, ...) {
    int size=1;
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

int rescue(char *mac, char* ip, char *netmask, char *gateway)
{
    int ret = -1;

    try
    {
        sendIpRecovery(mac, ip2int(ip), ip2int(netmask), ip2int(gateway));
        ret = 0;
    }
    catch (...)
    {
    }

    return ret;
}