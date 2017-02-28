#pragma once

#include "DeviceTypeDesc.h"
#include "Camera.h"
#include "IReportProgress.h"

#include <vector>

namespace lib33u
{
namespace firmware_update
{
	struct IFirmware
	{
		virtual ~IFirmware()
		{
		}

		virtual int version () const = 0;
		virtual std::vector<DeviceTypeDesc> device_types () const = 0;
		virtual void upload (Camera& dev, util::progress::IReportProgress& progress,
                             DeviceTypeDesc overrideDeviceType = {}) = 0;
	};
} /* namespace device_interface */
} /* namespace lib33u */
