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

#include "MachXO3.h"

#include "JedecFile.h"

#include <cstdlib>
#include <algorithm>
#include <iterator>

#include <unistd.h> // usleep
#include <climits>

using namespace MachXO3;

inline int millisleep (const unsigned int millisec)
{
    return usleep(millisec * 1000);
}


template <typename T>
T swap_endian (T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
    {
        dest.u8[k] = source.u8[sizeof(T) - k - 1];
    }

    return dest.u;
}


std::map<DeviceType, DeviceInfo> MachXO3::DeviceInfo::AllTypes =
{
    { DeviceType::MachXO2_256, { DeviceType::MachXO2_256, "MachXO2-256", 575, 0, 700, 0, 1 } },
    { DeviceType::MachXO2_640, { DeviceType::MachXO2_640, "MachXO2-640", 1152, 191, 1100, 600, 1 } },
    { DeviceType::MachXO2_640U, { DeviceType::MachXO2_640U, "MachXO2-640U", 2175, 512, 1400, 700, 1 } },
    { DeviceType::MachXO2_1200, { DeviceType::MachXO2_1200, "MachXO2-1200", 2175, 512, 1400, 700, 1 } },
    { DeviceType::MachXO2_1200U,{ DeviceType::MachXO2_1200U, "MachXO2-1200U", 3200, 639, 1900, 900, 2 } },
    { DeviceType::MachXO2_2000, { DeviceType::MachXO2_2000, "MachXO2-2000", 3200, 639, 1900, 900, 2 } },
    { DeviceType::MachXO2_2000U,{ DeviceType::MachXO2_2000U, "MachXO2-2000U", 5760, 767, 3100, 1000, 3 } },
    { DeviceType::MachXO2_4000, { DeviceType::MachXO2_4000, "MachXO2-4000", 5760, 767, 3100, 1000, 3 } },
    { DeviceType::MachXO2_7000, { DeviceType::MachXO2_7000, "MachXO2-7000", 9216, 2046, 4800, 1600, 4 } },

    { DeviceType::MachXO3_6900, { DeviceType::MachXO3_6900, "MachXO3L-6900", 9216, 2046, 4800, 1600, 4 } },
};


bool string_contains (const std::string& str, const std::string& sub)
{
    return str.find(sub) != std::string::npos;
}


DeviceInfo DeviceInfo::Find (std::string deviceNameLine)
{
    if( string_contains(deviceNameLine, "LCMXO2-256"))
        return AllTypes[DeviceType::MachXO2_256];
    if( string_contains(deviceNameLine, "LCMXO2-640"))
        return AllTypes[DeviceType::MachXO2_640];
    if( string_contains(deviceNameLine, "LCMXO2-1200"))
        return AllTypes[DeviceType::MachXO2_1200];
    if( string_contains(deviceNameLine, "LCMXO2-2000"))
        return AllTypes[DeviceType::MachXO2_2000];
    if( string_contains(deviceNameLine, "LCMXO2-4000"))
        return AllTypes[DeviceType::MachXO2_4000];
    if( string_contains(deviceNameLine, "LCMXO2-7000"))
        return AllTypes[DeviceType::MachXO2_7000];
    if( string_contains(deviceNameLine, "LCMXO3L-6900"))
        return AllTypes[DeviceType::MachXO3_6900];

    return {};
}


DeviceInfo DeviceInfo::Find (uint32_t idCode)
{
    switch(idCode)
    {
        case 0x43002B01:
        case 0x43802B01:
            return Find("LCMXO2-256");
        case 0x43102B01:
        case 0x43902B01:
            return Find("LCMXO2-640");
        case 0x43202B01:
        case 0x43A02B01:
            return Find("LCMXO2-1200");
        case 0x43302B01:
        case 0x43B02B01:
            return Find("LCMXO2-2000");
        case 0x43402B01:
        case 0x43C02B01:
            return Find("LCMXO2-4000");
        case 0x43502B01:
        case 0x43D02B01:
            return Find("LCMXO2-7000");
        case 0x43502b61:
        case 0x43502b41:
            return Find("LCMXO3L-6900");
        default:
            return {};
    }
}


MachXO3Device::MachXO3Device (I2C::I2CDevice& itf)
    : _itf(itf)
{
    uint32_t idCode = QueryIDCode();
    _info = DeviceInfo::Find(idCode);

    // Querying the ID code sometimes yields bogus results on the first try
    if (_info.type() == DeviceType::MachXO2_Unknown)
    {
        idCode = QueryIDCode();
        _info = DeviceInfo::Find(idCode);
    }

    if (_info.type() == DeviceType::MachXO2_Unknown)
        throw std::runtime_error("Unknown MachXO device type");
}


