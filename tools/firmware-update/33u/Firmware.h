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

#include "IFirmware.h"
#include "Camera.h"

#include <vector>
#include <memory>

namespace lib33u
{
	using namespace firmware_update;

	class Firmware : public IFirmware
	{
	public:
		virtual int version () const override;
		virtual std::vector<DeviceTypeDesc> device_types () const override;
		virtual void upload (Camera & dev, util::progress::IReportProgress& progress, DeviceTypeDesc overrideDeviceType = {}) override;

	public:
		static Firmware load_package (const std::string& fn);

	private:
		std::unique_ptr<IFirmware> impl_;

		Firmware (std::unique_ptr<IFirmware> impl);
	};
}
