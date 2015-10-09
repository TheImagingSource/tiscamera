#ifndef __PROPERTY_H__
#define __PROPERTY_H__

class CProperty
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
    } _type;

    
    char _name[255];
    int _id;
    int _minimum;
    int _maximum;
    int _default;
    int _value;
   
    CProperty( const char* name, int id, int minimum, int maximum, int def, int value, VALUE_TYPE type = INTEGER);
    
  
};

#endif