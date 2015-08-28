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

#ifndef CTRL_MULTIMEDIA_H
#define CTRL_MULTIMEDIA_H

#include <string>
#include <tcam.h>

namespace tcam
{

bool save_image (CaptureDevice& g, const std::string& filename);

bool save_stream (CaptureDevice& g, const std::string& file);

} /* namespace tcam */

#endif /* CTRL_MULTIMEDIA_H */
