
#ifndef DUTILS_CPU_FEATURES_H_INC_
#define DUTILS_CPU_FEATURES_H_INC_

#pragma once


/** 

When this is a A64 target, the following are defined:
     DUTILS_ARM_ARCH_A64
     DUTILS_ARM_ARCH_A8
     DUTILS_ARM_ARCH

When this is a A8 target, the following are defined:
     DUTILS_ARM_ARCH_A8
     DUTILS_ARM_ARCH

When this is a ARM CPU older then A8 only the following is defined:
    DUTILS_ARM_ARCH 

When this is a Intel compatible CPU the following is defined as the minimum this library accepts

    DUTILS_ARCH_SSE41

 */

#if defined _M_ARM || defined __arm__ || defined __aarch64__

// See echo | gcc -dM -E - -march=native

#define DUTILS_ARCH_ARM 1


#if defined __aarch64__
#  define DUTILS_ARCH_ARM_A64         1
#  define DUTILS_ARCH_ARM_A8          1
#  define DUTILS_ARCH_ARM             1

#elif defined __ARM_ARCH_8__ || defined __ARM_ARCH_8A__  || (defined __ARM_ARCH && __ARM_ARCH == 8)

#  define DUTILS_ARCH_ARM_A8          1
#  define DUTILS_ARCH_ARM             1

#elif defined __ARM_NEON || defined __ARM_NEON__ || defined __ARM_ARCH_7__ || (defined __ARM_ARCH && __ARM_ARCH == 7)

#  define DUTILS_ARCH_ARM             1

#else

#  define DUTILS_ARCH_ARM             1

#endif

#else

#if defined __AVX512F__ && defined __AVX512BW__ && defined __AVX512DQ__ && defined __AVX512VL__
#define DUTILS_ARCH_AVX512_BASE       1
#endif

#if defined __AVX2__
#define DUTILS_ARCH_AVX2              1
#endif
#if defined __AVX__
#define DUTILS_ARCH_AVX1              1
#endif

#define DUTILS_ARCH_SSE41             1


#endif



namespace img {
namespace cpu
{
#if defined DUTILS_ARCH_ARM
    enum cpu_features
    {
        CPU_NOFEATURES          = 0x0000,
        CPU_C                   = 0x0001,
        CPU_ARM_A7              = 0x0002,  // supported by ARM Cortex-A7 processors (raspberry pi 2 + 3) (Note: this is not AARCH32)
        CPU_ARM_A8              = 0x0004,  // supported by ARM Cortex-A8 processors, this is AARCH32
        CPU_ARM_A64             = 0x0008,  // supported by ARM Cortex-A8+ processors 64-bit OS, this also known as AARCH64

        // Each step contains the previous + a new flag
        // even though a cpu might have features, a specification of this uses the lowest denominator
        // all algorithms should implement a C variant.
        CPU_C_ALGORITHM = CPU_C,

        CPU_UsesARM_A7 = CPU_ARM_A7 | CPU_C,
        CPU_UsesARM_A8 = CPU_ARM_A8 | CPU_UsesARM_A7,
        CPU_UsesARM_A64 = CPU_ARM_A64 | CPU_UsesARM_A8,

        CPU_JETSON_NANO = CPU_UsesARM_A64,                   // Cortex-A57, 64bit OS
        CPU_RPI4 = CPU_UsesARM_A8,                          // Cortex-A57, 64bit OS
    };
#else
    enum cpu_features
    {
        CPU_NOFEATURES  = 0x0000,
        CPU_C           = 0x0001,
        CPU_SSSE3       = 0x0002,	// currently minimum supported PSIGNB, pabs, pshufb
        CPU_SSE41       = 0x0004,	// pmin/pmax, blend, pinsert/pextract, pmovzxbw, ptest, packusdw
        CPU_SSE42       = 0x0008,	// CRC32, pcmpestr, popcnt
        CPU_AVX1        = 0x0010,	// AVX, ymm, 512bit floating point
        CPU_AVX2        = 0x0020,	// AVX2, ymm 512bit integer, gather ....
        CPU_FMA3        = 0x0040,   // FMA3, as supported by AVX2 CPUs, separate flag used in CPUs
            
