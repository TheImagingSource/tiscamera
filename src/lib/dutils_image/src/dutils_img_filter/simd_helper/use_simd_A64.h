

#pragma once

#include "see_intrin_base.h"

#ifdef DUTILS_SIMD_USAGE_LEVEL
#error "SIMD usage level already defined"
#endif // DUTILS_SIMD_USAGE_LEVEL

#define DUTILS_SIMD_USAGE_LEVEL     DUTILS_SIMD_USAGE_LEVEL_ARM_A64

#include "include_A64.h"
