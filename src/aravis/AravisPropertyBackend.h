/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "error.h"

struct _ArvDevice;

namespace tcam::property
{

class AravisPropertyBackend
{

    public:

        AravisPropertyBackend(_ArvDevice*);

        outcome::result<int64_t> get_int(const std::string& name);
        outcome::result<void> set_int(const std::string& name, int64_t new_value);
    
        outcome::result<double> get_double(const std::string& name);
        outcome::result<void> set_double(const std::string& name, double new_value);
    
        outcome::result<bool> get_bool(const std::string& name);
        outcome::result<void> set_bool(const std::string& name, bool new_value);
    
        outcome::result<void> execute(const std::string& name);
    
        outcome::result<std::string> get_enum(const std::string& name);
        outcome::result<void> set_enum(const std::string& name, const std::string& value);
    

    private:    
    _ArvDevice* p_device;

};

}