

#pragma once

#include "SoftwarePropertiesBase.h"

namespace tcam::property
{
class SoftwareProperties;
}

namespace tcam::property::emulated
{

class SoftwarePropertyBackend
{
public:
    SoftwarePropertyBackend(SoftwareProperties*);

    int get_int(software_prop id);
    bool set_int(software_prop id, int i);

    double get_double(software_prop id);
    bool set_double(software_prop id, double new_value);

private:
    SoftwareProperties* p_impl;
};

} /* namespace tcam::property::emulated */
