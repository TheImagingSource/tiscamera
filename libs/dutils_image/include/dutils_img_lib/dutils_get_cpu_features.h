
#pragma once

#include <dutils_img/dutils_cpu_features.h>

namespace img_lib {
namespace cpu {
    unsigned int	get_features() noexcept;
    const char*     to_string( img::cpu::cpu_features feature ) noexcept;
}
}
