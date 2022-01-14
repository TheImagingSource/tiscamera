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

#include "logging.h"

#include <memory>

VISIBILITY_DEFAULT

namespace libtcam
{
void setup_default_logger(bool add_stdout_logger = false);

auto get_spdlog_logger() -> std::shared_ptr<spdlog::logger>;

void print_version_info_once();

} // namespace libtcam

VISIBILITY_POP
