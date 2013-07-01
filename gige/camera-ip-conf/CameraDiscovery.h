///
/// @file CameraDiscovery.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///
/// @brief handles network tasks
///

#ifndef _CAMERA_DISCOVERY_H_
#define _CAMERA_DISCOVERY_H_

#include "Camera.h"
#include "NetworkInterface.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace tis
{
    /// @name detectInterfaces
    /// @return vector containing all usable network interfaces and a corresponding socket; empty on error
    std::vector<std::shared_ptr<NetworkInterface>> detectNetworkInterfaces ();

    /// @name discoverCameras
    /// @param discover_call - function to call on discovery of a camera
    void discoverCameras (const std::function<void(std::shared_ptr<Camera>)> &discover_call);

    /// @name
    /// @param interface - object describing the interface that shall be pinged
    /// @param discover_call - function to call on discovery of a camera
    void sendDiscovery (std::shared_ptr<NetworkInterface> interface, const std::function<void(std::shared_ptr<Camera>)> &discover_call);

    /// @name sendRescue
    /// @param mac - MAC address of the camera
    /// @brief sends forceIP on all interfaces for the wished ip
    void sendIpRecovery (const std::string mac, const uint32_t ip, const uint32_t netmask, const uint32_t gateway);

}
#endif /* _CAMERA_DISCOVERY_H_ */
