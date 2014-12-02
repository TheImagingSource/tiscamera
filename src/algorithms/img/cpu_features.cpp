

//#include "stdafx.h"

#include "cpu_features.h"

#ifdef __linux__

#include <cpuid.h>

static void    get_cpuid( int a[4], int b )
{
    __get_cpuid( b, (unsigned int*)&a[0], (unsigned int*)&a[1], (unsigned int*)&a[2], (unsigned int*)&a[3] );
}

#elif _WIN32

extern "C" void __cdecl __cpuid(int a[4], int b);
extern "C" unsigned __int64 __cdecl _xgetbv( unsigned int ext_ctrl_reg );

static void    get_cpuid( int a[4], int b )
{
    __cpuid( a, b );
}

#endif

#define BIT(idx) (0x1 << (idx))


#if 0 // original intel code for avx2 check
extern "C" void __cdecl __cpuidex( int a[4], int b );

typedef unsigned __int32 uint32_t;

#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1300)

#include <immintrin.h>

int check_4th_gen_intel_core_features()
{
    const int the_4th_gen_features =
        (_FEATURE_AVX2 | _FEATURE_FMA | _FEATURE_BMI | _FEATURE_LZCNT | _FEATURE_MOVBE);
    return _may_i_use_cpu_feature( the_4th_gen_features );
}

#else /* non-Intel compiler */

#include <stdint.h>
#if defined(_MSC_VER)
# include <intrin.h>
#endif

void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t* abcd)
{
#if defined(_MSC_VER)
    __cpuidex(abcd, eax, ecx);
#else
    uint32_t ebx, edx;
# if defined( __i386__ ) && defined ( __PIC__ )
     /* in case of PIC under 32-bit EBX cannot be clobbered */
    __asm__ ( "movl %%ebx, %%edi \n\t cpuid \n\t xchgl %%ebx, %%edi" : "=D" (ebx),
# else
    __asm__ ( "cpuid" : "+b" (ebx),
# endif
              "+a" (eax), "+c" (ecx), "=d" (edx) );
    abcd[0] = eax; abcd[1] = ebx; abcd[2] = ecx; abcd[3] = edx;
#endif
}

int check_xcr0_ymm()
{
    uint32_t xcr0;
#if defined(_MSC_VER)
    xcr0 = (uint32_t)_xgetbv(0);  /* min VS2010 SP1 compiler is required */
#else
    __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx" );
#endif
    return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}


int check_4th_gen_intel_core_features()
{
    uint32_t abcd[4];
    uint32_t fma_movbe_osxsave_mask = ((1 << 12) | (1 << 22) | (1 << 27));
    uint32_t avx2_bmi12_mask = (1 << 5) | (1 << 3) | (1 << 8);

    /* CPUID.(EAX=01H, ECX=0H):ECX.FMA[bit 12]==1   &&
       CPUID.(EAX=01H, ECX=0H):ECX.MOVBE[bit 22]==1 &&
       CPUID.(EAX=01H, ECX=0H):ECX.OSXSAVE[bit 27]==1 */
    run_cpuid( 1, 0, abcd );
    if ( (abcd[2] & fma_movbe_osxsave_mask) != fma_movbe_osxsave_mask )
        return 0;

    if ( ! check_xcr0_ymm() )
        return 0;

    /*  CPUID.(EAX=07H, ECX=0H):EBX.AVX2[bit 5]==1  &&
        CPUID.(EAX=07H, ECX=0H):EBX.BMI1[bit 3]==1  &&
        CPUID.(EAX=07H, ECX=0H):EBX.BMI2[bit 8]==1  */
    run_cpuid( 7, 0, abcd );
    if ( (abcd[1] & avx2_bmi12_mask) != avx2_bmi12_mask )
        return 0;

    /* CPUID.(EAX=80000001H):ECX.LZCNT[bit 5]==1 */
    run_cpuid( 0x80000001, 0, abcd );
    if ( (abcd[2] & (1 << 5)) == 0)
        return 0;

    return 1;
}

#endif /* non-Intel compiler */


static int can_use_intel_core_4th_gen_features()
{
    static int the_4th_gen_features_available = -1;
    /* test is performed once */
    if (the_4th_gen_features_available < 0 )
        the_4th_gen_features_available = check_4th_gen_intel_core_features();

    return the_4th_gen_features_available;
}
#endif

static bool isAVXSupported()
{
	bool avxSupported = false;

	// If Visual Studio 2010 SP1 or later
#if (_MSC_FULL_VER >= 160040219)
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

	bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
	bool cpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

	if( osUsesXSAVE_XRSTORE && cpuAVXSuport )
	{
		// Check if the OS will save the YMM registers
		unsigned long long xcrFeatureMask = _xgetbv( _XCR_XFEATURE_ENABLED_MASK );
		avxSupported = (xcrFeatureMask & 0x6) || false;
	}
#endif
	return avxSupported;
}

static bool is_AVX2_supported()	// do not call this without checking for os support for avx1
{
	bool avs2_supported = false;

	// If Visual Studio 2010 SP1 or later
#if (_MSC_FULL_VER >= 160040219)
#define _XCR_XFEATURE_ENABLED_MASK 0

	int cpuInfo[4];
	get_cpuid( cpuInfo, 7 );	// request cpuid leaf 7

	// avx2 is bit 5
	avs2_supported = (cpuInfo[1] & (1 << 5)) != 0;
#endif
	return avs2_supported;
}

unsigned int win32::optimization::getCPUFeatures()
{
	static bool bInit = false;
	static unsigned int cpu_features = 0;
	if( !bInit )
	{
		int CPUInfo[4] = {};
		int InfoType = 1;
		get_cpuid( CPUInfo,InfoType );
		cpu_features |= (CPUInfo[3] & BIT(23)) ? CPU_MMX : 0;
		cpu_features |= (CPUInfo[3] & BIT(25)) ? CPU_SSE1 : 0;
		cpu_features |= (CPUInfo[3] & BIT(26)) ? CPU_SSE2 : 0;
		cpu_features |= (CPUInfo[2] & BIT(0)) ? CPU_SSE3 : 0;
		cpu_features |= (CPUInfo[2] & BIT(9)) ? CPU_SSSE3 : 0;
		cpu_features |= (CPUInfo[2] & BIT(19)) ? CPU_SSE41 : 0;
		cpu_features |= (CPUInfo[2] & BIT(20)) ? CPU_SSE42 : 0;
		if( isAVXSupported() )	// this includes a check for os support (since Win7 SP1)
		{
			cpu_features |= CPU_AVX;
			cpu_features |= is_AVX2_supported() ? CPU_AVX2 : 0;
		}

		bInit = true;
	}
	return cpu_features;
}
