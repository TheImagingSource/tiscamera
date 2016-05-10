/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include <string>
#include <map>
#include <cstdint>
#include <functional>
#include <vector>

#include "I2CDevice.h"

namespace MachXO2
{
class JedecFile;

enum class DeviceType
{
    MachXO2_256, /**< XO2 256 LUT */
    MachXO2_640, /**< XO2 640 LUT */
    MachXO2_640U, /**< XO2 640U LUT with larger UFM size */
    MachXO2_1200, /**< XO2 1200 LUT */
    MachXO2_1200U, /**< XO2 1200 LUT with larger UFM size */
    MachXO2_2000, /**< XO2 2000 LUT */
    MachXO2_2000U, /**< XO2 2000 LUT with larger UFM size */
    MachXO2_4000, /**< XO2 4000 LUT device */
    MachXO2_7000, /**< XO2 7000 LUT device */
    MachXO2_Unknown,
};


namespace Commands
{

const uint8_t IDCODE_PUB[] = { 0xE0, 0x00, 0x00, 0x00 };
const uint8_t ISC_ENABLE_X[] = { 0x74, 0x08, 0x00 };
const uint8_t ERASE_FLASH[] = { 0x0E, 0x04, 0x00, 0x00 };
const uint8_t CHECK_BUSY[] = { 0xF0, 0x00, 0x00, 0x00 };
const uint8_t READ_STATUS[] = { 0x3C, 0x00, 0x00, 0x00 };
const uint8_t INIT_ADDRESS[] = { 0x46, 0x00, 0x00, 0x00 };
const uint8_t READ_ONE_PAGE[] = { 0x73, 0x00, 0x00, 0x01 };
const uint8_t READ_32_PAGES[] = { 0x73, 0x00, 0x00, 0x21 };
const uint8_t PROG_ONE_PAGE[] = { 0x70, 0x00, 0x00, 0x01 };
const uint8_t READ_FEATURE[] = { 0xE7, 0x00, 0x00, 0x00 };
const uint8_t PROG_FEATURE[] = { 0xE4, 0x00, 0x00, 0x00 };
const uint8_t READ_FEABITS[] = { 0xFB, 0x00, 0x00, 0x00 };
const uint8_t PROG_FEABITS[] = { 0xF8, 0x00, 0x00, 0x00 };
const uint8_t SET_PROGRAM_DONE[] = { 0x5E, 0x00, 0x00, 0x00 };
const uint8_t REFRESH[] = { 0x79, 0x00, 0x00 };
const uint8_t READ_USERCODE[] = { 0xC0, 0x00, 0x00, 0x00 };
const uint8_t PROG_USERCODE[] = { 0xC2, 0x00, 0x00, 0x00 };

} /* namespace Commands */

class DeviceInfo
{
    DeviceType _type;
    std::string _name;
    int _numCfgPages;
    int _numUFMPages;
    int _cfgEraseDelay;
    int _ufmEraseDelay;
    int _tRefresh;

    static std::map<DeviceType,DeviceInfo> AllTypes;

public:
    DeviceType type () const { return _type; }
    std::string name () const { return _name; }
    int numCfgPages () const { return _numCfgPages; }
    int numUFMPages () const { return _numUFMPages; }
    int cfgEraseDelay () const { return _cfgEraseDelay; }
    int ufmEraseDelay () const { return _ufmEraseDelay; }
    int tRefresh () const { return _tRefresh; }

public:
    DeviceInfo (DeviceType type, std::string name, int cfgPages, int ufmPages, int cfgErase, int ufmErase, int trefresh)
        : _type(type),
          _name(name),
          _numCfgPages(cfgPages),
          _numUFMPages(ufmPages),
          _cfgEraseDelay(cfgErase),
          _ufmEraseDelay(ufmErase),
          _tRefresh(trefresh)
    {}

    DeviceInfo ()
        : DeviceInfo( DeviceType::MachXO2_Unknown, std::string(), 0, 0, 0, 0, 0 )
    {}

public:
    static DeviceInfo Find (std::string deviceNameLine);
    static DeviceInfo Find (uint32_t idCode);
};

class MachXO2Device
{
    I2C::I2CDevice& _itf;
    DeviceInfo _info;

public:
    MachXO2Device(I2C::I2CDevice& itf);

public:
    const DeviceInfo& info () const { return _info; }

private:
    uint32_t QueryIDCode ()
    {
        return _itf.read<uint32_t>(Commands::IDCODE_PUB);
    }

    std::vector<uint8_t> ReadConfiguration (std::function<void(int)> reportProgress);
    void WriteUserCode (uint32_t uc);
    void EnableTransparentConfigurationMode ();
    void EraseFlash ();
    void WriteConfiguration (const std::vector<uint8_t> data,
                             std::function<void(int)> reportProgress);
    void SetProgramDone ();
    void Refresh ();

    bool CheckBusy ();
    int ReadStatus ();
    bool CheckStatusFail ();

public:
    uint32_t QueryUserCode ();
    bool UpdateConfiguration (const JedecFile& jedec,
                              std::function<void(const char*, int)> reportProgress,
                              bool forceUpdate = false);
}; /* class DeviceInfo */

} /* namespace MachXO2 */
