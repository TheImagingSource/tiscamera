
#ifndef TCAM_PUBLIC_UTILS_H
#define TCAM_PUBLIC_UTILS_H

#include <stdint.h>

#include <stddef.h> /* size_t */

#include "base_types.h"

#if __cplusplus
extern "C"
{
#endif

const char* tcam_fourcc_to_description (uint32_t fourcc);


uint32_t tcam_description_to_fourcc (const char* description);


/**
 * @param format - format description that shall be used
 * @param n_buffers - number of buffers that shall be allocated
 * @return pointer to the first image buffer
 */
struct tcam_image_buffer* tcam_allocate_image_buffers (const struct tcam_video_format* format,
                                                       size_t n_buffers);

/**
 * @param ptr - pointer to the first buffer that shall be freed
 * @param n_buffers - number of buffers that shall be freed
 */
void tcam_free_image_buffers (struct tcam_image_buffer* ptr, size_t n_buffer);


#if __cplusplus
}
#endif



#endif /* TCAM_PUBLIC_UTILS_H */
