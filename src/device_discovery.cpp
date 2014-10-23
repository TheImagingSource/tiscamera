
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <vector>
#include <string>
#include <memory>
#include <mutex>

#include "utils.h"
#include "logging.h"
#include "device_discovery.h"

#if HAVE_ARAVIS
#include "aravis_utils.h"
#endif

#if HAVE_USB
#include "v4l2_utils.h"
#endif

using namespace tis_imaging;


std::vector<CaptureDevice> DeviceIndex::getDeviceList () const
{
    return device_list;
}


DeviceIndex::DeviceIndex ()
    : continue_thread(false), wait_period(2000000)
{
    continue_thread = true;
    work_thread = std::thread(&DeviceIndex::run, this);
}


DeviceIndex::~DeviceIndex ()
{
    if (continue_thread)
    {
        continue_thread = false;
        work_thread.join();
    }
}


void DeviceIndex::updateDeviceList ()
{
    std::vector<CaptureDevice> tmp_dev_list (10);

#if HAVE_ARAVIS
    auto aravis_dev_list = get_aravis_device_list();
    tmp_dev_list.insert(aravis_dev_list.begin(), aravis_dev_list.end(), tmp_dev_list.end());
#endif

#if HAVE_USB
    auto v4l2_dev_list = get_v4l2_device_list();
    tmp_dev_list.insert(v4l2_dev_list.begin(), v4l2_dev_list.end(), tmp_dev_list.end());
#endif

    std::mutex mtx;

    mtx.lock();

    device_list.clear();

    device_list.insert(tmp_dev_list.begin(), tmp_dev_list.end(), device_list.end());

    mtx.unlock();
}


void DeviceIndex::run ()
{
    while (continue_thread)
    {
        updateDeviceList();
        usleep(wait_period);
    }
}


std::shared_ptr<DeviceIndex> getDeviceIndex ()
{
    return std::make_shared<DeviceIndex>();
}


// TODO: something makes list return 0 cameras when aravis is not included

int tis_get_camera_count ()
{
    int count = 0;

#if HAVE_ARAVIS
    count += tis_get_gige_camera_count();
#endif

#if HAVE_USB
    count += tis_get_usb_camera_count();
#endif

    return count;
}


int tis_get_camera_list (struct tis_device_info* user_list, unsigned int array_size)
{
    memset(user_list, 0, sizeof(struct tis_device_info)*array_size);

    unsigned int count = tis_get_camera_count();

    if (count > array_size)
    {
        // TODO: errno missing
        return -1;
    }

    std::vector<struct tis_device_info> info_vec(count);

    unsigned int usb_count = 0;

#if HAVE_USB
    usb_count = tis_get_usb_camera_list(info_vec.data(), count);
#endif

    unsigned int gige_count = 0;

#if HAVE_ARAVIS
    gige_count = tis_get_gige_camera_list(&(info_vec.data()[usb_count]), array_size - usb_count);
#endif

    if (usb_count + gige_count != count)
    {
        return -1;
    }

    memcpy(user_list, info_vec.data(), count * sizeof(tis_device_info));

    return count;
}