        // skip 0x0080
        CPU_AVX512_F        = 0x0100,       // expands most 32-bit and 64-bit based AVX instructions with EVEX coding scheme to support 512-bit registers, operation masks, parameter broadcasting, and embedded rounding and exception control
        CPU_AVX512_CD       = 0x0200,       // Conflict Detection Instructions (CDI) – efficient conflict detection to allow more loops to be vectorized
        CPU_AVX512_BW       = 0x0400,       // Byte and Word Instructions (BW) – extends AVX-512 to cover 8-bit and 16-bit integer operations
        CPU_AVX512_DQ       = 0x0800,       // Doubleword and Quadword Instructions (DQ) – adds new 32-bit and 64-bit AVX-512 instructions
        CPU_AVX512_VL       = 0x1000,       // Vector Length Extensions (VL) – extends most AVX-512 operations to also operate on XMM (128-bit) and YMM (256-bit) registers
            
        CPU_AVX512_IFMA     = 0x2000,       // AVX-512 Integer Fused Multiply Add (IFMA) - fused multiply add of integers using 52-bit precision.
        CPU_AVX512_VBMI     = 0x4000,       // AVX-512 Vector Byte Manipulation Instructions (VBMI) adds vector byte permutation instructions which were not present in AVX-512BW.
        CPU_AVX512_VPOPCNTDQ = 0x8000,      // Vector population count instruction. Introduced with Knights Mill and Ice Lake.[7]

        CPU_AVX512_VNNI     = 0x0001'0000,  // AVX-512 Vector Neural Network Instructions (VNNI) - vector instructions for deep learning.
        CPU_AVX512_VBMI2    = 0x0002'0000,  // AVX-512 Vector Byte Manipulation Instructions 2 (VBMI2) - byte/word load, store and concatenation with shift.
        CPU_AVX512_BITALG   = 0x0004'0000,  // AVX-512 Bit Algorithms (BITALG) - byte/word bit manipulation instructions expanding VPOPCNTDQ.
        
        CPU_AVX512_GFNI       = 0x0008'0000,      // EVEX-encoded Galois field new instructions:
        CPU_AVX512_VPCLMULQDQ = 0x0010'0000,      // VPCLMULQDQ with AVX-512F adds EVEX-encoded 512-bit version of PCLMULQDQ instruction
        CPU_AVX512_VAES       = 0x0020'0000,      // EVEX-encoded AES instructions. 

        CPU_AVX512_VP2INTERSECT = 0x0040'0000,   // AVX-512 Vector Pair Intersection to a Pair of Mask Registers (VP2INTERSECT).

        CPU_AVX512_BF16 = 0x0080'0000,

        // most likely only on Xeon Phi, Knights landing (dead?)
        //CPU_AVX512_ER       = 0,  // Exponential and Reciprocal Instructions (ERI)
        //CPU_AVX512_PF       = 0,  // Prefetch Instructions (ERI)
        //CPU_AVX512_4VNNIW   = 0,  // AVX-512 Vector Neural Network Instructions Word variable precision (4VNNIW) - vector instructions for deep learning, enhanced word, variable precision.
        //CPU_AVX512_4FMAPS   = 0,  // AVX-512 Fused Multiply Accumulation Packed Single precision (4FMAPS) - vector instructions for deep learning, floating point, single precision.

        CPU_MAX_FEATURE = 0x0FFF'FFFF,
            
        CPU_UsesSSSE3   = CPU_C | CPU_SSSE3,                             // this is currently our minimum of support for SIMD operations
        CPU_UsesSSE41   = CPU_UsesSSSE3 | CPU_SSE41,
        CPU_UsesSSE42   = CPU_UsesSSE41 | CPU_SSE42,
            
        CPU_UsesAVX1    = CPU_UsesSSE42 | CPU_AVX1,
        CPU_UsesAVX2    = CPU_UsesAVX1 | CPU_AVX2 | CPU_FMA3,
            
