
#ifndef TCAM_PUBLIC_UTILS_H
#define TCAM_PUBLIC_UTILS_H

#include <stdint.h>

#if __cplusplus
extern "C"
{
#endif

const char* tcam_fourcc_to_description (uint32_t fourcc);


uint32_t tcam_description_to_fourcc (const char* description);


#if __cplusplus
}
#endif



#endif /* TCAM_PUBLIC_UTILS_H */
