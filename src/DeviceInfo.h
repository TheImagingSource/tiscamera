/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef TCAM_DEVICEINFO_H
#define TCAM_DEVICEINFO_H

#include "base_types.h"

#include <vector>
#include <memory>

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

/**
 * @class DeviceInfo
 * Contains a unique device description
 */
class DeviceInfo
{

public:


    explicit DeviceInfo (const struct tcam_device_info&);

    /**
     * @brief Creates an invalid device
     */
    DeviceInfo ();

    DeviceInfo& operator= (const DeviceInfo&);

    /**
     * @name get_info
     * @brief returns a struct version of the device description
     * @return struct tcam_device_info
     */
    struct tcam_device_info get_info () const;

    /**
     * Description for get_name.
     * @return string containing the device model
     */
    std::string get_name () const;

    /**
     * @return string containing the serial number of the device
     */
    std::string get_serial () const;

    /**
     * returns identifier used for communication
     * with underlying system (e.g. /dev/video0)
     * @return string containing the identifier
     */
    std::string get_identifier () const;

    /**
     * @return TCAM_DEVICE_TYPE of the device
     */
    enum TCAM_DEVICE_TYPE get_device_type () const;

    /**
     * @brief returns @TCAM_DEVICE_TYPE string representation
     * @return std::string
     */
    std::string get_device_type_as_string () const;

private:

    /// internal device representation
    struct tcam_device_info device;

}; /* class DeviceInfo */

} /* namespace tcam */

/** @} */

#endif /* TCAM_DEVICEINFO_H */
