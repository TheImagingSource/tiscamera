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

#pragma once

#include <functional>
#include <string>
#include <memory>

#include "gigevision.h"


typedef unsigned char byte;

typedef std::function<void(int, const std::string&)> tReportProgressFunc;

namespace FirmwareUpdate
{

/// @class IFirmwareWriter
/// @brief base class for actual firmware writer
class IFirmwareWriter
{
public:
    virtual ~IFirmwareWriter() {};

    /// @name write
    /// @param addr - adddress that shall be written
    /// @param pData - data that shall be written
    /// @param data_size - size of pData
    /// @param timeout_in_ms - maximum waiting time
    /// @return true on success
    /// @brief pure virtual function to connect firmware functions with writing methods of device
    virtual bool write (uint32_t addr, void* pData, size_t data_size, unsigned int timeout_in_ms = 2000) = 0;

    /// @name write
    /// @param addr - adddress that shall be written
    /// @param val - data that shall be written
    /// @param timeout_in_ms - maximum waiting time
    /// @return true on success
    /// @brief pure virtual function to connect firmware functions with writing methods of device
    virtual bool write (uint32_t addr, uint32_t val, unsigned int timeout_in_ms = 2000) = 0;

    /// @name read
    /// @param addr - address that shall be read
    /// @param val - variable that will be filled with read value
    /// @param timeout_in_ms - maximum waiting time
    /// @return true on success
    /// @brief pure virtual function to connect firmware functions with reading methods of device
    virtual bool read (uint32_t addr, uint32_t& val, unsigned int timeout_in_ms = 2000) = 0;

    /// @name read
    /// @param addr - address that shall be read
    /// @param data_size - size of data that shal lbe read
    /// @param pData - variable that will be filled with read value
    /// @param read_count
    /// @param timeout_in_ms - maximum waiting time
    /// @return true on success
    /// @brief pure virtual function to connect firmware functions with reading methods of device
    virtual bool read (uint32_t addr, unsigned int data_size, void* pData, unsigned int& read_count, unsigned int timeout_in_ms = 2000) = 0;

}; /* class IFirmwareWriter */


enum class Status
{
    Success						=  0,
		SuccessDisconnectRequired	=  1,
		SuccessNoActionRequired		=  2,
		DeviceNotRecognized			= -1,	// Unable to upgrade firmware for the selected device
		DeviceSupportsFwOnly		= -2,	// The device only accepts plain *.fw files (Blackfin-devices),
    // but a firmware package file was supplied
		InvalidFile					= -3,	// The file is invalid (bad length or content)
		NoMatchFoundInPackage		= -4,	// The firmware package file did not contain a matching firmware
		WriteError					= -5,	// The firmware could not be written
		WriteVerificationError		= -6,	// An error occurred while writing the firmware,
    // the device is probably in a "broken" state
		DeviceAccessFailed			= -7,	// Failed to lock the device
		MotorFirmwareUpdateFailed	= -8,	// Failed to update the motor controller firmware
		FocusTableUpdateFailed		= -9,	// Failed to update the motor controller's focus table

		MachXO2UpdateFailed			= -10,	// Failed to update the MachXO2 FPGA configuration
        };

inline bool succeeded (Status status) { return (int)status >= 0; }
inline bool failed (Status status) { return !succeeded(status); }

/// @name upgradeFirmware
/// @param dev -
/// @param disc - discovery acknowledge packet describing the camera
/// @param fileName - firmware file to use
/// @param progressFunc - callback function for current progress; int the current percentage
/// @return Status  describing the success of the upgrade
Status upgradeFirmware (IFirmwareWriter& dev,
                        const Packet::ACK_DISCOVERY& disc,
                        const std::string& fileName,
                        const std::string& overrideModelName,
                        std::function<void(int,const std::string&)> progressFunc);

} /* namespace FirmwareUpdate */
