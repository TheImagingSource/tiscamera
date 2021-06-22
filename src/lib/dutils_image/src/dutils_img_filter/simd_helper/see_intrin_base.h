
#ifndef SEE_INTRIN_BASE_H_INC__
#define SEE_INTRIN_BASE_H_INC__

#pragma once

#include <dutils_img/dutils_cpu_features.h>
#include "../src/dutils_img_base/interop_private.h"

#if !defined DUTILS_ARCH_ARM

#define DUTILS_SIMD_USAGE_LEVEL_SSSE3   3
#define DUTILS_SIMD_USAGE_LEVEL_SSE41  4

#define DUTILS_SIMD_USAGE_LEVEL_AVX1    5
#define DUTILS_SIMD_USAGE_LEVEL_AVX2    6

#else

#define DUTILS_SIMD_USAGE_LEVEL_ARM_A8      1
#define DUTILS_SIMD_USAGE_LEVEL_ARM_A64     2

#endif

#endif // SEE_INTRIN_BASE_H_INC__
