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

#include "GenCPCommand.h"

#include <cstring>
#include <stdexcept>

namespace lib33u
{
namespace device_interface
{
namespace gencp
{
	uint16_t g_request_id = 0;

	std::vector<uint8_t> build_readmem( uint64_t address, uint16_t length, uint16_t& req_id )
	{
		std::vector<uint8_t> result( sizeof( ReadMemCommand ) );
		auto cmd = reinterpret_cast<ReadMemCommand*>(result.data());

		cmd->prefix = 0x43563355; // U3VC
		cmd->common.flags = CommandFlags::NONE;
		cmd->common.id = CommandId::READMEM_CMD;
		cmd->common.length = static_cast<uint16_t>(result.size());
		cmd->common.req_id = req_id = g_request_id++;
		cmd->address = address;
		cmd->reserved = 0;
		cmd->length = length;

		return result;
	}

	void parse_readmem_ack( const std::vector<uint8_t>& ack_buffer, uint16_t req_id, uint8_t* dest, uint16_t dest_length )
	{
		auto ack = reinterpret_cast<const CommonAckData*>(ack_buffer.data() + 4);

		if( ack->req_id != req_id )
			throw std::runtime_error( "ack::req_id mismatch" );

		if( ack->status & 0x8000 )
			throw std::runtime_error( "status failed" );

		if( ack->length != dest_length )
			throw std::runtime_error( "ack length mismatch" );

		auto data_start = ack_buffer.data() + 4 + sizeof( CommonAckData );

		memcpy( dest, data_start, dest_length );
	}


	std::vector<uint8_t> build_writemem( uint64_t address, const uint8_t* data, uint16_t length, uint16_t& req_id )
	{
		std::vector<uint8_t> result;
		result.reserve( sizeof( WriteMemCommand ) + length);
		result.resize( sizeof( WriteMemCommand ) );

		auto cmd = reinterpret_cast<WriteMemCommand*>(result.data());
		cmd->prefix = 0x43563355; // U3VC
		cmd->common.flags = CommandFlags::REQUEST_ACK;
		cmd->common.id = CommandId::WRITEMEM_CMD;
		cmd->common.length = sizeof( cmd->address ) + length;
		cmd->common.req_id = req_id = g_request_id++;
		cmd->address = address;

		result.insert( result.end(), data, data + length );

		return result;
	}

	void parse_writemem_ack( const std::vector<uint8_t>& ack_buffer, uint16_t req_id )
	{
		auto ack = reinterpret_cast<const CommonAckData*>(ack_buffer.data() + 4);

		if( ack->req_id != req_id )
			throw std::runtime_error( "ack::req_id mismatch" );

		if( ack->status & 0x8000 )
			throw std::runtime_error( "status failed" );
	}
} /* namespace gencp */
} /* namespace device_interface */
} /* namespace lib33u */
