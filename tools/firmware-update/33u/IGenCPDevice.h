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

#include <cstdint>
#include <vector>

namespace lib33u
{
namespace driver_interface
{
	struct IGenCPDevice
	{
		virtual ~IGenCPDevice()
		{
		}

		virtual uint16_t max_read_mem() const = 0;
		virtual uint16_t max_write_mem() const = 0;

		virtual void read_mem( uint64_t address, uint8_t* buffer, uint16_t length ) const = 0;
		virtual void write_mem( uint64_t address, const uint8_t* buffer, uint16_t length ) = 0;
	};
} /* namespace driver_interface */
} /* namespace lib33u */
