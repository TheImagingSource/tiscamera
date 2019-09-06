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

namespace img
{
struct point
{
    int x, y;
};

struct rect
{
    int left;
    int top;
    int right;
    int bottom;
};

struct dim
{
    int cx, cy;
};

constexpr bool operator==( const dim& lhs, const dim& rhs ) noexcept { return (lhs.cx == rhs.cx) && (lhs.cy == rhs.cy); }
constexpr bool operator!=( const dim& lhs, const dim& rhs ) noexcept { return !(lhs == rhs); }

}
