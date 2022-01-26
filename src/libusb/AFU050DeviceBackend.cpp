
#include "AFU050DeviceBackend.h"

#include "AFU050Device.h"
#include "../logging.h"

namespace tcam::property
{

AFU050DeviceBackend::AFU050DeviceBackend(AFU050Device* dev) : p_device(dev) {}


outcome::result<int64_t> AFU050DeviceBackend::get_int(control_definition ctrl, CONTROL_CMD cmd)
{
    int value = 0;
    bool ret = p_device->get_control(ctrl.unit, ctrl.control, 4, (unsigned char*)&value, cmd);
    if (!ret)
        SPDLOG_ERROR("get_control returned with: {}", ret);
    return value;
}


outcome::result<void> AFU050DeviceBackend::set_int(control_definition ctrl,
                                                   int64_t new_value,
                                                   CONTROL_CMD /*cmd*/)
{
    int value = new_value;
    bool ret = p_device->set_control(ctrl.unit, ctrl.control, 4, (unsigned char*)&value);
    if (!ret)
        SPDLOG_ERROR("set_control returned with: {}", ret);
    return outcome::success();
}


outcome::result<bool> AFU050DeviceBackend::get_bool(control_definition ctrl, CONTROL_CMD cmd)
{
    int value = 0;
    bool ret = p_device->get_control(ctrl.unit, ctrl.control, 4, (unsigned char*)&value, cmd);
    if (ret)
    {
        SPDLOG_ERROR("get_control returned with: {}", ret);
    }
    return (value ? 1 : 0);
}


outcome::result<void> AFU050DeviceBackend::set_bool(control_definition ctrl,
                                                    bool new_value,
                                                    CONTROL_CMD /*cmd*/)
{
    char val = (new_value ? 1 : 0);
    bool ret = p_device->set_control(ctrl.unit, ctrl.control, 4, (unsigned char*)&val);
    if (!ret)
        SPDLOG_ERROR("set_control returned with: {}", ret);
    return outcome::success();
}


} // namespace tcam::property
