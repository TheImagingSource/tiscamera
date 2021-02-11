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

// This file contains all base structs, etc. for region of interest

#ifndef byte
typedef unsigned char byte;
#endif

/**
 * @name roi_area
 * @brief description of a region of interest with coordinates and width/height
 */
typedef struct roi_area
{
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;

    bool operator==(const struct roi_area& other) const
    {
        if (top == other.top && left == other.left && width == other.width
            && height == other.height)
        {
            return true;
        }
        return false;
    }

    bool operator!=(const struct roi_area& other) const
    {
        return !operator==(other);
    }

} roi_area;


/**
 * The roi_cache contains the percentile value of the image content.
 * e.g. left: 0.0 top: 0.0 width: 50.0 height: 50.0 covers 25% of the
 * image content. => image size 640x460 -> roi 320x240
 *
 */
typedef struct roi_cache
{

    float left_cache;
    float top_cache;
    float width_cache;
    float height_cache;

    bool empty() const
    {
        if (left_cache == 0 && top_cache == 0 && width_cache == 0 && height_cache == 0)
        {
            return true;
        }
        return false;
    }

    void clear()
    {
        left_cache = 0;
        top_cache = 0;
        width_cache = 0;
        height_cache = 0;
    }

} roi_object;
