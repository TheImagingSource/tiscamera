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

#include "IUsbDevice.h"
#include "GenCPFacade.h"
#include "Flash.h"

#include <memory>
#include <string>
#include <cstdint>

namespace lib33u
{
	class Camera
	{
		struct Impl
		{
			std::shared_ptr<driver_interface::IUsbDevice>	device_;
			device_interface::GenCPFacade					gencp_;
			device_interface::Flash							flash_;

			Impl (std::shared_ptr<driver_interface::IUsbDevice> dev);

			uint16_t product_id () const;
			std::string model_description () const;
			std::string fpga_version () const;
			std::string nios_firmware_version () const;
		};

		std::unique_ptr<Impl> impl_;


	public:
		Camera (std::shared_ptr<driver_interface::IUsbDevice> dev);

        // prefer this
		static Camera attach (std::shared_ptr<driver_interface::IUsbDevice> dev);

	public:
		uint16_t product_id () const
		{
			return impl_->product_id();
		}
		std::string model_description () const
		{
			return impl_->model_description();
		}
		std::string fpga_version () const
		{
			return impl_->fpga_version();
		}
		std::string nios_firmware_version () const
		{
			return impl_->nios_firmware_version();
		}

	public:
		device_interface::Flash& flash () const
		{
			return impl_->flash_;
		}
		device_interface::GenCPFacade& gencp () const
		{
			return impl_->gencp_;
		}
	};
}
