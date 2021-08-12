

#include "cpu_features.h"

#include "interop_private.h"

#if defined DUTILS_ARCH_ARM

unsigned int img_lib::cpu::get_features() noexcept
{
#if defined DUTILS_ARCH_ARM_A64
    return img::cpu::CPU_UsesARM_A64;
#elif defined  DUTILS_ARCH_ARM_A8
    return img::cpu::CPU_UsesARM_A8;
#elif defined DUTILS_ARCH_ARM
    return img::cpu::CPU_UsesARM_A7;
#else
#pragma message "Failed to identify ARM arch >= 7"
    return img::cpu::CPU_C;
#endif
}


#else

#ifdef __linux__

#include <cpuid.h>

static void    get_cpuid( int a[4], int b ) noexcept
{
    __get_cpuid( (unsigned int) b, (unsigned int*)&a[0], (unsigned int*)&a[1], (unsigned int*)&a[2], (unsigned int*)&a[3] );
}

#elif _WIN32

#include <intrin.h>

static void    get_cpuid( int a[4], int b ) noexcept
{
    __cpuid( a, b );
}

#endif

#define BIT(idx) (0x1 << (idx))


static bool is_AVX_supported() noexcept
{
    bool supported = false;

    // If Visual Studio 2010 SP1 or later
#if defined _MSC_FULL_VER && (_MSC_FULL_VER >= 160040219)
#define _XCR_XFEATURE_ENABLED_MASK 0 

    // Checking for AVX requires 3 things:
    // 1) CPUID indicates that the OS uses XSAVE and XRSTORE
    //     instructions (allowing saving YMM registers on context
    //     switch)
    // 2) CPUID indicates support for AVX
    // 3) XGETBV indicates the AVX registers will be saved and
    //     restored on context switch
    //
    // Note that XGETBV is only available on 686 or later CPUs, so
    // the instruction needs to be conditionally run.
    int cpuInfo[4];
    get_cpuid( cpuInfo, 1 );

    const bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
    const bool cpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

    if( osUsesXSAVE_XRSTORE && cpuAVXSuport )
    {
        // Check if the OS will save the YMM registers
        const unsigned long long xcrFeatureMask = _xgetbv( _XCR_XFEATURE_ENABLED_MASK );
        supported = (xcrFeatureMask & 0x6) || false;
    }
#elif defined __GNUC__
    supported = __builtin_cpu_supports( "avx" );
#endif
    return supported;
}

static bool is_AVX2_supported()	noexcept // do not call this without checking for OS support for avx1
{
    bool supported = false;
    // If Visual Studio 2010 SP1 or later
#if defined _MSC_FULL_VER &&  (_MSC_FULL_VER >= 160040219)
    int cpuInfo[4];
    get_cpuid( cpuInfo, 7 );	// request cpuid leaf 7

    // avx2 is bit 5
    supported = (cpuInfo[1] & (1 << 5)) != 0;
#elif defined __GNUC__
    supported = __builtin_cpu_supports( "avx2" );
#endif
    return supported;
}

static bool is_FMA_supported() noexcept	// do not call this without checking for OS support for avx1
{
    bool supported = false;

    // If Visual Studio 2010 SP1 or later
#if defined _MSC_FULL_VER && (_MSC_FULL_VER >= 160040219)
    int cpuInfo[4];
    get_cpuid( cpuInfo, 1 );	// request cpuid leaf 1

    supported = (cpuInfo[2] & (1 << 12)) != 0;
#elif defined __GNUC__
    supported = __builtin_cpu_supports( "fma" ) != 0;
#endif
    return supported;
}

static bool is_AVX512F_supported() noexcept
{
    bool supported = false;
#if defined _MSC_VER
#elif defined __GNUC__
    supported = __builtin_cpu_supports( "avx512f" );
#endif
    return supported;
}


static unsigned int     actual_get_features() noexcept
{
    using namespace img::cpu;

    unsigned int features = CPU_C;

    int CPUInfo[4] = {};
    const int InfoType = 1;
    get_cpuid( CPUInfo, InfoType );
    //features |= (CPUInfo[3] & BIT( 23 )) ? CPU_MMX : 0;
    //features |= (CPUInfo[3] & BIT( 25 )) ? CPU_SSE1 : 0;
    //features |= (CPUInfo[3] & BIT( 26 )) ? CPU_SSE2 : 0;
    //features |= (CPUInfo[2] & BIT( 0 )) ? CPU_SSE3 : 0;
    features |= (CPUInfo[2] & BIT( 9 )) ? CPU_SSSE3 : 0;
    features |= (CPUInfo[2] & BIT( 19 )) ? CPU_SSE41 : 0;
    features |= (CPUInfo[2] & BIT( 20 )) ? CPU_SSE42 : 0;
    if( is_AVX_supported() )	// this includes a check for OS support (since Win7 SP1)
    {
        features |= CPU_AVX1;
        features |= is_AVX2_supported() ? (unsigned)CPU_AVX2 : 0;
        features |= is_FMA_supported() ? (unsigned)CPU_FMA3 : 0;
        features |= is_AVX512F_supported() ? (unsigned)CPU_AVX512_F : 0;
    }
    return features;
}

unsigned int img_lib::cpu::get_features() noexcept
{
    static unsigned int cpu_features = actual_get_features();
    return cpu_features;
}

#endif

const char*     img_lib::cpu::to_string( img::cpu::cpu_features feat ) noexcept
{
    using namespace img::cpu;

#if !defined DUTILS_ARCH_ARM
    if( feat & CPU_AVX2 ) {
        return "AVX2";
    } else if( feat & CPU_AVX1 ) {
        return "AVX";
    } else if( feat & CPU_SSE41 ) {
        return "SSE 4.1";
    } else if( feat & CPU_SSSE3 ) {
        return "SSSE3";
    } else if( feat & CPU_C ) {
        return "C";
    }
#else
    if( feat & CPU_ARM_A64 ) {
        return "ARMv8 NEON A64";
    } else if( feat & CPU_ARM_A8 ) {
        return "ARMv8 NEON A32";
    } else if( feat & CPU_ARM_A7 ) {
        return "ARMv7 NEON";
    } else if( feat & CPU_C ) {
        return "ARM C";
    }
#endif

    return "Unspecified";
}