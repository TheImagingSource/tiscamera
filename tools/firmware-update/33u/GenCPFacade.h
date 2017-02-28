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

#include "IGenCPDevice.h"
#include "IUsbDevice.h"
#include "GenCPDevice.h"

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace lib33u
{
namespace device_interface
{
	class GenCPFacade
	{
		std::unique_ptr<driver_interface::IGenCPDevice> dev_;

	public:
		GenCPFacade( std::shared_ptr<driver_interface::IUsbDevice> dev )
			: dev_ { std::unique_ptr<device_interface::gencp::GenCPDevice>(new  device_interface::gencp::GenCPDevice(dev) ) }
		{
		}

	public:
		void read_mem( uint64_t address, uint8_t* buffer, uint16_t length ) const
		{
			dev_->read_mem( address, buffer, length );
		}
		void write_mem( uint64_t address, const uint8_t* buffer, uint16_t length )
		{
			dev_->write_mem( address, buffer, length );
		}

		uint16_t max_read_mem() const
		{
			return dev_->max_read_mem();
		}
		uint16_t max_write_mem() const
		{
			return dev_->max_write_mem();
		}

	public:
		std::vector<uint8_t> read_mem( uint64_t address, uint16_t length ) const;
		void write_mem( uint64_t address, const std::vector<uint8_t>& buffer );

		std::string read_string( uint64_t address, uint16_t length ) const;

		template<typename T>
		T read( uint64_t address )
		{
			uint32_t value;
			read_mem( address, reinterpret_cast<uint8_t*>(&value), sizeof( value ) );

			return value;
		}

		template<typename T>
		void write( uint64_t address, T value )
		{
			write_mem( address, reinterpret_cast<uint8_t*>(&value), sizeof( value ) );
		}

		uint32_t read_u32( uint64_t address )
		{
			return read<uint32_t>( address );
		}

		void write_u32( uint64_t address, uint32_t value )
		{
			write( address, value );
		}
	};
} /* namespace device_interface */
} /* namespace lib33u */
