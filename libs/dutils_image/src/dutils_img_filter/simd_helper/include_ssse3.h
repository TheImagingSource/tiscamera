
#pragma once

#if !defined DUTILS_SIMD_USAGE_LEVEL || (DUTILS_SIMD_USAGE_LEVEL < DUTILS_SIMD_USAGE_LEVEL_SSSE3)
#error "This file needs SSSE3 intrinsics. The current TU is marked as <= SSE2."
#endif

#pragma once

#include <tmmintrin.h>	// SSSE3
#include <emmintrin.h>  // SSE2

#include "sse_init_reg.h"
#include "impl/sse_utils_ssse3.h"
