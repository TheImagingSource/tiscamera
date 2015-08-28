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

#ifndef TCAM_PROPERTYIMPL_H
#define TCAM_PROPERTYIMPL_H

namespace tcam
{

class Property;

/**
 * Interface used by property implementations
 */
class PropertyImpl
{

public:

    virtual ~PropertyImpl () {};

    /**
     * Set internal properties to the values described in parameter
     * @return true is given property values where successfully applied; else false
     */
    virtual bool set_property (const Property&) = 0;

    /**
     * Fill given property with the currently used values
     * @return true if property was successfully filled; else false
     */
    virtual bool get_property (Property&) = 0;
};

} /* namespace tcam */

#endif /* TCAM_PROPERTYIMPL_H */
