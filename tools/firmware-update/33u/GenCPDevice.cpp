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

#include "GenCPDevice.h"
#include "GenCPCommand.h"
#include "VendorCommands.h"

#include <stdexcept>

namespace lib33u
{
namespace device_interface
{
namespace gencp
{
	GenCPDevice::GenCPDevice (std::shared_ptr<driver_interface::IUsbDevice> dev)
		: dev_ { dev }
	{
	}

	uint16_t GenCPDevice::device_read_buffer_size () const
	{
		if( device_read_buffer_size_ == 0 )
		{
			auto val = dev_->read_vendor_request<uint32_t>( VendorCommands::GENCP_MAX_READ, 0, 0 );

			device_read_buffer_size_ = static_cast<uint16_t>(val);
		}

		return device_read_buffer_size_;
	}

	uint16_t GenCPDevice::device_write_buffer_size () const
	{
		if( device_write_buffer_size_ == 0 )
		{
			auto val = dev_->read_vendor_request<uint32_t>( VendorCommands::GENCP_MAX_WRITE, 0, 0 );

			device_write_buffer_size_ = static_cast<uint16_t>(val);
		}

#ifdef __linux__
        if (device_write_buffer_size_ > 4096)
        {
            device_write_buffer_size_ = 4096;
        }
#endif
		return device_write_buffer_size_;
	}

	uint16_t GenCPDevice::max_read_mem () const
	{
		return device_read_buffer_size() - (4 + 8);
	}

	uint16_t GenCPDevice::max_write_mem () const
	{
		return device_write_buffer_size() - (4 + 8 + 8);
	}

	void GenCPDevice::read_mem (uint64_t address, uint8_t* buffer, uint16_t length) const
	{
		std::unique_lock<std::mutex> lck( mtx_ );

		if (length % 4)
		{
			throw std::invalid_argument("length must be divisible by 4");
		}

		uint16_t req_id;
		auto cmd = build_readmem( address, length, req_id );

		dev_->write_vendor_request(VendorCommands::GENCP, 0, 0, cmd.data(),
                                   static_cast<uint16_t>(cmd.size()));

		std::vector<uint8_t> ack_buffer(device_read_buffer_size());
		dev_->read_vendor_request(VendorCommands::GENCP, 0, 0, ack_buffer.data(),
                                  static_cast<uint16_t>(ack_buffer.size()));

		parse_readmem_ack(ack_buffer, req_id, buffer, length);
	}

	void GenCPDevice::write_mem (uint64_t address, const uint8_t* buffer, uint16_t length)
	{
		std::unique_lock<std::mutex> lck(mtx_);

		if (length % 4)
		{
			throw std::invalid_argument("length must be divisible by 4");
		}

		uint16_t req_id;
		auto cmd = build_writemem(address, buffer, length, req_id);

		dev_->write_vendor_request(VendorCommands::GENCP, 0, 0, cmd.data(),
                                   static_cast<uint16_t>(cmd.size()));

		std::vector<uint8_t> ack_buffer(device_read_buffer_size());
		dev_->read_vendor_request(VendorCommands::GENCP, 0, 0, ack_buffer.data(),
                                  static_cast<uint16_t>(ack_buffer.size()));

		parse_writemem_ack(ack_buffer, req_id);
	}
} /* namespace gencp */
} /* namespace device_interface */
} /* namespace lib33u */
