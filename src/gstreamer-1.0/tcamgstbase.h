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


/**
 * Change the name of the given caps to name
 */
void gst_caps_change_name (GstCaps* caps, const char* name);


/**
 * Create intersecting caps between bayer and gst_caps_change_name(raw, "video/x-bayer")
 */
GstCaps* bayer_transform_intersect (const GstCaps* bayer, const GstCaps* raw);


/**
 * caps only contain bayer
 */
bool gst_caps_are_bayer_only (const GstCaps* caps);


GstCaps* tcam_gst_find_largest_caps (const GstCaps* incoming);


bool contains_bayer (const GstCaps* caps);

/**
 * @param elementname - name of the GstElement that shall be queried
 * @param padname - name of the static pad that shall be queried
 * @return GstCaps from static pad with name padname, can return nullptr
 */
GstCaps* get_caps_from_element (const char* elementname, const char* padname);


/**
 * @param available_caps - caps the source offers
 * @param wanted_caps - caps the sink wants, if null available_caps will be returned
 * @param requires_conversion - will be set to true when a conversion element is required
 *
 * @return possible caps for the source
 */
GstCaps* find_input_caps (const GstCaps* available_caps,
                          const GstCaps* wanted_caps,
                          bool& requires_conversion);


bool fill_structure_fixed_resolution (GstStructure* structure,
                                      const tcam::VideoFormatDescription& format,
                                      const tcam_resolution_description& res);


GstCaps* convert_videoformatsdescription_to_caps (std::vector<tcam::VideoFormatDescription> descriptions);

#ifdef __cplusplus
}
#endif

#endif /* TCAM_GSTTCAMBASE_H */
