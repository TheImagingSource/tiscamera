
#pragma once

#include "see_intrin_base.h"

#if !defined DUTILS_ARCH_ARM || (DUTILS_SIMD_USAGE_LEVEL < DUTILS_SIMD_USAGE_LEVEL_ARM_A8)
#error "This file needs ARM A8 NEON intrinsics. The current TU is not build with ARM arch activated (-mcpu=cortex-a7 ...)."
#endif

#include <arm_neon.h>
