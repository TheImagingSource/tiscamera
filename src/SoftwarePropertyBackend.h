

#pragma once

#include "SoftwarePropertiesBase.h"
#include "error.h"
#include "PropertyFlags.h"

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

    outcome::result<int64_t> get_int(software_prop id);
    outcome::result<void> set_int(software_prop id, int64_t i);

    outcome::result<double> get_double(software_prop id);
    outcome::result<void> set_double(software_prop id, double new_value);

    tcam::property::PropertyFlags get_flags(software_prop id) const;

private:
    SoftwareProperties* p_impl;
};

} /* namespace tcam::property::emulated */
