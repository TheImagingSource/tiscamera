
#ifndef TCAM_GSTTCAMBASE_H
#define TCAM_GSTTCAMBASE_H

#include <stdint.h>


const char* tcam_fourcc_to_gst_0_10_caps_string (uint32_t fourcc);

uint32_t tcam_fourcc_from_gst_0_10_caps_string (const char* name, const char* format);

const char* tcam_fourcc_to_gst_1_0_caps_string (uint32_t);

uint32_t tcam_fourcc_from_gst_1_0_caps_string (const char* name, const char* format);

#endif /* TCAM_GSTTCAMBASE_H */
