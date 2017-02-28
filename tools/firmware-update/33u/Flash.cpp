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

#include "Flash.h"
#include "MemoryMap.h"
#include "Crc32.h"
#include "ReportProgress.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <cstdint>

namespace lib33u
{
namespace device_interface
{
	namespace
	{
		const uint32_t FLASH_UNLOCK_CODE = 0xCD56631A;

		const uint64_t MEM_ADDRESS_DATA = MemoryMap::CAMERA_FLASH + 0x00000000;
		const uint64_t MEM_ADDRESS_LOCK = MemoryMap::CAMERA_FLASH + 0x00800000;
		const uint64_t MEM_ADDRESS_ERASE = MemoryMap::CAMERA_FLASH + 0x00800004;
		const uint64_t MEM_ADDRESS_CRC32_START = MemoryMap::CAMERA_FLASH + 0x00800008;
		const uint64_t MEM_ADDRESS_CRC32_LENGTH = MemoryMap::CAMERA_FLASH + 0x0080000C;
		const uint64_t MEM_ADDRESS_CRC32_RESULT = MemoryMap::CAMERA_FLASH + 0x00800010;
		const uint64_t MEM_ADDRESS_REBOOT = MemoryMap::CAMERA_FLASH + 0x00800020;

		const uint32_t MAX_CRC_BLOCK_SIZE = 0x10000;
	}

	void Flash::unlock()
	{
		gencp_.write_u32( MEM_ADDRESS_LOCK, FLASH_UNLOCK_CODE );
	}

	void Flash::lock()
	{
		gencp_.write_u32( MEM_ADDRESS_LOCK, 0 );
	}

	uint32_t Flash::block_crc32( uint32_t address, uint32_t length ) const
	{
		if( length > MAX_CRC_BLOCK_SIZE )
		{
			throw std::invalid_argument( "length for crc check has to be smaller than 64K" );
		}

		gencp_.write_u32( MEM_ADDRESS_CRC32_START, address );
		gencp_.write_u32( MEM_ADDRESS_CRC32_LENGTH, length );

		return gencp_.read_u32( MEM_ADDRESS_CRC32_RESULT );
	}

	void Flash::block_write( uint32_t address, const uint8_t* data, uint16_t length )
	{
		if( length > gencp_.max_write_mem() )
		{
			throw std::invalid_argument( "length has to be <= gencp_.max_write_mem()" );
		}

		gencp_.write_mem( MEM_ADDRESS_DATA + address, data, length );
	}

	void Flash::write_verify( uint32_t address, const uint8_t* data, uint32_t length, util::progress::IReportProgress& progress )
	{
		if( length % 4 )
		{
			throw std::invalid_argument( "length must be divisible by 4" );
		}

		unlock();

		uint32_t bytes_remaining = length;
		uint32_t offset = 0;
		uint32_t step_size = gencp_.max_write_mem();

		auto upload_progress = util::progress::MapItemProgress( progress, length );
		upload_progress.report_step_format( "Upload %d bytes", length );

		while( bytes_remaining > 0 )
		{
			uint16_t chunk_size = static_cast<uint16_t>(std::min( bytes_remaining, step_size ));

			block_write( address + offset, data + offset, chunk_size );

			auto crc_flash = block_crc32( address + offset, chunk_size );
			auto crc_buffer = util::crc32::calc( 0, data + offset, chunk_size );
			if( crc_flash != crc_buffer )
			{
				throw std::runtime_error( "CRC error" );
			}

			offset += chunk_size;
			bytes_remaining -= chunk_size;

			upload_progress.report_items( chunk_size, "KiB/s", 1024 );
		}

		lock();
	}

	void Flash::erase_page( uint32_t address )
	{
		try
		{
			gencp_.write_u32( MEM_ADDRESS_ERASE, address );
		}
		catch( std::exception )
		{
			// If this exception occurs, we might have run into a timeout because of a slow flash chip
			// Since we cannot fix the firmware for this (this is a firmware upgrade in progress),
			// we need to fix this issue in the updater
			// Ignore this error and give the camera some time

			//using namespace std::chrono_literals;
            std::chrono::milliseconds ms{1000};
			std::this_thread::sleep_for( ms );
		}
	}

	void Flash::erase( uint32_t address, uint32_t length )
	{
		unlock();

		uint32_t bytes_remaining = length;
		uint32_t offset = 0;
		uint32_t step_size = FLASH_PAGE_SIZE;

		while( bytes_remaining > 0 )
		{
			uint32_t chunk_size = std::min( bytes_remaining, step_size );

			erase_page( address + offset );

			offset += chunk_size;
			bytes_remaining -= chunk_size;
		}

		lock();
	}

	void Flash::erase_write_verify( uint32_t address, const uint8_t* data, uint32_t length, util::progress::IReportProgress& progress )
	{
		erase( address, length );
		write_verify( address, data, length, progress );
	}

	std::vector<uint8_t> Flash::read( uint32_t address, uint32_t length ) const
	{
		if( length % 4 )
		{
			throw std::invalid_argument( "length must be divisible by 4" );
		}

		std::vector<uint8_t> result( length );

		uint32_t bytes_remaining = length;
		uint32_t offset = 0;
		uint32_t step_size = gencp_.max_read_mem();
		while( bytes_remaining > 0 )
		{
			uint16_t chunk_size = static_cast<uint16_t>(std::min( bytes_remaining, step_size ));

			gencp_.read_mem( MEM_ADDRESS_DATA + address + offset, result.data() + offset, chunk_size );

			offset += chunk_size;
			bytes_remaining -= chunk_size;
		}

		return result;
	}
} /* namespace device_interface */
} /* namespace lib33u */
