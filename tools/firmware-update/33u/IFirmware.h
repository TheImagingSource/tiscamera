#pragma once

#include "Camera.h"
#include "DeviceTypeDesc.h"
#include "IReportProgress.h"

#include <vector>

namespace lib33u
{
namespace firmware_update
{
struct IFirmware
{
    virtual ~IFirmware() {}

    virtual int version() const = 0;
    virtual std::vector<DeviceTypeDesc> device_types() const = 0;
    virtual void upload(Camera& dev,
                        util::progress::IReportProgress& progress,
                        DeviceTypeDesc overrideDeviceType = {}) = 0;
};
} // namespace firmware_update
} /* namespace lib33u */
