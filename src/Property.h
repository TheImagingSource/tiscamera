




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

    Property (const camera_property&);

    Property (const camera_property&, const std::map<int, std::string>&);

    virtual ~Property ();

    void reset ();

    std::string getName () const;

    PROPERTY_TYPE getType () const;

    bool isReadOnly () const;
    bool isWriteOnly () const;

    bool isDisabled () const;

    uint32_t getFlags () const;

    struct camera_property getStruct () const;

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

protected:

    bool setReadOnly (const bool);

    bool setWriteOnly (const bool);

    bool setInactive (const bool);

    // PropertyImple interface
    bool isAvailable (const Property&);
    bool setProperty (const Property&);
    bool getProperty (Property&);

    std::weak_ptr<PropertyImpl> impl;

    // struct control_mapping mapping;
    struct camera_property prop;
    std::map<std::string, int> string_map;

    // internal method that notifies implementations about requests
    void notifyImpl ();
};

} /* namespace tis_imaging */

#endif /* PROPERTY_H_ */
