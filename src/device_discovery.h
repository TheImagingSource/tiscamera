



#ifndef _CAMERA_INDEX_H_
#define _CAMERA_INDEX_H_

#include "base_types.h"
#include "config.h"

#include "CaptureDevice.h"

#include <vector>
#include <thread>

namespace tcam
{

class DeviceIndex
{

public:

    DeviceIndex ();

    ~DeviceIndex ();

    std::vector<CaptureDevice> getDeviceList () const;

private:

    bool continue_thread;
    unsigned int wait_period;
    std::thread work_thread;

    std::vector<CaptureDevice> device_list;

    void updateDeviceList ();

    void run ();

};

std::shared_ptr<DeviceIndex> getDeviceIndex ();

} /* namespace tcam */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @name tcam_get_camera_count
     * @return number of available devices
     */
    int tcam_get_camera_count ();

    /**
     * @name
     * @param ptr        - pointer to the array that shall be filled
     * @param array_size - size of array that ptr points to
     * @return number of devices copied to ptr; -1 on error
     */
    int tcam_get_camera_list (struct tcam_device_info* ptr, unsigned int array_size);

#ifdef __cplusplus
}
#endif

#endif /* _CAMERA_INDEX_H_ */
