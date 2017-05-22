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

#include "tcam.h"
#include "tcamgststrings.h"

#include <gst/gst.h>

#ifdef __cplusplus
extern "C"
{
#endif

GstElement* tcam_gst_find_camera_src (GstElement* element);

/*
  extracts video/x-raw from caps and checks if only mono is present
*/
bool tcam_gst_raw_only_has_mono (const GstCaps* src_caps);


bool tcam_gst_is_fourcc_bayer (const unsigned int fourcc);


bool tcam_gst_is_fourcc_rgb (const unsigned int fourcc);


bool tcam_gst_fixate_caps (GstCaps* caps);


GstCaps* tcam_gst_find_largest_caps (const GstCaps* incoming);

// bool gst_buffer_to_tcam_image_buffer(GstBuffer* buffer, tcam_image_buffer* buf);

#ifdef __cplusplus
}
#endif

#endif /* TCAM_GSTTCAMBASE_H */
