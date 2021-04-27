
#include "SoftwarePropertyBackend.h"

#include "SoftwareProperties.h"

namespace tcam::property::emulated
{

SoftwarePropertyBackend::SoftwarePropertyBackend(tcam::property::SoftwareProperties* impl)
    : p_impl(impl)
{
}

int SoftwarePropertyBackend::get_int(software_prop id)
{
    return p_impl->get_int(id);
}


bool SoftwarePropertyBackend::set_int(software_prop id, int i)
{
    return p_impl->set_int(id, i);
}

double SoftwarePropertyBackend::get_double(software_prop id)
{
    return p_impl->get_double(id);
}


bool SoftwarePropertyBackend::set_double(software_prop id, double new_value)
{
    return p_impl->set_double(id, new_value);
}

} // namespace tcam::property::emulated
