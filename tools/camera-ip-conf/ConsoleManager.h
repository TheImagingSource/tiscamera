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

#ifndef _CONSOLE_MANAGER_H_
#define _CONSOLE_MANAGER_H_

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <memory>
#include "Camera.h"

namespace tis
{
    /// @name getArgumentValue
    /// @param args - vector that shall be searched
    /// @param long_name - long ident of wanted argument; empty if not wanted
    /// @param short_name - short ident of wanted argument; empty if not wanted
    /// @return std::string containing the value; empty if none found
    std::string getArgumentValue (const std::vector<std::string>& args, const std::string& long_name, const std::string& short_name);

    /// @name getCameraList
    /// @return vector containing all found cameras
    camera_list getCameraList ();

    /// @name findCamera
    /// @param args - vector that shall be searched
    /// @return shared_ptr to found camera; empty if non found
    /// @searches args for camera identifier and queries for a corresponding Camera instance
    std::shared_ptr<Camera> findCamera (const std::vector<std::string>& args);

    /// @name listCameras
    /// prints overview over detected cameras
    void listCameras ();

    /// @name printCameraInformation
    /// @param args - vector containing camera serial for which information shall be printed
    /// prints information about camera
    void printCameraInformation (const std::vector<std::string>& args);

    /// @name writeChanges
    /// @param camera - camera that shall be written
    /// @param ip - ip the camera shall use
    /// @param subnet - subnet the camera shall use
    /// @param gateway - gateway the camera shall use
    /// @brief writes given arguments in given camera
    void writeChanges (std::shared_ptr<Camera> camera, const std::string ip, const std::string subnet, const std::string gateway);

    /// @name isAccessible
    /// @param args - vector containing camera identification options
    /// Tests whether a camera could be accessed, ie. whether write/read operations will
    /// succeed.
    int isAccessible (const std::vector<std::string>& args);

    int isAccessible (std::shared_ptr<Camera> camera);

    /// @name setCamera
    /// @param args - vector containing string that shall be searched
    /// sets the values described in args for described camera
    void setCamera (const std::vector<std::string>& args);

    /// @name forceIP
    /// @param args - vector containing camera and ip information
    /// sends forced ip configuration to described camera
    void forceIP (const std::vector<std::string>& args);

    /// @name upgradeFirmware
    /// @param args - vector containing camera and filepath to firmware
    /// checks for newer firmware in given file and uploads it to camera
    void upgradeFirmware (const std::vector<std::string>& args);

    void rescue (std::vector<std::string> args);

} /* namespace tis */

#endif /* _CONSOLE_MANAGER_H_ */
