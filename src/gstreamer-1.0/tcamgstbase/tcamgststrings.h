/*
* Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_GST_STRINGS_H
#define TCAM_GST_STRINGS_H

#include <cstdint>
#include <string>


namespace tcam::gst
{

std::string tcam_fourcc_to_gst_1_0_caps_string(uint32_t fourcc);
uint32_t tcam_fourcc_from_gst_1_0_caps_string(const char* name, const char* format);

} // namespace tcam::gst

#endif /* TCAM_GST_STRINGS_H */
