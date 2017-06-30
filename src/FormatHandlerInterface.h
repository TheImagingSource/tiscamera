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

#ifndef TCAM_FORMATHANDLERINTERFACE_H
#define TCAM_FORMATHANDLERINTERFACE_H

#include "compiler_defines.h"
#include "base_types.h"

#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{

class FormatHandlerInterface
{
public:

    virtual ~FormatHandlerInterface () {};

    virtual std::vector<double> get_framerates (const struct tcam_image_size&, int pixelformat=0) = 0;
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_FORMATHANDLERINTERFACE_H */
