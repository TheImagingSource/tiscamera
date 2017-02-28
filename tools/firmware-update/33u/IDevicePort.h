#pragma once

#include "Camera.h"
#include "UploadGroup.h"
#include "IReportProgress.h"

#include <string>

namespace lib33u
{
namespace firmware_update
{
	struct IDevicePort
	{
		virtual ~IDevicePort ()
		{}

		virtual std::string name () = 0;
		virtual void upload (Camera& dev, const std::vector<UploadItem>& items,
                             util::progress::IReportProgress& progress) = 0;
	};
} /* namespace device_interface */
} /* namespace lib33u */
