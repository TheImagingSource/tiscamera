
#include "tcam_c.h"

#include "tcam.h"
#include "Error.h"

using namespace tcam;

/* error / debug */

int tcam_last_errno ()
{
    return getError().getErrno();
}


const char* tcam_last_error_messsage ()
{
    return getError().getString().c_str();
}


/* device discovery / watchdog */

tcam_device_index* tcam_create_device_index ()
{
    return (tcam_device_index*)new DeviceIndex();
}


void tcam_destroy_device_index (tcam_device_index* index)
{
    delete reinterpret_cast<DeviceIndex*>(index);
}


int tcam_device_index_get_device_count (tcam_device_index* index)
{
    return reinterpret_cast<DeviceIndex*>(index)->getDeviceList().size();
}


int tcam_device_index_get_device_infos (tcam_device_index* index,
                                        tcam_device_info* arr,
                                        size_t size)
{
    auto vec = reinterpret_cast<DeviceIndex*>(index)->getDeviceList();

    if (vec.size() < size)
    {
        setError(Error("Array is to small to contain all devices", EFAULT));
        return -1;
    }

    int i = 0;
    for (const auto& v : vec)
    {
        arr[i] = v.getInfo();
        i++;
    }

    return vec.size();
}


/* image source */

tcam_capture_device* tcam_create_new_capture_device (const tcam_device_info* info)
{
    return (tcam_capture_device*) new CaptureDevice(DeviceInfo(*info));
}


void tcam_destroy_capture_device (tcam_capture_device* source)
{
    delete reinterpret_cast<CaptureDevice*>(source);
}


/* device related */

bool tcam_capture_device_is_device_open (tcam_capture_device* source)
{
    return reinterpret_cast<CaptureDevice*>(source)->isDeviceOpen();
}


/* property related*/

int tcam_capture_device_get_property_count (const tcam_capture_device* source)
{
    return reinterpret_cast<const CaptureDevice*>(source)->getAvailableProperties().size();
}


int tcam_capture_device_get_properties (const tcam_capture_device* source,
                                        const tcam_camera_property* properties)
{}


bool tcam_capture_device_set_property (tcam_capture_device* source,
                                       const tcam_camera_property* property)
{
    auto vec = reinterpret_cast<CaptureDevice*>(source)->getAvailableProperties();

    for (auto& v :vec)
    {
        if (v.getID() == property->id)
        {
            //return v.set
        }
    }
    setError(Error("No such property found", ENOENT));
    return false;
}


/* video format related */

int tcam_capture_device_get_image_format_description_count (const tcam_capture_device* source)
{
    if (source == nullptr)
    {
        return -1;
    }

    return reinterpret_cast<const CaptureDevice*>(source)->getAvailableVideoFormats().size();
}


int tcam_capture_device_get_image_format_description (const tcam_capture_device* source,
                                                      tcam_video_format_description* arr,
                                                      const size_t size)
{

    auto vec = reinterpret_cast<const CaptureDevice*>(source)->getAvailableVideoFormats();

    if (vec.size() > size)
    {
        return -1;
    }

    int i = 0;
    for (const auto& v : vec)
    {
        arr[i] = v.getFormatDescription();
        i++;
    }

    return vec.size();
}


bool tcam_capture_device_set_image_format (tcam_capture_device* source,
                                           const tcam_video_format* format)
{

}


bool tcam_capture_device_get_image_format (tcam_capture_device* source,
                                           tcam_video_format* format)
{

}


/* streaming functions */

bool tcam_capture_device_start_stream (tcam_capture_device* source,
                                       tcam_image_callback callback,
                                       void* user_data)
{

}


bool tcam_capture_device_stop_stream (tcam_capture_device* source)
{
    return reinterpret_cast<CaptureDevice*>(source)->stopStream();
}
