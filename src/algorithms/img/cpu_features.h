
#ifndef CPU_FEATURES_H_INC_
#define CPU_FEATURES_H_INC_

namespace win32
{
	namespace optimization
	{
		enum cpu_features
		{
			CPU_NOFEATURES = 0x0,
			CPU_MMX = 0x1,
			CPU_SSE1 = 0x2,	
			CPU_SSE2 = 0x4,
			CPU_SSE3 = 0x8,		// very few interesting intrinsics, horz add
			CPU_SSSE3 = 0x10,	// PSIGNB, pabs, pshufb
			CPU_SSE41 = 0x20,	// pmin/pmax, blend, pinsert/pextract, pmovzxbw, ptest, packusdw
			CPU_SSE42 = 0x40,	// CRC32, pcmpestr, popcnt
			CPU_SSE4a = 0x80,	// amd extension, don't use
			CPU_AVX = 0x100,	// AVX, ymm, 512bit floating point
			CPU_AVX2 = 0x200,	// AVX2, ymm 512bit integer, gather ....
            CPU_AVX512 = 0x400, // extension to AVX 512 registers. currently not implemented (due to not yet supported)

			CPU_MAX_FEATURE = 0xFFF,

			CPU_UsesSSE1 = CPU_MMX | CPU_SSE1,
			CPU_UsesSSE2 = CPU_UsesSSE1 | CPU_SSE2,
			CPU_UsesSSE3 = CPU_UsesSSE2 | CPU_SSE3,
			CPU_UsesSSSE3 = CPU_UsesSSE3 | CPU_SSSE3,
			CPU_UsesSSE41 = CPU_UsesSSSE3 | CPU_SSE41,
			CPU_UsesSSE42 = CPU_UsesSSE41 | CPU_SSE42,
			CPU_UsesAVX = CPU_UsesSSE42 | CPU_AVX,
			CPU_UsesAVX2 = CPU_UsesAVX | CPU_AVX2,
            CPU_UsesAVX512 = CPU_UsesAVX2 | CPU_AVX2,

			// even though a cpu might have features, a specification of this uses the lowest denominator
			// all algorithms should implement a C variant.
			CPU_C_ALGORITHM =			CPU_NOFEATURES,

			CPU_PENTIUM_NOSSE2_CLASS =	CPU_MMX | CPU_SSE1,
			CPU_PENTIUM_CLASS =			CPU_MMX | CPU_SSE1 | CPU_SSE2,
			CPU_PENTIUM_HT_CLASS =		CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3,
			CPU_AMD_ATHLON_CLASS =		CPU_MMX | CPU_SSE1,
			CPU_AMD_ATHLON64_CLASS =	CPU_MMX | CPU_SSE1 | CPU_SSE2,
			CPU_AMD_ATHLONX2_CLASS =	CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3,
			CPU_ATOM_CLASS =			CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3 | CPU_SSSE3,
			CPU_CORE2_CLASS =			CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3 | CPU_SSSE3 | CPU_SSE41,
			CPU_COREi7_CLASS =			CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3 | CPU_SSSE3 | CPU_SSE41 | CPU_SSE42,						// 2010/1st gen
			CPU_COREi_NEHALEM =			CPU_COREi7_CLASS,
			CPU_COREi_SANDYBRIDGE =		CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3 | CPU_SSSE3 | CPU_SSE41 | CPU_SSE42 | CPU_AVX,	            // 2011/2nd gen
			CPU_COREi_HASWELL =			CPU_MMX | CPU_SSE1 | CPU_SSE2 | CPU_SSE3 | CPU_SSSE3 | CPU_SSE41 | CPU_SSE42 | CPU_AVX | CPU_AVX2,	// 2013/4th gen
		};

		unsigned int	getCPUFeatures();
	};
};

#endif // CPU_FEATURES_H_INC_

