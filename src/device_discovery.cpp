
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <vector>
#include <string>
#include <dirent.h>
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
#include "logging.h"
#include "device_discovery.h"

#if HAVE_ARAVIS
#include "aravis_utils.h"
#endif

#if HAVE_USB
#include "v4l2_utils.h"
#endif

using namespace tis_imaging;

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
