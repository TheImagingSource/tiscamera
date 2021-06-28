
#pragma once

#include "see_intrin_base.h"


#if !defined DUTILS_SIMD_USAGE_LEVEL || (DUTILS_SIMD_USAGE_LEVEL < DUTILS_SIMD_USAGE_LEVEL_SSE41)
#error "This file needs SSE4_1 intrinsics. The current TU is marked as <= SSSE3."
#endif

#include <smmintrin.h>	// SSE4.1
#include <tmmintrin.h>	// SSSE3
#include <emmintrin.h>  // SSE2

#include "sse_init_reg.h"

#include "impl/sse_utils_sse41.h"

