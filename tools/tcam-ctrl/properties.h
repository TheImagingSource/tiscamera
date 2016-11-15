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

#ifndef CONTROLS_H
#define CONTROLS_H

#include <tcam.h>

#include <vector>
#include <string>
#include <memory>

using namespace tcam;

/**
 * @brief print function for properties
 * @param properties - Property collection that shall be printed
 */
void print_properties (const std::vector<Property*>& properties);


/**
 * @brief Set property described in string to new value
 * @param g - CaptureDevice of the device that shall be used
 * @param new_prop - string describing the format
 * @return true on success
 */
bool set_property (std::shared_ptr<CaptureDevice> dev, const std::string& new_prop);

#endif /* CONTROLS_H */
