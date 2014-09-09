



#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include "Property.h"

#include <linux/videodev2.h>

#include <string>
#include <vector>
#include <memory>

namespace tis_imaging
{


/*

 */
class PropertyString : public Property
{
public:

    PropertyString (std::shared_ptr<PropertyImpl>,
                    const camera_property&,
                    const VALUE_TYPE&);
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
                       const VALUE_TYPE&);

    ~PropertyStringMap ();

    std::vector<std::string> getValues () const;

    std::string getDefault () const;

    bool setValue (const std::string&);
    std::string getValue () const;

    std::map<std::string, int> getMapping () const;

};



/*

 */
class PropertySwitch : public Property
{
public:

    PropertySwitch (std::shared_ptr<PropertyImpl>, const camera_property&, const VALUE_TYPE&);

    ~PropertySwitch ();

    bool getDefault () const;

    bool setValue (const bool&);

    bool getValue () const;
};



/*

 */
class PropertyInteger : public Property
{
public:

    PropertyInteger (std::shared_ptr<PropertyImpl>, const camera_property&, const VALUE_TYPE&);

    ~PropertyInteger ();

    int64_t getDefault () const;

    int64_t getMin () const;
    int64_t getMax () const;

    int64_t getValue () const;

    bool setValue(const int64_t&);
};


/*

 */
class PropertyDouble : public Property
{
public:

    PropertyDouble (std::shared_ptr<PropertyImpl>, const camera_property&, const VALUE_TYPE&);
    ~PropertyDouble ();

    double getDefault () const;

    double getMin () const;
    double getMax () const;

    double getValue () const;

    bool setValue(const double&);
};


/*

 */
class PropertyButton : public Property
{
public:

    PropertyButton (std::shared_ptr<PropertyImpl>, const camera_property&, const VALUE_TYPE&);
    ~PropertyButton ();

    bool activate ();
};


} /* namespace tis_imaging */

#endif /* PROPERTIES_H_ */
