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
	class Flash
	{
		GenCPFacade& gencp_;

	public:
		Flash( GenCPFacade& gencp )
			: gencp_ (gencp)
		{
		}

		static const uint32_t FLASH_PAGE_SIZE = 0x10000;

	private:
		void unlock();
		void lock();
		uint32_t block_crc32( uint32_t address, uint32_t length ) const;
		void block_write( uint32_t address, const uint8_t* data, uint16_t length );
		void erase_page( uint32_t address );

	public:
		void erase( uint32_t address, uint32_t length );
		void write_verify( uint32_t address, const uint8_t* data, uint32_t length, util::progress::IReportProgress& progress );
		void erase_write_verify( uint32_t address, const uint8_t* data, uint32_t length, util::progress::IReportProgress& progress );

	public:
		std::vector<uint8_t> read( uint32_t address, uint32_t length ) const;

		void erase_write_verify( uint32_t address, const std::vector<uint8_t> data, util::progress::IReportProgress& progress )
		{
			erase_write_verify( address, data.data(), static_cast<uint32_t>(data.size()), progress );
		}
	};
} /* namespace device_interface */
} /* namespace lib33u */
