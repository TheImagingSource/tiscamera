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

#include <mutex>
#include <memory>
#include <cstdint>

namespace lib33u
{
namespace device_interface
{
namespace gencp
{
	class GenCPDevice : public driver_interface::IGenCPDevice
	{
	private:
		std::shared_ptr<driver_interface::IUsbDevice> dev_;
		mutable std::mutex mtx_;

		mutable uint16_t device_read_buffer_size_ = 0;
		mutable uint16_t device_write_buffer_size_ = 0;

	public:
		GenCPDevice( std::shared_ptr<driver_interface::IUsbDevice> dev );

	public:
		virtual uint16_t max_read_mem() const override;
		virtual uint16_t max_write_mem() const override;
		virtual void read_mem( uint64_t address, uint8_t * buffer, uint16_t length ) const override;
		virtual void write_mem( uint64_t address, const uint8_t * buffer, uint16_t length ) override;

	private:
		uint16_t device_read_buffer_size() const;
		uint16_t device_write_buffer_size() const;
	};

} /* namespace gencp */
} /* namespace device_interface */
} /* namespace lib33u */
