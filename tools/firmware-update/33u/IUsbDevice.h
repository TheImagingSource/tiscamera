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

namespace lib33u
{
namespace driver_interface
{
	struct IUsbDevice
	{
		virtual ~IUsbDevice ()
		{}

		virtual uint16_t product_id () = 0;
		virtual void open () = 0;

		virtual void read_vendor_request (uint8_t req,
                                          uint16_t value,
                                          uint16_t index,
                                          uint8_t* buffer,
                                          uint16_t length) = 0;

		template<typename T> T read_vendor_request (uint8_t req,
                                                    uint16_t value,
                                                    uint16_t index)
		{
			T val;
			read_vendor_request(req, value, index, reinterpret_cast<uint8_t*>(&val), sizeof(val));

			return val;
		}

		virtual void write_vendor_request (uint8_t req,
                                           uint16_t value,
                                           uint16_t index,
                                           uint8_t* data,
                                           uint16_t length) = 0;
	};
}
}
