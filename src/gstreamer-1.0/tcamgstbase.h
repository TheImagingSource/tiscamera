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

#include <string>
#include <vector>

#include <gst/gst.h>

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * returns true if serial and type have been found
 * returns false if only serial has been found
 */
bool separate_serial_and_type (const std::string& input,
                               std::string& serial,
                               std::string& type);

GstElement* tcam_gst_find_camera_src (GstElement* element);


std::string get_plugin_version (const char* plugin_name);


std::vector<std::string> gst_list_to_vector (const GValue* gst_list);

/*
  extracts video/x-raw from caps and checks if only mono is present
*/
bool tcam_gst_raw_only_has_mono (const GstCaps* src_caps);


bool tcam_gst_is_fourcc_bayer (const unsigned int fourcc);


bool tcam_gst_is_bayer8_string (const char* fourcc);


bool tcam_gst_is_bayer10_string (const char* format_string);


bool tcam_gst_is_bayer10_packed_string (const char* format_string);


bool tcam_gst_is_bayer12_string (const char* format_string);


bool tcam_gst_is_bayer12_packed_string (const char* format_string);


bool tcam_gst_is_bayer16_string (const char* format_string);


bool tcam_gst_is_fourcc_rgb (const unsigned int fourcc);


bool tcam_gst_is_polarized (const unsigned int fourcc);


bool tcam_gst_contains_bayer_8_bit (const GstCaps* caps);


bool tcam_gst_contains_bayer_10_bit (const GstCaps* caps);


bool tcam_gst_contains_bayer_12_bit (const GstCaps* caps);


bool tcam_gst_contains_bayer_16_bit (const GstCaps* caps);


bool tcam_gst_fixate_caps (GstCaps* caps);

std::vector<std::string> index_caps_formats (GstCaps* caps);

std::vector<uint32_t> index_caps_fourcc (GstCaps* caps);


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


/**
 * Find the caps with the largest resolution and the highest framerate.
 * The format that will be selected is determined through internal ranking.
 * @param incoming - GstCaps from which to select
 * @return pointer to the largest caps, nullptr on error user has ownership
 */
GstCaps* tcam_gst_find_largest_caps (const GstCaps* incoming);


bool contains_bayer (const GstCaps* caps);

bool contains_jpeg (const GstCaps* caps);


/**
 * @param elementname - name of the GstElement that shall be queried
 * @param padname - name of the static pad that shall be queried
 * @return GstCaps from static pad with name padname, can return nullptr
 */
GstCaps* get_caps_from_element_name (const char* elementname, const char* padname);
GstCaps* get_caps_from_element (GstElement* element, const char* padname);


bool tcam_gst_can_intersect_simple(const GstCaps *caps, const gchar *capsstring);

/**
 * @param available_caps - caps the source offers
 * @param wanted_caps - caps the sink wants, if null available_caps will be returned
 * @param requires_bayertransform(out) - will be set to true when the tcambayertransform element is required
 * @param requires_bayer2rgb(out) - will be set to true when the bayer2rgb element is required
 * @param requires_vidoeconvert(out) - will be set to true when the videoconvert element is required
 * @param requires_jpegconvert(out) - will be set to true when the jpegdec element is required
 * @param use_dutils(in) - false when dutils shall be ignored
 *
 * @return possible caps for the source
 */
GstCaps* find_input_caps (GstCaps* available_caps,
                          GstCaps* wanted_caps,
                          bool& requires_bayertransform,
                          bool& requires_bayer2rgb,
                          bool& requires_vidoeconvert,
                          bool& requires_jpegconvert,
                          bool& requires_dutils,
                          bool use_dutils
);


bool fill_structure_fixed_resolution (GstStructure* structure,
                                      const tcam::VideoFormatDescription& format,
                                      const tcam_resolution_description& res);


GstCaps* convert_videoformatsdescription_to_caps (const std::vector<tcam::VideoFormatDescription>& descriptions);


/**
 * @param desc - vector of VideoFormatDescriptions that shall be converted
 * @param str(out) - string containing the converted caps
 * @return true if desc could be converted
 */
bool videoformatsdescription_to_gst_caps_string (const std::vector<tcam::VideoFormatDescription>& descriptions,
                                                 std::string& str);


bool gst_caps_to_tcam_video_format (GstCaps* caps, struct tcam_video_format* format);

bool gst_buffer_to_tcam_image_buffer (GstBuffer* buffer, GstCaps* caps, tcam_image_buffer* image);


int calc_pitch (int fourcc, int width);

#ifdef __cplusplus
}
#endif

#endif /* TCAM_GSTTCAMBASE_H */
