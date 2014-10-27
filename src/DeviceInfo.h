



#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "base_types.h"

#include <vector>
#include <memory>


namespace tcam
{

/**
 * @class
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
     * @name getInfo
     * @brief returns a struct version of the device description
     * @return struct tcam_device_info
     */
    struct tcam_device_info getInfo () const;

    std::string getName () const;

    std::string getSerial () const;

    /**
     * returns identifier used for communication
     * with underlying system (e.g. /dev/video0)
     * @return string containing the identifier
     */
    std::string getIdentifier () const;

    enum TCAM_DEVICE_TYPE getDeviceType () const;

    /**
     * @brief returns @TCAM_DEVICE_TYPE string representation
     * @return std::string
     */
    std::string getDeviceTypeAsString () const;

private:

    /// internal device representation
    struct tcam_device_info device;

}; /* class DeviceInfo */

} /* namespace tcam */

#endif /* DEVICEINFO_H */
