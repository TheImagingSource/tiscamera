



#ifndef PROPERTYIMPL_H_
#define PROPERTYIMPL_H_

namespace tis_imaging
{

class Property;

class PropertyImpl
{
    
public:
    
    virtual ~PropertyImpl () {};

    virtual bool isAvailable (const Property&) = 0;

    virtual bool setProperty (const Property&) = 0;
    
    virtual bool getProperty (Property&) = 0;
};

} /* namespace tis_imaging */

#endif /* PROPERTYIMPL_H_ */
