
#pragma once

#include "see_intrin_base.h"

#if !defined DUTILS_ARCH_ARM || (DUTILS_SIMD_USAGE_LEVEL < DUTILS_SIMD_USAGE_LEVEL_ARM_A64)
#error "This file needs AARCH64 intrinsics. The current TU is not build with ARM AARCH64 intrinsics activated."
#endif

#include <arm_neon.h>
#include "include_A8.h"
#include "impl/sse_utils_A64.h"