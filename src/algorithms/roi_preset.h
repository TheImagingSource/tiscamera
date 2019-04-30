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

#include <iostream>
#include <cstring>

// namespace roi
// {

/**
 * @brief preconfigured region of interest configuration
 */
enum ROI_PRESET
{
    ROI_PRESET_FULL_SENSOR = 0, ///< entire image region
    ROI_PRESET_CUSTOM_RECTANGLE = 1, ///< user defined region
    ROI_PRESET_CENTER_50 = 2, ///< center 50% of the image
    ROI_PRESET_CENTER_25 = 3, ///< center 25% of the image
    ROI_PRESET_BOTTOM_HALF = 4,
    ROI_PRESET_TOP_HALF = 5,
};


const char* roi_preset_to_string (ROI_PRESET preset);


ROI_PRESET roi_preset_from_string (const char* str);


// std::ostream& operator<< (std::ostream& out, ROI_PRESET preset)
// {
//     out << roi_preset_to_string(preset);
//     return out;
// }


// } /* namespace roi */
