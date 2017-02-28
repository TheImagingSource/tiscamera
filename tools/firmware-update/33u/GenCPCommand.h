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
namespace device_interface
{
namespace gencp
{
	enum class CommandFlags : uint16_t
	{
		NONE = 0,
		REQUEST_ACK = 0x4000,
		COMMAND_RESEND = 0x8000
	};

	enum class CommandId : uint16_t
	{
		READMEM_CMD = 0x0800,
		READMEM_ACK = 0x0801,
		WRITEMEM_CMD = 0x0802,
		WRITEMEM_ACK = 0x0803,
		PENDING_ACK = 0x0805,
		EVENT_CMD = 0x0C00,
		EVENT_ACK = 0x0C01
	};

#pragma pack(push, 2)
	struct CommonCommandData
	{
		CommandFlags flags;
		CommandId id;
		uint16_t length;
		uint16_t req_id;
	};

	struct ReadMemCommand
	{
		uint32_t prefix;
		CommonCommandData common;
		uint64_t address;
		uint16_t reserved;
		uint16_t length;
	};

	struct WriteMemCommand
	{
		uint32_t prefix;
		CommonCommandData common;
		uint64_t address;
	};

	struct CommonAckData
	{
		uint16_t status;
		CommandId id;
		uint16_t length;
		uint16_t req_id;
	};

	struct ReadMemAck
	{
		uint32_t prefix;
		CommonAckData common;
	};
#pragma pack(pop)

	std::vector<uint8_t> build_readmem( uint64_t address, uint16_t length, uint16_t& req_id );
	std::vector<uint8_t> build_writemem( uint64_t address, const uint8_t* data, uint16_t length, uint16_t& req_id );

	void parse_readmem_ack( const std::vector<uint8_t>& ack_buffer, uint16_t req_id, uint8_t* dest, uint16_t dest_length );
	void parse_writemem_ack( const std::vector<uint8_t>& ack_buffer, uint16_t req_id );

} /* namespace gencp */
} /*namespace device_interface */
} /* namespace lib33u */
