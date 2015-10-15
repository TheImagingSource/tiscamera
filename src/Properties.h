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

#ifndef TCAM_PROPERTIES_H
#define TCAM_PROPERTIES_H

#include "Property.h"

#include <string>
#include <vector>
#include <memory>


/**
 * @addtogroup API
 * @{
 */


namespace tcam
{


/*

 */
class PropertyString : public Property
{
public:

    PropertyString (std::shared_ptr<PropertyImpl>,
                    const tcam_device_property&,
                    VALUE_TYPE);
    ~PropertyString ();

    std::string get_default () const;

    bool set_value (const std::string&);
    std::string get_value () const;
};



/*

 */
class PropertyEnumeration : public Property
{
public:

    PropertyEnumeration (std::shared_ptr<PropertyImpl>,
                         const tcam_device_property&,
                         const std::map<std::string, int>&,
                         VALUE_TYPE);

    ~PropertyEnumeration ();

    std::vector<std::string> get_values () const;

    std::string get_default () const;

    bool set_value (const std::string&);
    std::string get_value () const;

    std::map<std::string, int> get_mapping () const;

};



/*

 */
class PropertyBoolean : public Property
{
public:

    PropertyBoolean (std::shared_ptr<PropertyImpl>, const tcam_device_property&, VALUE_TYPE);

    ~PropertyBoolean ();

    bool get_default () const;

    bool set_value (bool);

    bool get_value () const;
};



/*

 */
class PropertyInteger : public Property
{
public:

    PropertyInteger (std::shared_ptr<PropertyImpl>, const tcam_device_property&, VALUE_TYPE);

    ~PropertyInteger ();

    int64_t get_default () const;

    int64_t get_min () const;
    int64_t get_max () const;
    int64_t get_step () const;
    int64_t get_value () const;

    bool set_value (int64_t);

    static const TCAM_PROPERTY_TYPE type = TCAM_PROPERTY_TYPE_INTEGER;
};


/*

 */
class PropertyDouble : public Property
{
public:

    PropertyDouble (std::shared_ptr<PropertyImpl>, const tcam_device_property&, VALUE_TYPE);
    ~PropertyDouble ();

    double get_default () const;

    double get_min () const;
    double get_max () const;
    double get_step () const;
    double get_value () const;

    bool set_value (double);

    static const TCAM_PROPERTY_TYPE type = TCAM_PROPERTY_TYPE_DOUBLE;
};


/*

 */
class PropertyButton : public Property
{
public:

    PropertyButton (std::shared_ptr<PropertyImpl>, const tcam_device_property&, VALUE_TYPE);
    ~PropertyButton ();

    bool activate ();

private:
    static const TCAM_PROPERTY_TYPE type = TCAM_PROPERTY_TYPE_BUTTON;
};


} /* namespace tcam */

/** @} */

#endif /* TCAM_PROPERTIES_H */
