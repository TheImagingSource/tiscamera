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
#include <string_view>

namespace tcam
{

enum class Visibility
{
    Beginner,
    Expert,
    Guru,
    Invisible
};


inline const std::string_view visibility_to_string( Visibility type )
{
    switch (type)
    {
        case Visibility::Beginner:
            return "Beginner";
        case Visibility::Expert:
            return "Expert";
        case Visibility::Guru:
            return "Guru";
        case Visibility::Invisible:
            return "Invisible";
    }
}

} /* namespace tcam */
