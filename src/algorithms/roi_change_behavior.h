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

#include <cstring>
#include <iostream>

// namespace roi
// {

enum ROI_CHANGE_BEHAVIOR
{
    ROI_CHANGE_BEHAVIOR_UNDEFINED = 0, ///< error, never
    ROI_CHANGE_BEHAVIOR_RESET = 1, ///< Reset the ROI to cover the entire image
};


const char* roi_change_behavior_to_string(ROI_CHANGE_BEHAVIOR behavior);


ROI_CHANGE_BEHAVIOR roi_change_behavior_from_string(const char* str);


// std::ostream& operator<< (std::ostream& out, ROI_CHANGE_BEHAVIOR behavior)
// {
//     out << roi_change_behavior_to_string(behavior);
//     return out;
// }

//} /* namespace roi */
