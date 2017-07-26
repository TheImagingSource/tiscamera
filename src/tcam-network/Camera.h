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

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "gigevision.h"
#include "NetworkInterface.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace tis
{
    class Camera;
    typedef std::vector<std::shared_ptr<Camera>> camera_list;

    enum camera_ident
    {
        CAMERA_NAME = 0,
        CAMERA_SERIAL,
        CAMERA_MAC
    };

    /// @name getCameraFromList
    /// @param cameras - vector containing cameras that shall be searched
    /// @param identifier - string containing the serial of the wished camera
    /// @param id_type - camera_ident describing the identifier; default CAMERA_SERIAL
    /// @return shared pointer to the wished camera, else null
    std::shared_ptr<Camera> getCameraFromList (const camera_list cameras, const std::string& identifier, camera_ident id_type = CAMERA_SERIAL);

    const int PACKET_RETRY_COUNT = 5;
class Camera
{
private:

    /// received acknowledge packet
    Packet::ACK_DISCOVERY packet;

    /// interface used for communication
    std::shared_ptr<NetworkInterface> interface;

    /// socket used for communication
    std::shared_ptr<Socket> socket;

    /// unique paket id
    unsigned short requestID;

    // flag if we have control over camera
    bool isControlled;

    // current value
    int timeoutCounter;

    // default value from constructor
    int timeoutCounterDefault;

public:

    Camera (const Packet::ACK_DISCOVERY& packet, std::shared_ptr<NetworkInterface> _interface, int timeoutIntervals = 3);

    /// copy constructor
    Camera (const Camera& _camera) = delete;
    Camera& operator=(const Camera& ) = delete;

    ~Camera();

    /// @name getSocket
    /// @return Socket object from Camera
    std::shared_ptr<Socket> getSocket ();

    /// @name generateRequestID
    /// @return returns a new request id
    int generateRequestID ();

    /// @name reduceCounter
    /// @return new value of counter
    int reduceCounter ();

    /// @name resetCounter
    /// @return new value of value
    /// @brief resets counter to initial value
    int resetCounter ();

    std::string getNetworkInterfaceName ();

    /// @name updateCamera
    /// @param cam - new camera instance
    /// @brief updates this camera with the information given by cam
    void updateCamera (std::shared_ptr<Camera> cam);

    /// @name getModelName
    /// @return string containing the model name
    std::string getModelName ();

    /// @name getSerialNumber
    /// @return string containing the unique serial number
    std::string getSerialNumber ();

    /// @name getVendorName
    /// @return string containing the vendor description
    std::string getVendorName ();

    /// @name getUserDefinedName
    /// @return string containing the user defined name
    std::string getUserDefinedName ();

    /// @name setUserDefinedName
    /// @param name - string containing name to be used; 16 chars maximum
    /// @return true on success
    bool setUserDefinedName (const std::string& name);

    /// @name getMAC
    /// @return string containing the mac address in format XX:XX:XX:XX:XX:XX
    const std::string getMAC ();

    const std::string getCurrentIP ();
    const std::string getCurrentSubnet ();
    const std::string getCurrentGateway ();

    bool isStaticIPactive ();
    bool setStaticIPstate (const bool on);
    bool isDHCPactive ();
    bool setDHCPstate (const bool on);

    /// @name setIPconfigState
    /// @param dhcp - true if dhcp shall be active
    /// @param staticIP - true is static IP shall be active
    /// @return true on success
    /// @brief Sets the ip configuration according to the given flags
    bool setIPconfigState (const bool dhcp, const bool staticIP);

    const std::string getPersistentIP ();
    bool setPersistentIP (const std::string& ip);
    const std::string getPersistentSubnet ();
    bool setPersistentSubnet (const std::string& ip);
    const std::string getPersistentGateway ();
    bool setPersistentGateway (const std::string& ip);

    /// @name forceIP
    /// @param ip - address to be used
    /// @param subnet - subnet address to be used
    /// @param gateway - gateway to be used
    /// @return true on success
    bool forceIP (const std::string& ip, const std::string& subnet, const std::string& gateway);

    /// @name getFirmwareVersion
    /// @return string containing information about current firmware
    std::string getFirmwareVersion ();

    /// @name uploadFirmware
    /// @param filename - string containing the location of the firmware that shall be used
    /// @param overrideModelName - string containing the model name that shall be used. empty on default
    /// @param progressFunc callback function to inform over progress
    /// @return true on success
    int uploadFirmware (const std::string& filename,
                        const std::string& overrideModelName,
                        std::function<void(int, const std::string&)> progressFunc);

    /// @name getInterfaceName
    /// @return name of interface used for communication
    std::string getInterfaceName ();

    /// checks if camera is reachable from interface
    bool isReachable();

    /// @name resetIP
    /// @brief makes camera re-run its ipconfiguration cycle
    void resetIP ();

    /// @name getControl
    /// @brief sets control bits on camera to assure exclusive control
    /// @return true on success
    bool getControl();

    /// @name abandonControl
    /// @brief sets control bits on camera to give up exclusive control
    /// @return true on success
    bool abandonControl();

    /// @name isBusy
    /// @brief returns true if camera is controlled by another application
    /// @return true if busy
    ///
    /// !!! The return value is for informational purposes only since the
    /// camera could be locked by another application immediately after this
    /// call returns
    bool getIsBusy();

    /// @name sendReadMemory
    /// @param address - address that shall be read
    /// @param size - size of memory to read
    /// @param data - pointer to container that shall be filled
    /// @return true on success
    /// @brief Sends request for register and fills data with value
    bool sendReadMemory (const uint32_t address, const uint32_t size, void* data);

    /// @name sendWriteMemory
    /// @param address - memory address to be written
    /// @param size - size of data that shall be written
    /// @param data - pointer to information that shall be written
    /// @return int containing the return value of write attempt
    bool sendWriteMemory (const uint32_t address, const size_t size, void* data);

private:

    /// @name sendForceIP
    /// @param ip - ip address camera shall use
    /// @param netmask - netmask camera shall use
    /// @param gateway - gateway camera shall use
    void sendForceIP (const uint32_t ip, const uint32_t subnet, const uint32_t gateway);

};

} /* namespace tis_network */

#endif /* _CAMERA_H_ */
