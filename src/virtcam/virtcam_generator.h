/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "dutils_img/image_fourcc_enum.h"
#include "dutils_img/image_transform_base.h"
#include <memory>
#include <vector>

#include "generator/generator_base.h"

namespace tcam::virtcam
{

// returns a vector of all supported fourcc that can be generated
std::vector<img::fourcc> get_supported_fourcc();

// create an image generator for the given fourcc
// returns nullptr if fourcc is not supported
std::unique_ptr<tcam::generator::IGenerator> get_generator(img::fourcc);

} // namespace tcam::virtcam
