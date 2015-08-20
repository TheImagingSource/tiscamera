




#ifndef TCAM_C_H
#define TCAM_C_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "base_types.h"
#include "public_utils.h"


    /* error / debug */

    /**
     * Retrieve the last known error number
     * @return errno of the last error
     */
    int tcam_last_errno ();


    /**
     * Retriever last known error message.
     * @return string containing the last error; NULL when no error available
     */
    const char* tcam_last_error_messsage ();


    /* device discovery / watchdog */

    struct tcam_device_index;
    typedef struct tcam_device_index tcam_device_index;


    /**
     * Create a device indexer
     * @return pointer to opaque object; NULL on error
     */
    tcam_device_index* tcam_create_device_index ();


    /**
     *
     * @param index - device_index that shall be destroyed
     */
    void tcam_destroy_device_index (tcam_device_index* index);


    /**
     * Description for tcam_device_index_get_device_count.
     *
     * @param index <doc>
     * @return number of found devices; -1 on error
     */
    int tcam_device_index_get_device_count (tcam_device_index* index);


    /**
     * Description for tcam_device_index_get_device_infos.
     *
     * @param[in]  index - index to use
     * @param[out] array - pointer to array tha shall be filled
     * @param[in]  size  - size of the given array
     * @return number of devices copied; -1 on error
     */
    int tcam_device_index_get_device_infos (tcam_device_index* index,
                                            struct tcam_device_info* array,
                                            size_t size);


    /* image source */

    struct tcam_capture_device;
    typedef struct tcam_capture_device tcam_capture_device;


    /**
     * @return newly created capture_device; NULL on error
     */
    tcam_capture_device* tcam_create_new_capture_device (const struct tcam_device_info* info);


    /**
     *
     */
    void tcam_destroy_capture_device (tcam_capture_device* source);


    /* device related */


    /**
     * get info about the currently open device
     * @param[in]  source - capture_device
     * @param[out] info   - tcam_device_info about currently opened device
     * @return false on error
     */
    bool tcam_capture_device_get_device_info (struct tcam_capture_device* source,
                                              struct tcam_device_info* info);

    /**
     * Description for tcam_capture_device_load_configuration.
     * @param[in] source <doc>
     * @param[in] filename <doc>
     * @return <doc>
     */
    bool tcam_capture_device_load_configuration (tcam_capture_device* source, const char* filename);


    /**
     * Description for tcam_capture_device_save_confguration.
     * @param[in] source <doc>
     * @param[in] filename <doc>
     * @return <doc>
     */
    bool tcam_capture_device_save_confguration (const tcam_capture_device* source, const char* filename);


    /* property related */

    /**
     * Description for tcam_capture_device_get_properties_count.
     * @param source - device whichs properties shall be read
     * @return number of available properties; -1 on error
     */
    int tcam_capture_device_get_properties_count (tcam_capture_device* source);


    /**
     * Retrieve tcam_capture_device properties
     * @param[in] source - device whichs properties shall be read
     * @param[out] array  - property collection that shall be filled
     * @param[in] size   - size of the given array
     * @return number of elements copied; -1 on error
     */
    int tcam_capture_device_get_properties (const tcam_capture_device* source,
                                            struct tcam_device_property* array,
                                            const size_t size);


    bool  tcam_capture_device_find_property (tcam_capture_device* source,
                                             enum TCAM_PROPERTY_ID id,
                                             struct tcam_device_property* property);
    /**
     *
     */
    int tcam_capture_device_set_property (tcam_capture_device* source,
                                          const struct tcam_device_property* property);


    /* video format related */

    /**
     * Description for tcam_capture_device_get_image_format_description_count.
     * @param source <doc>
     * @return number of available format descriptions
     */
    int tcam_capture_device_get_image_format_descriptions_count (const tcam_capture_device* source);


    /**
     * Description for tcam_capture_device_get_image_format_description.
     * @param[in]  source - source for which descriptions shall be queried
     * @param[out] array  - container into which descriptions shall be copied
     * @param[in]  size   - size of array
     * @return number of elements copied; -1 on error
     */
    int tcam_capture_device_get_image_format_descriptions (const tcam_capture_device* source,
                                                           struct tcam_video_format_description* array,
                                                           const size_t size);

    /**
     * Description for tcam_capture_device_set_image_format.
     * @param[in] source <doc>
     * @param[in] format - format that shall be used
     * @return true on success
     */
    bool tcam_capture_device_set_image_format (tcam_capture_device* source,
                                               const struct tcam_video_format* format);


    /**
     * Description for tcam_capture_device_get_image_format.
     * @param[in] source -
     * @param[out] format - format container that shall be filled
     * @return true on success
     */
    bool tcam_capture_device_get_image_format (tcam_capture_device* source,
                                               struct tcam_video_format* format);


    /* streaming functions */

    /* callback that will be used to push images to the user */
    typedef void (*tcam_image_callback)(const struct tcam_image_buffer* buffer, void* user_data);

    struct stream_obj;
    typedef struct stream_obj stream_obj;

    /**
     * @param[in] source - capture_device that shall be used
     * @param[in] cb     - function that shall be called for new images
     * @param[in] data   - data that shall be given to data
     * @return true if stream could be started
     */
    stream_obj* tcam_capture_device_start_stream (tcam_capture_device* source,
                                                 tcam_image_callback cb,
                                                 void* data);

    /**
     * Stops image capture in the device
     * Images that are in the pipeline will still be transmitted until pipeline is empty
     * @param[in] source
     * @return true on success
     */
    bool tcam_capture_device_stop_stream (tcam_capture_device* source);


#ifdef __cplusplus
}
#endif

#endif /* TCAM_C_H */
