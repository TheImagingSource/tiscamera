#include "property.h"
#include <string.h>

CProperty::CProperty( const char* name, int id, int minimum, int maximum, int def, int value, VALUE_TYPE type )
{
  if(name == NULL )
    strcpy( _name, "Unknown");
  else
    strcpy( _name, name);
  
  _id = id;
  _minimum = minimum;
  _maximum = maximum;
  _default = def;
  _value = value;
  
  _type = type;
}
