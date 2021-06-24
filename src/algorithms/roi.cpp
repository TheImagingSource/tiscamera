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

#include "roi.h"

#include "RegionOfInterest.h"

#include <cstdlib>
#include <math.h>

using namespace roi;

namespace tcam::algorithms::roi
{
ROI* create_roi(const tcam_image_size* min_size, const tcam_image_size* image_size)
{
    return reinterpret_cast<ROI*>(new RegionOfInterest(*min_size, *image_size));
}


void destroy_roi(ROI* r)
{
    delete reinterpret_cast<RegionOfInterest*>(r);
}


roi_area get_roi(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->get();
}


bool roi_set_position(ROI* roi, unsigned int left, unsigned int top)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_position(left, top);
}


tcam_image_size roi_position(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->position();
}


bool roi_set_left(ROI* roi, unsigned int left)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_left(left);
}


unsigned int roi_left(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->left();
}


bool roi_set_top(ROI* roi, unsigned int top)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_top(top);
}


unsigned int roi_top(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->top();
}


bool roi_set_size(ROI* roi, unsigned int width, unsigned int height)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_size(width, height);
}


tcam_image_size roi_size(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->size();
}


bool roi_set_width(ROI* roi, unsigned int width)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_width(width);
}


unsigned int roi_width(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->width();
}


bool roi_set_height(ROI* roi, unsigned int height)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_height(height);
}


unsigned int roi_height(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->height();
}


bool roi_fits(const ROI* roi, const tcam_image_size& image_size)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->fits(image_size);
}


bool roi_set_image_size(ROI* roi, const tcam_image_size& image_size)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_image_size(image_size);
}


tcam_image_size roi_image_size(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->image_size();
}


bool roi_set_minimal_size(ROI* roi, const tcam_image_size& min)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_minimal_size(min);
}


tcam_image_size roi_get_minimal_size(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->minimal_size();
}


void roi_set_preset(ROI* roi, ROI_PRESET preset)
{
    return reinterpret_cast<RegionOfInterest*>(roi)->set_preset(preset);
}


ROI_PRESET roi_get_preset(const ROI* roi)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->preset();
}


bool roi_extract(const ROI* roi, const tcam_image_buffer* image, tcam_image_buffer* roi_image)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->copy_roi(*image, *roi_image);
}


bool roi_extract_view(const ROI* roi, const tcam_image_buffer* image, tcam_image_buffer* roi_image)
{
    return reinterpret_cast<const RegionOfInterest*>(roi)->extract_roi_view(*image, *roi_image);
}


bool roi_fits_image(const tcam_image_size& size, const roi_area& roi)
{
    if ((roi.left + roi.width <= size.width) && (roi.top + roi.height <= size.height))
    {
        return true;
    }
    return false;
}

} // namespace tcam::algorithms::roi
