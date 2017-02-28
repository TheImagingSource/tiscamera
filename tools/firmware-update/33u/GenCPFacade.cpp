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

#include "GenCPFacade.h"

#include <limits>
#include <string.h>

namespace lib33u
{
namespace device_interface
{
	std::vector<uint8_t> GenCPFacade::read_mem (uint64_t address, uint16_t length) const
	{
		std::vector<uint8_t> result( length );
		read_mem(address, result.data(), (uint16_t)result.size());
		return result;
	}

	void GenCPFacade::write_mem (uint64_t address, const std::vector<uint8_t>& buffer)
	{
		if (buffer.size() > std::numeric_limits<uint16_t>::max())
        {
			throw std::range_error("buffer too large");
        }
		write_mem(address, buffer.data(), (uint16_t)buffer.size());
	}

	std::string GenCPFacade::read_string (uint64_t address, uint16_t length) const
	{
		uint16_t aligned_length = (uint16_t)((length + 3) & 0xFFFC);

		std::string buffer;
		buffer.resize( aligned_length );

		read_mem(address,
                 reinterpret_cast<uint8_t*>(&buffer[0]),
                 static_cast<uint16_t>(buffer.length()) );

		auto actual_length = strlen( buffer.c_str() );
		buffer.resize( actual_length );

		return buffer;
	}
} /* namespace device_interface */
} /* namespace lib33u */
