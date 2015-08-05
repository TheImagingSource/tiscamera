



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
class PropertyStringMap : public Property
{
public:

    PropertyStringMap (std::shared_ptr<PropertyImpl>,
                       const tcam_device_property&,
                       const std::map<std::string, int>&,
                       VALUE_TYPE);

    ~PropertyStringMap ();

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

    double get_value () const;

    bool set_value (double);
};


/*

 */
class PropertyButton : public Property
{
public:

    PropertyButton (std::shared_ptr<PropertyImpl>, const tcam_device_property&, VALUE_TYPE);
    ~PropertyButton ();

    bool activate ();
};


} /* namespace tcam */

/** @} */

#endif /* TCAM_PROPERTIES_H */
