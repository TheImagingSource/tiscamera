#pragma once

#include "PropertyInterfaces.h"

#include <string_view>
#include <tcamprop1.0_base/tcamprop_base.h>
#include <vector>

namespace tcam::property::emulated
{
enum class software_prop
{
    ExposureTime,
    ExposureAuto,
    ExposureAutoLowerLimit,
    ExposureAutoUpperLimit,
    ExposureAutoReference,
    ExposureAutoUpperLimitAuto,
    ExposureAutoHighlightReduction,

    Gain,
    GainAuto,
    GainAutoLowerLimit,
    GainAutoUpperLimit,

    Iris,
    IrisAuto,
    Focus,
    FocusAuto,

    BalanceWhiteAuto,
    BalanceWhiteRed,
    BalanceWhiteGreen,
    BalanceWhiteBlue,
    ClaimBalanceWhiteSoftware,
};

struct prop_range_integer_def
{
    tcamprop1::prop_range_integer range;
    int64_t def = 0;
};

struct prop_range_float_def
{
    tcamprop1::prop_range_float range;
    double def = 0;
};

inline auto to_range(IPropertyFloat& prop)
{
    return prop_range_float_def { prop.get_range(), prop.get_default() };
}
inline auto to_range(IPropertyInteger& prop)
{
    return prop_range_integer_def { prop.get_range(), prop.get_default() };
}

template<size_t N> inline auto to_range(const std::array<std::string_view, N>& arr)
{
    return std::vector<std::string_view> { arr.begin(), arr.end() };
}

class SoftwarePropertyBackend
{
public:
    virtual ~SoftwarePropertyBackend() = default;

    virtual outcome::result<int64_t> get_int(software_prop id) = 0;
    virtual outcome::result<void> set_int(software_prop id, int64_t i) = 0;

    virtual outcome::result<double> get_double(software_prop id) = 0;
    virtual outcome::result<void> set_double(software_prop id, double new_value) = 0;
    virtual tcam::property::PropertyFlags get_flags(software_prop id) const = 0;
};

} // namespace tcam::property::emulated
