#ifndef PROPERTYWIDGETBASE_H
#define PROPERTYWIDGETBASE_H

#include "tcam.h"

class PropertyWidgetBase
{
public:
    PropertyWidgetBase ();

    virtual PropertyWidgetBase ();

protected:

    tcam::Property property;
};

#endif // PROPERTYWIDGETBASE_H
