/*
 * Copyright 2015 The Imaging Source Europe GmbH
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

#ifndef TCAM_FORMAT_H
#define TCAM_FORMAT_H

#include <string>

#include <cstdint>

namespace tcam
{


/**
 * Description for fourcc2description.
 * @param fourcc - format type that shall be descriped
 * @return description of the fourcc; NULL if none
 */
const char* fourcc2description (uint32_t fourcc);


/**
 * @brief convert string to fourcc
 * @param description - string that shall be converted
 * @return fourcc of the description; 0 if none
 */
uint32_t description2fourcc (const char* description);


/**
 * @brief
 */
std::string fourcc2string (uint32_t fourcc);


/**
 *
 */
uint32_t string2fourcc (const std::string& s);

} /* namespace tcam */

#endif /* TCAM_FORMAT_H */
