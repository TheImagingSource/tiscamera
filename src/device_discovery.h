



#ifndef _CAMERA_INDEX_H_
#define _CAMERA_INDEX_H_

#include "base_types.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @name tis_get_camera_count
     * @return number of available devices
     */
    int tis_get_camera_count ();
    
    /**
     * @name
     * @param ptr        - pointer to the array that shall be filled
     * @param array_size - size of array that ptr points to
     * @return number of devices copied to ptr; -1 on error
     */
    int tis_get_camera_list (struct tis_device_info* ptr, unsigned int array_size);

#if HAVE_USB

    /**
     * @name tis_get_camera_count
     * @return number of available usb devices
     */
    int tis_get_usb_camera_count ();
    
    /**
     * @name
     * @param ptr        - pointer to the array that shall be filled
     * @param array_size - size of array that ptr points to
     * @return number of devices copied to ptr; -1 on error
     */
    int tis_get_usb_camera_list (struct tis_device_info* ptr, unsigned int array_size);

#endif /* HAVE_USB */

#if HAVE_ARAVIS

    /**
     * @name tis_get_camera_count
     * @return number of available gige devices
     */
    int tis_get_gige_camera_count ();

    /**
     * @name
     * @param ptr        - pointer to the array that shall be filled
     * @param array_size - size of array that ptr points to
     * @return number of devices copied to ptr; -1 on error
     */
    int tis_get_gige_camera_list (struct tis_device_info* ptr, unsigned int array_size);

#endif /* HAVE_ARAVIS */
    
#ifdef __cplusplus
}
#endif

#endif /* _CAMERA_INDEX_H_ */