        CPU_UsesAVX512_F = CPU_UsesAVX2 | CPU_AVX512_F,
        CPU_UsesAVX512_BASE0 = CPU_UsesAVX512_F | CPU_AVX512_CD | CPU_AVX512_VL | CPU_AVX512_DQ | CPU_AVX512_BW,

        CPU_UsesMaxAvailable = CPU_UsesAVX512_BASE0,
            
        // even though a cpu might have features, a specification of this uses the lowest denominator
        // all algorithms should implement a C variant.
        CPU_C_ALGORITHM             = CPU_C,
            
        //CPU_PENTIUM_CLASS           = CPU_UsesSSE2,                               // Note, there are Pentium CPUs out there without SSE2
        //CPU_PENTIUM_HT_CLASS        = CPU_UsesSSE2 | CPU_SSE3,
        //CPU_AMD_ATHLON64_CLASS      = CPU_UsesSSE2,
        //CPU_AMD_ATHLONX2_CLASS      = CPU_UsesSSE2 | CPU_SSE3,
        //CPU_ATOM_CLASS              = CPU_UsesSSE2 | CPU_SSE3 | CPU_SSSE3,
        //CPU_CORE2_CLASS = CPU_UsesSSE2 | CPU_SSE3 | CPU_SSSE3 | CPU_SSE41,
        CPU_CORE2_CLASS             = CPU_UsesSSE41,  // this is currently our minimum of support for SIMD operations
        CPU_COREi7_CLASS            = CPU_UsesSSE42,

        // Intel core microarchitectures
        CPU_COREi_NEHALEM           = CPU_UsesSSE42,                                    //      2008/1st gen
        CPU_COREi_SANDYBRIDGE       = CPU_UsesAVX1,	                                    // 2xxx 2011/2nd gen
        CPU_COREi_IVY_BRIDGE        = CPU_UsesAVX1,                                     // 3xxx 2012
        CPU_COREi_HASWELL           = CPU_UsesAVX2,                                     // 4xxx 2013/4th gen
        CPU_COREi_BROADWELL         = CPU_UsesAVX2,                                     // 5xxx 2014
        CPU_COREi_SKYLAKE           = CPU_UsesAVX2,                                     // 6xxx 2015
        CPU_COREi_KABYLAKE          = CPU_UsesAVX2,                                     // 7xxx 2016
        CPU_COREi_COFFEELAKE        = CPU_UsesAVX2,                                     // 8xxx
        //CPU_COREi_CANNONLAKE        = CPU_UsesAVX512_BASE0 | CPU_AVX512_IFMA | CPU_AVX512_VBMI,       // 8121U   2018
        CPU_COREi_CASCADELAKE       = CPU_UsesAVX512_BASE0 /*| CPU_AVX512_VNNI */,                      // 109xxX   2019
        CPU_COREi_ICELAKE           = CPU_UsesAVX512_BASE0 | CPU_AVX512_IFMA | CPU_AVX512_VBMI,         // 10xxGx   ~2019
        CPU_COREi_COOPERLAKE        = CPU_UsesAVX512_BASE0 | CPU_AVX512_VNNI | CPU_AVX512_BF16,         // ??   2020
        CPU_COREi_TIGERLAKE         = CPU_UsesAVX512_BASE0 | CPU_AVX512_IFMA | CPU_AVX512_VBMI,         // 10xxGx   ~2019

        CPU_ZEN1                    = CPU_UsesAVX2,                                     // Ryzen 1xxx 2017
        CPU_ZEN2                    = CPU_UsesAVX2,                                     // Ryzen 3xxx 2019
        CPU_ZEN3                    = CPU_UsesAVX2,                                     // Ryzen 4xxx 2020 ????

        // Intel Atom microarchitectures
        CPU_ATOM_BONELL             = CPU_UsesSSSE3,                                    // 2008
        CPU_ATOM_SILVERMONT         = CPU_UsesSSE42,                                    // 2013
        CPU_ATOM_GOLDMONT           = CPU_UsesSSE42,                                    // 2016, N3450, J3455, E3940
    };
#endif
}
}

#endif // CPU_FEATURES_H_INC_

