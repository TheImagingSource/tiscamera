/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include <vector>
#include <string>

namespace FirmwareUpdate
{

namespace FirmwarePackage
{

bool isPackageFile (const std::string& fileName);

std::vector<uint8_t> extractFile (const std::string& packageFileName,
                                  const std::string& fileName);

std::string extractTextFile (const std::string& packageFileName,
                             const std::string& fileName);

} /* namespace FirmwarePackage */

} /* namespace FirmwareUpdate */
