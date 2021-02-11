/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "GenCPFacade.h"
#include "IReportProgress.h"

namespace lib33u
{
namespace device_interface
{
class EEPROM
{
    GenCPFacade& gencp_;

public:
    EEPROM(GenCPFacade& gencp) : gencp_(gencp) {}

private:
    void unlock();
    void lock();
    uint32_t block_crc32(uint32_t address, uint32_t length) const;
    void block_write(uint32_t address, const uint8_t* data, uint16_t length);

public:
    void write_verify(uint32_t address,
                      const uint8_t* data,
                      uint32_t length,
                      util::progress::IReportProgress& progress);
};
} /* namespace device_interface */
} /* namespace lib33u */
