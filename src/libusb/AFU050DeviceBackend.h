

#pragma once

#include "../error.h"
#include "afu050_definitions.h"

namespace tcam
{
class AFU050Device;
}

namespace tcam::property
{
class AFU050DeviceBackend
{
public:
    explicit AFU050DeviceBackend(AFU050Device*);

    outcome::result<int64_t> get_int(control_definition ctrl, CONTROL_CMD cmd = GET_CUR);
    outcome::result<void> set_int(control_definition ctrl,
                                  int64_t new_value,
                                  CONTROL_CMD cmd = SET_CUR);
    outcome::result<bool> get_bool(control_definition ctrl, CONTROL_CMD cmd = GET_CUR);
    outcome::result<void> set_bool(control_definition ctrl,
                                   bool new_value,
                                   CONTROL_CMD cmd = SET_CUR);

private:
    AFU050Device* p_device;
};

} // namespace tcam::property
