
#include "SoftwarePropertyBackend.h"

#include "SoftwareProperties.h"

namespace tcam::property::emulated
{

SoftwarePropertyBackend::SoftwarePropertyBackend(tcam::property::SoftwareProperties* impl)
    : p_impl(impl)
{
}

outcome::result<int64_t> SoftwarePropertyBackend::get_int(software_prop id)
{
    return p_impl->get_int(id);
}


outcome::result<void> SoftwarePropertyBackend::set_int(software_prop id, int64_t i)
{
    return p_impl->set_int(id, i);
}

outcome::result<double> SoftwarePropertyBackend::get_double(software_prop id)
{
    return p_impl->get_double(id);
}


outcome::result<void> SoftwarePropertyBackend::set_double(software_prop id, double new_value)
{
    return p_impl->set_double(id, new_value);
}

tcam::property::PropertyFlags SoftwarePropertyBackend::get_flags(software_prop id) const
{
    return p_impl->get_flags(id);
}

} // namespace tcam::property::emulated
