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

#include "Firmware.h"
#include "FirmwarePackage.h"

namespace lib33u
{
	using namespace firmware_update;

	Firmware::Firmware (std::unique_ptr<IFirmware> impl)
		: impl_ { std::move( impl ) }
	{
	}

	int Firmware::version () const
	{
		return impl_->version();
	}

	std::vector<DeviceTypeDesc> Firmware::device_types () const
	{
		return impl_->device_types();
	}

	void Firmware::upload (Camera& dev, util::progress::IReportProgress& progress,
                           DeviceTypeDesc overrideDeviceType)
	{
		return impl_->upload( dev, progress, overrideDeviceType );
	}

	Firmware Firmware::load_package (const std::string& fn)
	{
		return { std::unique_ptr<FirmwarePackage>( new FirmwarePackage(fn) ) };
	}
}
