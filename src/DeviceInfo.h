



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
