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

#ifndef TCAM_PROPERTY_H
#define TCAM_PROPERTY_H


#include "base_types.h"
#include "PropertyImpl.h"

#include <memory>
#include <string>
#include <vector>
#include <map>


namespace tcam
{

class Property : public PropertyImpl
{

public:
    enum VALUE_TYPE
    {
        UNDEFINED = 0,
        BOOLEAN,
        STRING,
        ENUM,
        INTEGER,
        INTSWISSKNIFE,
        FLOAT,
        COMMAND,
        BUTTON,
    };

    Property();

    Property (const tcam_device_property&, VALUE_TYPE);

    Property (const tcam_device_property&,
              const std::map<std::string, int>&,
              VALUE_TYPE);

    virtual ~Property ();

    virtual Property& operator= (const Property&);

    void reset ();

    /**
     * @brief update the property so that it will hold up to date information
     * @return true on success
     */
    bool update ();

    /**
     * @return TCAM_PROPERTY_ID of the property
     */
    TCAM_PROPERTY_ID get_ID () const;

    /**
     *
     */
    std::string get_name () const;

    /**
     *
     */
    TCAM_PROPERTY_TYPE get_type () const;


    /**
     * Convenience check for read-only and disabled
     */
    bool can_be_changed () const;

    /**
     * @return true if property can not be written
     */
    bool is_read_only () const;

    /**
     * @return true if property can not be read
     */
    bool is_write_only () const;

    /**
     *
     */
    bool is_disabled () const;


    bool is_external () const;

    /**
     *
     */
    uint32_t get_flags () const;


    struct tcam_device_property get_struct () const;

    bool set_value (const struct tcam_device_property&);

    bool set_value (const int64_t& value);
    bool set_value (const double& value);
    bool set_value (const bool& value);
    bool set_value (const std::string& value);
    bool set_value ();

    bool set_flags (); // ??????




    bool set_struct (const struct tcam_device_property&);

    /**
     * Set value from given tcam_device_property
     * @return true on success
     */
    void set_struct_value (const struct tcam_device_property&);
    void get_struct_value (struct tcam_device_property& p);
    Property::VALUE_TYPE get_value_type () const;


    std::string to_string () const;

    bool from_string (const std::string&);


    bool set_property (const Property&);
    bool set_property_from_struct (const tcam_device_property&);
    bool get_property (Property&);
    bool get_property_from_struct (tcam_device_property&);

protected:

    std::weak_ptr<PropertyImpl> impl;

    Property::VALUE_TYPE value_type;

    // internal storage
    struct tcam_device_property prop;

    // reference of initial state
    const struct tcam_device_property ref_prop;
    std::map<std::string, int> string_map;

    // internal method that notifies implementations about requests
    void notify_impl ();
};

TCAM_PROPERTY_TYPE value_type_to_ctrl_type (const Property::VALUE_TYPE& t);

} /* namespace tcam */

#endif /* TCAM_PROPERTY_H */
