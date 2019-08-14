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

#include "RegionOfInterest.h"

#include "image_transform_base.h"

#include "math.h"

using namespace roi;

RegionOfInterest::RegionOfInterest (const tcam_image_size& min_size,
                                    const tcam_image_size& image_size,
                                    ROI_CHANGE_BEHAVIOR behavior,
                                    ROI_PRESET preset)
    : min_size_(min_size), image_size_(image_size), behavior_(behavior), preset_(preset)
{

    behavior_ = ROI_CHANGE_BEHAVIOR_RESET;

    preset_ = ROI_PRESET_FULL_SENSOR;
}


roi_area RegionOfInterest::get () const
{
    return roi_;
}


bool RegionOfInterest::set_position (unsigned int left, unsigned int top)
{
    if (!roi_fits_image(image_size_,
                        {left, top, roi_.width, roi_.height}))
    {
        return false;
    }

    roi_.left = left;
    roi_.top = top;

    set_preset(ROI_PRESET_CUSTOM_RECTANGLE);

    return true;
}


bool RegionOfInterest::set_top (unsigned int top)
{
    if (!roi_fits_image(image_size_,
                        {roi_.left, top, roi_.width, roi_.height}))
    {
        return false;
    }

    preset_ = ROI_PRESET_CUSTOM_RECTANGLE;

    roi_.top = top;
    set_preset(ROI_PRESET_CUSTOM_RECTANGLE);

    return true;
}


bool RegionOfInterest::set_left (unsigned int left)
{
    if (!roi_fits_image(image_size_,
                        {left, roi_.top, roi_.width, roi_.height}))
    {
        return false;
    }
    preset_ = ROI_PRESET_CUSTOM_RECTANGLE;

    roi_.left = left;

    return true;
}




bool RegionOfInterest::set_size (unsigned int width, unsigned int height)
{
    if (width < min_size_.width
        || height < min_size_.height)
    {
        return false;
    }

    if (!roi_fits_image(image_size_, {roi_.left, roi_.top, width, height}))
    {
        return false;
    }
    //preset_ = ROI_PRESET_CUSTOM_RECTANGLE;

    roi_.width = width;
    roi_.height = height;

    //apply_preset();

    return true;
}


bool RegionOfInterest::set_width (unsigned int width)
{
    if (width < min_size_.width)
    {
        return false;
    }
    if (!roi_fits_image(image_size_, {roi_.left, roi_.top, width, roi_.height}))
    {
        return false;
    }

    preset_ = ROI_PRESET_CUSTOM_RECTANGLE;

    roi_.width = width;

    set_preset(ROI_PRESET_CUSTOM_RECTANGLE);

    return true;
}


bool RegionOfInterest::set_height (unsigned int height)
{
    if (height < min_size_.height)
    {
        return false;
    }
    if (!roi_fits_image(image_size_, {roi_.left, roi_.top, roi_.width, height}))
    {
        return false;
    }

    preset_ = ROI_PRESET_CUSTOM_RECTANGLE;

    roi_.height = height;
    set_preset(ROI_PRESET_CUSTOM_RECTANGLE);
    return true;
}


bool RegionOfInterest::fits (const tcam_image_size& image_size) const
{
    return roi_fits_image(image_size, roi_);
}


bool RegionOfInterest::set_image_size (const tcam_image_size& image_size)
{
    auto old_image_size = image_size_;

    image_size_ = image_size;

    preset_ = ROI_PRESET_FULL_SENSOR;

    roi_area new_roi = {};

    if (!calculate_new_roi(old_image_size,
                           image_size_,
                           roi_,
                           new_roi,
                           behavior_,
                           cache_))
    {
        //printf("calculate_new_roi failed!\n");

        // TODO: reset roi
        /*
          roi_.top = 0;
          roi_.left = 0;
          roi_.width = image_size.width;
          roi_.height = image_size.height;
        */

        return false;
    }


    if (!roi_fits_image(image_size_, new_roi))
    {
        //printf("roi does not fit!\n");
        return false;
    }

    roi_ = new_roi;

    // not here
    // set_preset(ROI_PRESET_CUSTOM_RECTANGLE);
    // apply_preset();

    return true;
}


/**
 * @name calculate_new_roi
 * @brief calculate new roi definition based on new image size and behavior
 * @param(in) old_size - size the old image had
 * @param(in) new_size - new size the image will have
 * @param(in) old_roi - current roi definition
 * @param(out) new_roi - updated roi definition
 * @param(in) behavior - How the roi shall be changed. See ::ROI_CHANGE_BEHAVIOR
 * @param(in) cache - ROI cache object
 * @return bool if calculation was successful
 * @
 */
bool RegionOfInterest::calculate_new_roi (const tcam_image_size& /* old_size */,
                                          const tcam_image_size& new_size,
                                          const roi_area& /* old_roi */,
                                          roi_area& new_roi,
                                          ROI_CHANGE_BEHAVIOR behavior,
                                          roi_object& /* cache */)
{

    switch (behavior)
    {
        case ROI_CHANGE_BEHAVIOR_RESET:
        {
            new_roi.left = 0;
            new_roi.top = 0;
            new_roi.width = new_size.width;
            new_roi.height = new_size.height;
            return true;
        }
        case ROI_CHANGE_BEHAVIOR_UNDEFINED:
        default:
        {
            return false;
        }
    }
}

tcam_image_size calc_relative_position (const roi_cache& cache,
                                        const tcam_image_size& new_size)
{
    tcam_image_size s;

    s.width = round(new_size.width * cache.left_cache / 100);
    s.height = round(new_size.height * cache.top_cache / 100);

    return s;
}

