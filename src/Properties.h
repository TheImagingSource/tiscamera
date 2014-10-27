



#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include "Property.h"

#include <linux/videodev2.h>

#include <string>
#include <vector>
#include <memory>

namespace tcam
{


/*

 */
class PropertyString : public Property
{
public:

    PropertyString (std::shared_ptr<PropertyImpl>,
                    const camera_property&,
                    VALUE_TYPE);
    ~PropertyString ();

    std::string getDefault () const;

    bool setValue (const std::string&);
    std::string getValue () const;
};



/*

 */
class PropertyStringMap : public Property
{
public:

    PropertyStringMap (std::shared_ptr<PropertyImpl>,
                       const camera_property&,
                       const std::map<std::string, int>&,
                       VALUE_TYPE);

    ~PropertyStringMap ();

    std::vector<std::string> getValues () const;

    std::string getDefault () const;

    bool setValue (const std::string&);
    std::string getValue () const;

    std::map<std::string, int> getMapping () const;

};



/*

 */
class PropertyBoolean : public Property
{
public:

    PropertyBoolean (std::shared_ptr<PropertyImpl>, const camera_property&, VALUE_TYPE);

    ~PropertyBoolean ();

    bool getDefault () const;

    bool setValue (bool);

    bool getValue () const;
};



/*

 */
class PropertyInteger : public Property
{
public:

    PropertyInteger (std::shared_ptr<PropertyImpl>, const camera_property&, VALUE_TYPE);

    ~PropertyInteger ();

    int64_t getDefault () const;

    int64_t getMin () const;
    int64_t getMax () const;
    int64_t getStep () const;
    int64_t getValue () const;

    bool setValue (int64_t);
};


/*

 */
class PropertyDouble : public Property
{
public:

    PropertyDouble (std::shared_ptr<PropertyImpl>, const camera_property&, VALUE_TYPE);
    ~PropertyDouble ();

    double getDefault () const;

    double getMin () const;
    double getMax () const;

    double getValue () const;

    bool setValue (double);
};


/*

 */
class PropertyButton : public Property
{
public:

    PropertyButton (std::shared_ptr<PropertyImpl>, const camera_property&, VALUE_TYPE);
    ~PropertyButton ();

    bool activate ();
};


} /* namespace tcam */

#endif /* PROPERTIES_H_ */
