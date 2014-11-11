




#ifndef PROPERTY_H_
#define PROPERTY_H_


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

    Property (const tcam_camera_property&, VALUE_TYPE);

    Property (const tcam_camera_property&,
              const std::map<std::string, int>&,
              VALUE_TYPE);

    virtual ~Property ();

    void reset ();

    /**
     * @brief update the property so that it will hold up to date information
     * @return true on success
     */
    bool update ();

    TCAM_PROPERTY_ID getID () const;

    std::string getName () const;

    TCAM_PROPERTY_TYPE getType () const;

    bool isReadOnly () const;
    bool isWriteOnly () const;

    bool isDisabled () const;

    uint32_t getFlags () const;

    struct tcam_camera_property getStruct () const;
    bool setStruct (const struct tcam_camera_property&);

    Property::VALUE_TYPE getValueType () const;


    std::string toString () const;

    bool fromString (const std::string&);


    bool setProperty (const Property&);
    bool getProperty (Property&);

protected:

    bool setReadOnly (bool);

    bool setWriteOnly (bool);

    bool setInactive (bool);

    std::weak_ptr<PropertyImpl> impl;

    Property::VALUE_TYPE value_type;

    // internal storage
    struct tcam_camera_property prop;

    // reference of initial state
    const struct tcam_camera_property ref_prop;
    std::map<std::string, int> string_map;

    // internal method that notifies implementations about requests
    void notifyImpl ();
};

TCAM_PROPERTY_TYPE value_type_to_ctrl_type (const Property::VALUE_TYPE& t);

} /* namespace tcam */

#endif /* PROPERTY_H_ */
