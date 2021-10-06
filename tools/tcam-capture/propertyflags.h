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

#ifndef PROPERTYFLAGS_H
#define PROPERTYFLAGS_H

#include <gst/gst.h>

enum class PropertyFlagsEnum : int
{
    None = 0x0,
    Implemented = 0x1,
    Available = 0x2,
    Locked = 0x4,
    External = 0x8,
};

class PropertyFlags
{
public:
    explicit PropertyFlags(GValue* value);
    explicit PropertyFlags(int value);

    void update(GValue* value);

    bool is_implemented() const
    {
        return (((int)flags & (int)PropertyFlagsEnum::Implemented) != 0);
    };
    bool is_available() const
    {
        return ((int)flags & (int)PropertyFlagsEnum::Available) != 0;
    };
    bool is_locked() const
    {
        return ((int)flags & (int)PropertyFlagsEnum::Locked) != 0;
    };
    bool is_external() const
    {
        return ((int)flags & (int)PropertyFlagsEnum::External) != 0;
    };

private:
    PropertyFlagsEnum flags;
};

#endif // PROPERTYFLAGS_H
