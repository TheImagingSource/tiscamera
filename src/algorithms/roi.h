/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#pragma once

#include "../base_types.h"
#include "../compiler_defines.h"
#include "roi_base.h"
#include "roi_change_behavior.h"
#include "roi_preset.h"

#include <iostream>
#include <string>
#include <vector>

VISIBILITY_DEFAULT

namespace tcam::algorithms::roi
{
/**
 * The following are a c wrapper around the RegionOfinterest class
 */

// opaque object
struct ROI;
typedef struct ROI ROI;

ROI* create_roi(const tcam_image_size* min_size, const tcam_image_size* image_size);

void destroy_roi(ROI*);


roi_area get_roi(const ROI* roi);

bool roi_set_position(ROI* roi, unsigned int left, unsigned int top);
tcam_image_size roi_position(const ROI* roi);

bool roi_set_left(ROI* roi, unsigned int left);
unsigned int roi_left(const ROI* roi);

bool roi_set_top(ROI* roi, unsigned int top);
unsigned int roi_top(const ROI* roi);

bool roi_set_size(ROI* roi, unsigned int width, unsigned int height);
tcam_image_size roi_size(const ROI* roi);

bool roi_set_width(ROI* roi, unsigned int width);
unsigned int roi_width(const ROI* roi);

bool roi_set_height(ROI* roi, unsigned int height);
unsigned int roi_height(const ROI* roi);

/**
 * @return true if the roi fits the given image size
 */
bool roi_fits(const ROI* roi, const tcam_image_size& image_size);

bool roi_set_image_size(ROI* roi, const tcam_image_size& image_size);
tcam_image_size roi_image_size(const ROI* roi);

bool roi_set_minimal_size(ROI* roi, const tcam_image_size& min);
tcam_image_size roi_get_minimal_size(const ROI* roi);

void roi_set_preset(ROI* roi, ROI_PRESET preset);
ROI_PRESET roi_get_preset(const ROI* roi);

bool roi_extract(const ROI* roi, const tcam_image_buffer* image, tcam_image_buffer* roi_image);

bool roi_extract_view(const ROI* roi, const tcam_image_buffer* image, tcam_image_buffer* roi_image);

bool roi_fits_image(const tcam_image_size& size, const roi_area& roi);


byte* extract_roi(byte* image, const tcam_image_size& size, int bytes_per_pixel, const roi_area&);

} // namespace tcam::algorithms::roi

VISIBILITY_POP
