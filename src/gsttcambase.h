/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_GSTTCAMBASE_H
#define TCAM_GSTTCAMBASE_H

#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif

const char* tcam_fourcc_to_gst_0_10_caps_string (uint32_t fourcc);

uint32_t tcam_fourcc_from_gst_0_10_caps_string (const char* name, const char* format);

const char* tcam_fourcc_to_gst_1_0_caps_string (uint32_t);

uint32_t tcam_fourcc_from_gst_1_0_caps_string (const char* name, const char* format);

#ifdef __cplusplus
}
#endif

#endif /* TCAM_GSTTCAMBASE_H */
