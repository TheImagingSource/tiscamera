
#pragma once

#include "see_intrin_base.h"

#ifdef DUTILS_SIMD_USAGE_LEVEL
#error SIMD usage level already defined to DUTILS_SIMD_USAGE_LEVEL
#endif // DUTILS_SIMD_USAGE_LEVEL

#define DUTILS_SIMD_USAGE_LEVEL     DUTILS_SIMD_USAGE_LEVEL_SSSE3

#include "include_ssse3.h"

