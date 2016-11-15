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


#ifndef _FILE_HANDLING_H_
#define _FILE_HANDLING_H_

#include <string>
#include <vector>

std::vector<uint8_t> open_firmware_file (std::string& filename);

std::vector<unsigned char> load_file (const std::string& filename);

bool is_package_file (const std::string& fileName);

std::vector<unsigned char> extract_file_from_package (const std::string& packageFileName,
                                                      const std::string& fileName);

bool is_valid_firmware_file (const std::string& firmware);

#endif /* _FILE_HANDLING_H_ */
