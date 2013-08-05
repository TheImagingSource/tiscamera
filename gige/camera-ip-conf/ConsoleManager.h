///
/// @file ConsoleManager.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///
/// @brief provides functions for display camera information
///

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
    /// @name getArgument
    /// @param args - vector containing string that shall be searched
    /// @param argument - string defining what shall be searched
    /// @return value of argument or string.empty()
    /// searches through args for string containing argument and returns x
    /// argument/value pairs are expected to be argument=x
    std::string getArgument (const std::vector<std::string>& args, const std::string& argument);

    /// @name getSerialFromArgs
    /// @param args - args - vector containing string with camera serial
    /// searches for -c and assumes the following element is serial
    std::string getSerialFromArgs (const std::vector<std::string>& args);

    /// @name getCameraList
    /// @return vector containing all found cameras
    camera_list getCameraList ();

    /// @name listCameras
    /// prints overview over detected cameras
    void listCameras ();

    /// @name printCameraInformation
    /// @param args - vector containing camera serial for which information shall be printed
    /// prints information about camera
    void printCameraInformation (const std::vector<std::string>& args);

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
