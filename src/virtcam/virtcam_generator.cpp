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

#include "virtcam_generator.h"
#include "dutils_img/fcc_to_string.h"
#include "dutils_img/image_fourcc_enum.h"
#include <algorithm>
#include <memory>
#include "dutils_img/image_fourcc_func.h"
#include "spdlog/spdlog.h"

#include "generator/mono_generator.h"
#include "generator/rgb_generator.h"
#include "generator/bayer_generator.h"


static const img::fourcc supported_fourcc[] {
    img::fourcc::MONO8,
    img::fourcc::MONO12_MIPI_PACKED,
    img::fourcc::MONO16,
    img::fourcc::RGGB8,
    img::fourcc::RGGB12_MIPI_PACKED,
    img::fourcc::RGGB16,
    img::fourcc::BGR24,
};


std::vector<img::fourcc> tcam::virtcam::get_supported_fourcc()
{
    return std::vector<img::fourcc> (std::begin(supported_fourcc), std::end(supported_fourcc));
}


std::unique_ptr<tcam::generator::IGenerator> tcam::virtcam::get_generator(img::fourcc fcc)
{
    switch (fcc)
    {
        case img::fourcc::MONO8:
        case img::fourcc::MONO12_MIPI_PACKED:
        case img::fourcc::MONO16:
        {
            return std::make_unique<tcam::generator::MonoGenerator>(fcc);
        }
        case img::fourcc::RGGB8:
        case img::fourcc::RGGB16:
        case img::fourcc::RGGB12_MIPI_PACKED:
        {
            return std::make_unique<tcam::generator::BayerGenerator>(fcc);
        }
        case img::fourcc::BGR24:
        {
            return std::make_unique<tcam::generator::RGBGenerator>(fcc);
        }
        default:
        {
            SPDLOG_ERROR("Generator not implemented for fourcc: {}", img::fcc_to_string(fcc));
            return nullptr;
        }
    }

    return nullptr;
}
