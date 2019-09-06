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

#include "image_fourcc.h"

namespace img
{

//constexpr
bool is_by16_fcc (uint32_t fcc) noexcept
{
    switch( fcc )
    {
        case FOURCC_BGGR16:
        case FOURCC_GBRG16:
        case FOURCC_RGGB16:
        case FOURCC_GRBG16:
            return true;
        default:
            return false;
    };
}

//constexpr
bool is_by8_fcc (uint32_t fcc) noexcept
{
    switch( fcc )
    {
        case FOURCC_BY8:		// deprecated
        case FOURCC_BGGR8:
        case FOURCC_GBRG8:
        case FOURCC_RGGB8:
        case FOURCC_GRBG8:
            return true;
        default:
            return false;
    };
}

} /* namespace img */
