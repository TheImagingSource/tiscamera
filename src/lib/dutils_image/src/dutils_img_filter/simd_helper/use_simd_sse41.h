
#ifndef USE_SIMD_SSE4_1_H_INC__
#define USE_SIMD_SSE4_1_H_INC__

#pragma once

#include "see_intrin_base.h"

#ifdef DUTILS_SIMD_USAGE_LEVEL
#error "SIMD usage level already defined"
#endif // DUTILS_SIMD_USAGE_LEVEL

#define DUTILS_SIMD_USAGE_LEVEL     DUTILS_SIMD_USAGE_LEVEL_SSE41

#include "include_sse41.h"


#endif // USE_SIMD_SSE4_1_H_INC__
