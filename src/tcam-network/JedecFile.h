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

#include "MachXO2.h"

#include <vector>
#include <cstdint>

namespace MachXO2
{

class JedecFile
{
    DeviceType _deviceType;
    uint32_t _userCode;
    std::vector<uint8_t> _configurationData;
    std::vector<uint8_t> _featureRow;
    std::vector<uint8_t> _featureBits;

public:
    DeviceType deviceType () const { return _deviceType; }
    uint32_t userCode () const { return _userCode; }
    const std::vector<uint8_t>& configurationData () const { return _configurationData; }
    const std::vector<uint8_t>& featureRow () const { return _featureRow; }
    const std::vector<uint8_t>& featureBits () const { return _featureBits; }

private:
    JedecFile (DeviceType devType, uint32_t uc,
               std::vector<uint8_t> configurationData,
               std::vector<uint8_t> featureRow,
               std::vector<uint8_t> featureBits)
        : _deviceType(devType),
          _userCode(uc),
          _configurationData(configurationData),
          _featureRow(featureRow),
          _featureBits(featureBits)
    {
    }

public:

    static JedecFile Parse (const std::vector<uint8_t>& data);

}; /* class JedecFile */

} /* namespace MachXO2 */