tcam_image_size calc_relative_roi_size (const roi_cache& cache,
                                        const tcam_image_size& new_size)
{
    tcam_image_size s;

    s.width  = round(cache.width_cache * new_size.width / 100);
    s.height = round(cache.height_cache * new_size.height / 100);

    return s;
}


bool RegionOfInterest::set_minimal_size (const tcam_image_size& min)
{
    if (min.width > roi_.width || min.height > roi_.height)
    {
        if (!roi_fits_image(image_size_, {roi_.left, roi_.top,
                                          min.width, min.height}))
        {
            return false;
        }

        if (min.width > roi_.width)
        {
            roi_.width = min.width;
        }
        if (min.height > roi_.height)
        {
            roi_.height = min.height;
        }

    }

    min_size_ = min;

    return true;
}


void RegionOfInterest::set_preset (ROI_PRESET preset)
{
    preset_ = preset;

    apply_preset();
}


size_t RegionOfInterest::roi_buffer_size () const
{
    // TODO: format
    return (roi_.height * roi_.width);
}


/**
 * Creates a new image buffer by memcpying the ROI from the given buffer
 */
bool RegionOfInterest::copy_roi (const tcam_image_buffer& image,
                                 tcam_image_buffer& image_roi) const
{
// always first multiply bpp and then divide by 8 to fix formats with actual 12 bpp
    int fourcc = image.format.fourcc;
    int src_width = image.format.width;

    size_t bpp = img::get_bits_per_pixel(fourcc);

	size_t bytesPerLineIn = src_width * bpp / 8;
    size_t bytesPerLineOut = roi_.width * bpp / 8;

    int startRow = roi_.top;
    int endRow = roi_.top + roi_.height;

	byte* pIn = image.pData;

    size_t size = roi_buffer_size() * bpp / 8;
    byte* pOut = (byte*)malloc(size);

    pIn += startRow * bytesPerLineIn + roi_.left * bpp / 8;


    byte* po = pOut;
	for (int y = startRow; y < endRow; ++y)
	{
        std::memcpy(po, pIn, bytesPerLineOut);

		po += bytesPerLineOut;
		pIn += bytesPerLineIn;
	}

    image_roi = {};

    image_roi.pData = pOut;
    image_roi.length = size;

    image_roi.format = image.format;
    // format adjustments
    image_roi.format.width = roi_.width;
    image_roi.format.height = roi_.height;

    image_roi.pitch = roi_.width * img::get_bits_per_pixel(image_roi.format.fourcc) / 8;

    return true;
}

bool RegionOfInterest::extract_roi_view (const tcam_image_buffer& image,
                                         tcam_image_buffer& image_roi) const
{
    size_t bpp = img::get_bits_per_pixel(image.format.fourcc);

    image_roi.pData = image.pData + (roi_.top * image.format.width * bpp / 8
                               + roi_.left * bpp / 8);

    image_roi.pitch = roi_.width * bpp / 8;

    image_roi.format = image.format;
    image_roi.format.height = roi_.height;
    image_roi.format.width = roi_.width;
    return true;
}


void RegionOfInterest::calculate_roi_based_on_cache ()
{
    roi_area new_roi = {};

    if (cache_.left_cache != 0)
    {
        new_roi.left = (float)image_size_.width / 100 * cache_.left_cache;
    }
    else
    {
        new_roi.left = 0;
    }

    if (cache_.width_cache != 0)
    {
        new_roi.width = (float)image_size_.width / 100 * cache_.width_cache;
    }
    else
    {
        new_roi.width = 0;
    }

    if (cache_.top_cache != 0)
    {
        new_roi.top = (float)image_size_.height / 100 * cache_.top_cache;
    }
    else
    {
        new_roi.top = 0;
    }

    if (cache_.height_cache != 0)
    {
        new_roi.height = (float)image_size_.height / 100 * cache_.height_cache;
    }
    else
    {
        new_roi.height = 0;
    }



    roi_ = new_roi;
}


roi_cache RegionOfInterest::fill_cache (const tcam_image_size& image,
                                        const roi_area& roi)
{
    roi_cache cache = {};

    cache.left_cache = (double)roi.left / image.width * 100;
    cache.top_cache  = (double)roi.top  / image.height * 100;

    cache.width_cache  = (double)roi.width / image.width * 100;
    cache.height_cache = (double)roi.height / image.height * 100;

    return std::move(cache);
}


void RegionOfInterest::apply_preset ()
{
    /*
      for 2594x1944

      25%:  648x486
      50%: 1296x972

     */


    switch (preset_)
    {
        case ROI_PRESET_FULL_SENSOR:
        {
            cache_ = { 0, 0, 100, 100 };
            break;
        }
        case ROI_PRESET_CENTER_50:
        {
            cache_ = { 25, 25, 50, 50 };
            break;
        }
        case ROI_PRESET_CENTER_25:
        {
            cache_ = { 50.0-12.5, 50-12.5, 25, 25 };
            break;
        }
        case ROI_PRESET_BOTTOM_HALF:
        {
            cache_ = { 0, 50, 100, 50 };
            break;
        }
        case ROI_PRESET_TOP_HALF:
        {
            cache_ = { 0, 0, 100, 50 };
            break;
        }
        case ROI_PRESET_CUSTOM_RECTANGLE:
        default:
        {
            return;
        }
    }
    calculate_roi_based_on_cache();

}
