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
#include "dutils_img/image_fourcc_func.h"
#include "dutils_img/image_transform_base.h"
#include "generator_base.h"
#include "pattern_generator.h"

#include <cmath>
#include <cstdint>
#include <dutils_img/image_bayer_pattern.h>
#include <dutils_img/pixel_structs.h>


namespace tcam::generator
{

class MonoGenerator : public IGenerator
{

private:

    pattern::WhiteNoise pattern_gen_;
    img::fourcc fourcc_;

public:
    explicit MonoGenerator(img::fourcc fcc);

    // switch to next image
    // this function iterates through all colors
    // by increasing a channel to max before increasing the next one
    // after that the first one will be decreased before beginning anew
    void step() final {}

    void fill_image(img::img_descriptor& dst) final;

}; // class MonoGenerator

} // namespace tcam::generator
