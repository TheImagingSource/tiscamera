

#pragma once

#include <cstdint>
#include <dutils_img/dutils_cpu_features.h>

#ifndef CLIP
#   define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h): (val) )  
#endif // !CLIP

//  [[maybe_unused]] C++17
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) ((void)(x))
#endif

#if !defined FORCEINLINE
#  if defined(_MSC_VER)
#    define FORCEINLINE     __forceinline
#  elif defined( __GNUC__ )
#    if defined _DEBUG
#      define FORCEINLINE inline 
#    else
#      define FORCEINLINE inline __attribute__((always_inline))
#    endif

//#pragma GCC diagnostic ignored "-Wattributes"

#  endif
#endif

#if !defined UNREACHABLE_CODE
#  if defined(_MSC_VER)
#    define UNREACHABLE_CODE()     __assume(0)
#  elif defined( __GNUC__ )
#    define UNREACHABLE_CODE()     __builtin_unreachable()
#  endif
#endif