std::vector<uint8_t> MachXO3Device::ReadConfiguration (std::function<void(int)> reportProgress)
{
    std::vector<uint8_t> buffer;

    _itf.write(Commands::INIT_ADDRESS);

    // Maximum pages per request as defined by MachXO3 spec
    const int MACHXO2_MAX_PAGES_PER_REQUEST = 32;

    // Maximum pages per request, limited by the interface's maximum I2C read length (GigE, I am looking at you!)
    int interfaceMaxPagesPerRequest = ((int)_itf.maxReadLength() - 16 - 16) / 20 - 1;

    // Actual maximum pages per request
    int PAGES_PER_REQUEST = std::min(MACHXO2_MAX_PAGES_PER_REQUEST, interfaceMaxPagesPerRequest);

    // Build read command with num of pages per request
    uint8_t readCmd[4] = { 0x73, 0, 0, (uint8_t)(PAGES_PER_REQUEST + 1) };

    int totalPages = info().numCfgPages();

    for (int p = 0; p < totalPages; p += PAGES_PER_REQUEST)
    {
        auto data = _itf.transaction(readCmd, 16 + 16 + (PAGES_PER_REQUEST + 1) * (16 + 4));

        reportProgress(p * 100 / totalPages);

        for (int i = 0; i < PAGES_PER_REQUEST && (p + i) < totalPages; ++i)
        {
            auto it = data.begin() + 16 + 16 + i * (16 + 4);
            std::copy(it, it + 16, std::back_inserter(buffer));
        }
    }

   reportProgress(100);

    return buffer;
}


uint32_t MachXO3Device::QueryUserCode()
{
    auto uc = _itf.read<uint32_t>(Commands::READ_USERCODE);

    // For some reason, the user code is returned byte-swapped once the device has been disconnected
    return swap_endian<uint32_t>(uc);
}


static std::function<void(int)> map_progress (std::function<void(const char*, int)> progress,
                                              int begin, int end)
{
    return [=](int x)
    {
        progress("", begin + x * (end - begin) / 100);
    };
}


bool MachXO3Device::UpdateConfiguration (const JedecFile& jedec,
                                         std::function<void(const char*, int)> reportProgress,
                                         bool forceUpdate)
{
    if (jedec.deviceType() != info().type())
    {
        throw std::runtime_error("The MachXO3 device does not match Jedec File");
    }

    if (jedec.userCode() == QueryUserCode() && !forceUpdate)
        return false;

    reportProgress("Writing auxiliary FPGA configuration", 0 );

    EnableTransparentConfigurationMode();
    EraseFlash();
    SetProgramDone();
    Refresh();

    millisleep(1000);

    EnableTransparentConfigurationMode();
    WriteConfiguration(jedec.configurationData(), map_progress(reportProgress, 0, 70));

    reportProgress("Verifying auxiliary FPGA configuration", 70);

    auto verifyBuffer = ReadConfiguration(map_progress(reportProgress, 70, 100));
    if (verifyBuffer != jedec.configurationData())
    {
        SetProgramDone();
        throw std::runtime_error("Verification failed");
    }

    SetProgramDone();
    Refresh();

    // Contrary to the documentation the user code has to be written AFTER refresh, otherwise it will be zero... (?)
    EnableTransparentConfigurationMode();
    WriteUserCode( jedec.userCode() );
    SetProgramDone();

    return true;
}


void MachXO3Device::WriteUserCode (uint32_t uc)
{
    _itf.write(Commands::PROG_USERCODE, uc);
}


bool MachXO3Device::CheckBusy ()
{
    auto busy = _itf.read<uint8_t>(Commands::CHECK_BUSY);

    return (busy & 0x80) != 0;
}


static bool IsStatusFail (uint32_t status)
{
    return (status & (1 << 13)) != 0;
}

/*
static bool IsStatusBusy (uint32_t status)
{
    return (status & (1 << 12)) != 0;
}
*/

int MachXO3Device::ReadStatus ()
{
    uint32_t status = _itf.read<uint32_t>(Commands::READ_STATUS);
    // bool isBusy = IsStatusBusy(status);
    // bool isFail = IsStatusFail(status);

    return status;
}


bool MachXO3Device::CheckStatusFail ()
{
    return IsStatusFail(ReadStatus());
}


void MachXO3Device::EnableTransparentConfigurationMode ()
{
    _itf.write(Commands::ISC_ENABLE_X);

    if (!CheckBusy())
    {
        millisleep(1);
    }
    else
    {
        while (CheckBusy())
        {}
    }

    if (CheckStatusFail())
    {
        throw std::runtime_error("The MachXO3 device is in fail state after enabling configuration mode");
    }
}


void MachXO3Device::EraseFlash ()
{
    _itf.write(Commands::ERASE_FLASH);

    while (CheckBusy())
    {}

    if (CheckStatusFail())
    {
        throw std::runtime_error("The MachXO3 is in failed state after trying to erase flash and features");
    }
}


void MachXO3Device::WriteConfiguration (const std::vector<uint8_t> data,
                                        std::function<void(int)> reportProgress)
{
    _itf.write(Commands::INIT_ADDRESS);

    int totalPages = info().numCfgPages();

    for (int p = 0; p < totalPages; ++p)
    {
        I2C::DataArray command(20);
        memcpy(command.data(), Commands::PROG_ONE_PAGE, 4);
        memcpy(command.data() + 4, data.data() + p * 16, 16);
        _itf.write(command);

        reportProgress(p * 100 / totalPages);

        if (!CheckBusy())
        {
            millisleep(1);
        }
        else
        {
            while (CheckBusy())
            {}
        }
    }

    reportProgress(100);
}


void MachXO3Device::SetProgramDone ()
{
    _itf.write(Commands::SET_PROGRAM_DONE);

    if (!CheckBusy())
    {
        millisleep(1);
    }
    else
    {
        while (CheckBusy())
        {}
    }
}


void MachXO3Device::Refresh ()
{
    _itf.write(Commands::REFRESH);

    millisleep(info().tRefresh() * 1000);

    if (CheckStatusFail())
    {
        throw std::runtime_error("MachXO3 device is in fail state after REFRESH command");
    }
}
