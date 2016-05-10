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

    /// @name discoverCameras
    /// @param selectected_interfaces - vector of interface names that shall be queried
    /// @param discover_call - function to call on discovery of a camera
    void discoverCameras (std::vector<std::string> selectected_interfaces,
                          std::function<void (std::shared_ptr<Camera>)> const & discover_call);

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
