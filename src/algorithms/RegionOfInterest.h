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

#include "base_types.h"

#include "roi_preset.h"
#include "roi_change_behavior.h"
#include "roi_base.h"

#include "roi.h"

#include <thread>

namespace roi
{

class RegionOfInterest
{

private:

    roi_area roi_ = {0, 0, 16, 16};

    tcam_image_size min_size_ = {16, 16};
    tcam_image_size step_size_ = {2, 2};
    tcam_image_size image_size_ ;

    roi_cache cache_ = {0, 0, 0, 0};

    ROI_CHANGE_BEHAVIOR behavior_ = ROI_CHANGE_BEHAVIOR_RESET;
    ROI_PRESET preset_ = ROI_PRESET_FULL_SENSOR;


    // auto verify_top = std::bind(roi_fits_image, {roi_.left, _1, roi_.width, roi_height});

public:

    explicit RegionOfInterest (const tcam_image_size& min_size,
                               const tcam_image_size& image_size,
                               ROI_CHANGE_BEHAVIOR behavior=ROI_CHANGE_BEHAVIOR_RESET,
                               ROI_PRESET preset=ROI_PRESET_FULL_SENSOR);

    RegionOfInterest (const RegionOfInterest&) = default;
    RegionOfInterest& operator= (const RegionOfInterest&) = default;

    roi_area get () const;

    /**
     *
     */
    bool set_position (unsigned int left, unsigned int top);

    tcam_image_size position () const
    {
        return {roi_.left, roi_.top};
    };

    /**
     *
     */
    bool set_left (unsigned int left);

    unsigned int left () const
    {
        return roi_.left;
    }

    /**
     *
     */
    bool set_top (unsigned int top);

    unsigned int top () const
    {
        return roi_.top;
    };

    /**
     *
     */
    bool set_size (unsigned int width, unsigned int height);


    tcam_image_size size () const
    {
        return { roi_.width, roi_.height };
    };

    bool set_width (unsigned int width);

    unsigned int width () const
    {
        return roi_.width;
    }

    bool set_height (unsigned int height);

    unsigned int height () const
    {
        return roi_.height;
    }

    /**
     * @return true if the roi fits the given image size
     */
    bool fits (const tcam_image_size& image_size) const;

    /**
     *
     */
    bool set_image_size (const tcam_image_size& image_size);

    tcam_image_size image_size () const
    {
        return image_size_;
    }

    bool calculate_new_roi (const tcam_image_size& old_size,
                            const tcam_image_size& new_size,
                            const roi_area& old_roi,
                            roi_area& new_roi,
                            ROI_CHANGE_BEHAVIOR behavior,
                            roi_object& cache);


    bool set_minimal_size (const tcam_image_size& min);
    tcam_image_size minimal_size () const
    {
        return min_size_;
    };

    void set_preset (ROI_PRESET preset);
    ROI_PRESET preset () const
    {
        return preset_;
    };

    /**
     *
     * @return size_t of the roi buffer
     */
    size_t roi_buffer_size () const;

    /**
     * Creates a new image buffer by memcpying the ROI from the given buffer
     * The user takes ownership of the allocated roi buffer
     */
    bool copy_roi (const tcam_image_buffer& image,
                   tcam_image_buffer& image_roi) const;

private:

    /**
     * Recalculate the roi_ values based on the current cache_ settings
     */
    void calculate_roi_based_on_cache ();

    roi_cache fill_cache (const tcam_image_size& image, const roi_area& roi);

    void apply_preset ();
    void update_roi_preset ();

}; // class RegionOfinterest


} /* namespace roi */
