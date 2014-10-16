
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <vector>
#include <string>
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
 
/* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <linux/media.h>

// #include "tis_video.h"
#include "utils.h"
#include "tis_logging.h"
#include "device_discovery.h"
#include "tis_logging.h"

#if HAVE_ARAVIS
#include "arv.h"
#endif

#if HAVE_UDEV
#include <libudev.h>
#endif

// TODO: something makes list return 0 cameras when aravis is not included

int tis_get_camera_count ()
{
    int count = 0;

#if HAVE_ARAVIS
    count += tis_get_gige_camera_count();
#endif
    
    count += tis_get_usb_camera_count();
    
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

    unsigned int usb_count = tis_get_usb_camera_list(info_vec.data(), count);

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


int tis_get_usb_camera_count ()
{
    // TODO: change to udev

    int count = 0;

    glob_t gs;


    int ret = glob("/dev/video*", 0, NULL, &gs);

    if (ret != 0)
    {
        // TODO: error
        return 0;
    }

    count = gs.gl_pathc;
    
    globfree(&gs);

    return count;
}


int tis_get_usb_camera_list (struct tis_device_info* ptr, unsigned int array_size)
{
    int camera_cnt = 0;
    int camera_count = 0;

    struct udev* udev = udev_new();
    if (!udev)
    {
        return -1;
    }

    /* Create a list of the devices in the 'video4linux' subsystem. */
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* dev_list_entry;

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char* path;
        char needed_path[100];

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/

        /* we need to copy the devnode (/dev/videoX) before the path
           is changed to the path of the usb device behind it (/sys/class/....) */
        strcpy(needed_path, udev_device_get_devnode(dev));

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        if (!dev)
        {
            return -1;
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passed into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the directory which represents
           the USB device. Note that USB strings are Unicode, UCS2
           encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */

        // TODO: no hard coded numbers find more general approach
        if (strcmp(udev_device_get_sysattr_value(dev, "idVendor"), "199e") == 0)
        {
            ptr->type = TIS_DEVICE_TYPE_V4L2;
            strcpy(ptr->identifier, needed_path);
            strcpy(ptr->name, udev_device_get_sysattr_value(dev, "product"));
            strcpy(ptr->serial_number, udev_device_get_sysattr_value(dev, "serial"));

            camera_cnt++;
            ptr++;
        }

        udev_device_unref(dev);
    }

    camera_count = camera_cnt;
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    return camera_count;
}


#if HAVE_ARAVIS

int tis_get_gige_camera_count ()
{
    arv_update_device_list();

    return arv_get_n_devices();
}


int tis_get_gige_camera_list (struct tis_device_info* ptr, unsigned int array_size)
{
    arv_update_device_list ();

    unsigned int number_devices = arv_get_n_devices();

    if (number_devices > array_size)
    {
        // TODO: errno missing.
        return -1;
    }

    unsigned int counter = 0;

    for (unsigned int i = 0; i < number_devices; ++i)
    {
        std::string name = arv_get_device_id(i);
        memcpy(ptr->identifier, name.c_str(), name.size());

        ArvCamera* cam = arv_camera_new(name.c_str());

        ptr->type = TIS_DEVICE_TYPE_ARAVIS;
        const char* n =  arv_camera_get_model_name(cam);

        if (n != NULL)
        {
            strncpy(ptr->name, n, sizeof(ptr->name));
        }
        else
        {
            tis_log(TIS_LOG_WARNING, "Unable to determine model name.");
        }
        size_t t = name.find("-");

        if (t != std::string::npos)
        {
            strcpy(ptr->serial_number, name.substr((t+1)).c_str());
        }

        if (counter < array_size)
        {
            ptr++;
            ++counter;
        }
        else
        {

        }

        g_object_unref(cam);

    }

    return counter;
}

#endif /* HAVE_ARAVIS */
