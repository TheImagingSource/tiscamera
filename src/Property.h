




#ifndef PROPERTY_H_
#define PROPERTY_H_


#include "base_types.h"
#include "PropertyImpl.h"

#include <memory>
#include <string>
#include <vector>
#include <map>


namespace tis_imaging
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

    Property (const camera_property&, VALUE_TYPE);

    Property (const camera_property&,
              const std::map<std::string, int>&,
              VALUE_TYPE);

    virtual ~Property ();

    void reset ();

    std::string getName () const;

    PROPERTY_TYPE getType () const;

    bool isReadOnly () const;
    bool isWriteOnly () const;

    bool isDisabled () const;

    uint32_t getFlags () const;

    struct camera_property getStruct () const;
    bool setStruct (const struct camera_property&);

    Property::VALUE_TYPE getValueType () const;

protected:

    bool setReadOnly (bool);

    bool setWriteOnly (bool);

    bool setInactive (bool);

    // PropertyImple interface
    bool isAvailable (const Property&);
    bool setProperty (const Property&);
    bool getProperty (Property&);

    std::weak_ptr<PropertyImpl> impl;

    Property::VALUE_TYPE value_type;
    // struct control_mapping mapping;
    struct camera_property prop;
    const struct camera_property ref_prop;
    std::map<std::string, int> string_map;

    // internal method that notifies implementations about requests
    void notifyImpl ();
};

} /* namespace tis_imaging */

#endif /* PROPERTY_H_ */
