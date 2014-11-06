



#ifndef CAMERA_INDEX_H
#define CAMERA_INDEX_H

#include "base_types.h"
#include "DeviceInfo.h"

#include <vector>
#include <thread>

namespace tcam
{

class DeviceIndex
{

public:

    DeviceIndex ();

    ~DeviceIndex ();

    std::vector<DeviceInfo> getDeviceList () const;

private:

    bool continue_thread;
    unsigned int wait_period;
    std::thread work_thread;

    std::vector<DeviceInfo> device_list;

    std::vector<dev_callback> callbacks;

    void updateDeviceList ();

    void run ();

    void fire_device_lost (const DeviceInfo& d);

};


std::shared_ptr<DeviceIndex> getDeviceIndex ();

} /* namespace tcam */

#endif /* CAMERA_INDEX_H */
