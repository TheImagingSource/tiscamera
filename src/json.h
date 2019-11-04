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

#ifndef TCAM_JSON_H
#define TCAM_JSON_H

#include <tcam.h>

#include <string>


namespace tcam
{


static const char* JSON_FILE_VERSION_CURRENT  __attribute__ ((unused)) = "v0.1";


// bool version_matches (const char* version=JSON_FILE_VERSION_CURRENT);

// serial_matches ();

std::string create_json_state (std::shared_ptr<CaptureDevice> dev);

void load_json_state (std::shared_ptr<CaptureDevice> dev, const std::string& state);

}


#endif /* TCAM_JSON_H */
