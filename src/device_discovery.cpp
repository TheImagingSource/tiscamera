
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
#include "arv.h"
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
