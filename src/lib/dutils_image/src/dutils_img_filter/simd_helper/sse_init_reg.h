
#ifndef SSE_INIT_REG_H_INC__
#define SSE_INIT_REG_H_INC__

#pragma once

// defines basic register init macros

#include <cstdint> // uint8_t

#if defined(_MSC_VER)

#define CAST_REG_PARAM_( v )  ((__int8)(uint8_t)((v) & 0xFF))

#define INIT_M128i_REG( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15 )	\
        { CAST_REG_PARAM_( v0 ), CAST_REG_PARAM_( v1 ),CAST_REG_PARAM_( v2 ),CAST_REG_PARAM_( v3 ),CAST_REG_PARAM_( v4 ),CAST_REG_PARAM_( v5 ),CAST_REG_PARAM_( v6 ),CAST_REG_PARAM_( v7 ),CAST_REG_PARAM_( v8 ),CAST_REG_PARAM_( v9 ), \
            CAST_REG_PARAM_( v10 ),CAST_REG_PARAM_( v11 ),CAST_REG_PARAM_( v12 ),CAST_REG_PARAM_( v13 ),CAST_REG_PARAM_( v14 ),CAST_REG_PARAM_( v15 ) }

#define INIT_M256i_REG( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                        v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31 )	\
        {   CAST_REG_PARAM_( v0 ), CAST_REG_PARAM_( v1 ), CAST_REG_PARAM_( v2 ),CAST_REG_PARAM_( v3 ),CAST_REG_PARAM_( v4 ),CAST_REG_PARAM_( v5 ),CAST_REG_PARAM_( v6 ),CAST_REG_PARAM_( v7 ), \
            CAST_REG_PARAM_( v8 ),CAST_REG_PARAM_( v9 ), CAST_REG_PARAM_( v10 ),CAST_REG_PARAM_( v11 ),CAST_REG_PARAM_( v12 ),CAST_REG_PARAM_( v13 ),CAST_REG_PARAM_( v14 ),CAST_REG_PARAM_( v15 ), \
            CAST_REG_PARAM_( v16 ), CAST_REG_PARAM_( v17 ), CAST_REG_PARAM_( v18 ), CAST_REG_PARAM_( v19 ), CAST_REG_PARAM_( v20 ), CAST_REG_PARAM_( v21 ), CAST_REG_PARAM_( v22 ), CAST_REG_PARAM_( v23 ),\
            CAST_REG_PARAM_( v24 ), CAST_REG_PARAM_( v25 ), CAST_REG_PARAM_( v26 ), CAST_REG_PARAM_( v27 ), CAST_REG_PARAM_( v28 ), CAST_REG_PARAM_( v29 ), CAST_REG_PARAM_( v30 ), CAST_REG_PARAM_( v31 ),\
        }

#define INIT_M128_PS( f0, f1, f2, f3 ) \
                { (f0), (f1), (f2), (f3) }

#define INIT_M256_PS( f0, f1, f2, f3, f4, f5, f6, f7 ) \
                { (f0), (f1), (f2), (f3), (f4), (f5), (f6), (f7) }


#else


#define CAST_REG_PARAM_( v )  ((uint8_t)((v) & 0xFF))

#define INIT_SINGLE_LL_EPU8( v0, v1, v2, v3, v4, v5, v6, v7 )     \
                                            ((long long)(CAST_REG_PARAM_( v0 ))) << (8*0) |      \
                                            ((long long)(CAST_REG_PARAM_( v1 ))) << (8*1) |      \
                                            ((long long)(CAST_REG_PARAM_( v2 ))) << (8*2) |      \
                                            ((long long)(CAST_REG_PARAM_( v3 ))) << (8*3) |      \
                                            ((long long)(CAST_REG_PARAM_( v4 ))) << (8*4) |      \
                                            ((long long)(CAST_REG_PARAM_( v5 ))) << (8*5) |      \
                                            ((long long)(CAST_REG_PARAM_( v6 ))) << (8*6) |      \
                                            ((long long)(CAST_REG_PARAM_( v7 ))) << (8*7)



#define INIT_M128i_REG( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15 )	\
    { INIT_SINGLE_LL_EPU8( v0, v1, v2, v3, v4, v5, v6, v7 ), INIT_SINGLE_LL_EPU8( v8, v9, v10, v11, v12, v13, v14, v15 ) }

#define INIT_M256i_REG( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                        v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31 )	\
    { INIT_SINGLE_LL_EPU8( v0, v1, v2, v3, v4, v5, v6, v7 ), INIT_SINGLE_LL_EPU8( v8, v9, v10, v11, v12, v13, v14, v15 ),       \
        INIT_SINGLE_LL_EPU8( v16, v17, v18, v19, v20, v21, v22, v23 ), INIT_SINGLE_LL_EPU8( v24, v25, v26, v27, v28, v29, v30, v31 ) }



#define INIT_M128_PS( f0, f1, f2, f3 ) \
    { (f0), (f1), (f2), (f3) }

#define INIT_M256_PS( f0, f1, f2, f3, f4, f5, f6, f7 ) \
    { (f0), (f1), (f2), (f3), (f4), (f5), (f6), (f7) }

#endif

#define INIT_M128_SET1_PS( fs )                     INIT_M128_PS( (fs), (fs), (fs), (fs) )
#define INIT_M256_SET1_PS( fs )                     INIT_M256_PS( (fs), (fs), (fs), (fs), (fs), (fs), (fs), (fs) )
#define INIT_M256_SET_PS( v0, v1, v2, v3, v4, v5, v6, v7 )                     INIT_M256_PS( (v0), (v1), (v2), (v3), (v4), (v5), (v6), (v7) )

#define INIT_M256i_SET16_EPU8( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF )   \
                                                    INIT_M256i_REG( (v0), (v1), (v2), (v3), (v4), (v5), (v6), (v7), (v8), (v9), (vA), (vB), (vC), (vD), (vE), (vF), \
                                                                    (v0), (v1), (v2), (v3), (v4), (v5), (v6), (v7), (v8), (v9), (vA), (vB), (vC), (vD), (vE), (vF) )

#define INIT_M256i_SET8_EPU8( v0, v1, v2, v3, v4, v5, v6, v7 )    INIT_M256i_SET16_EPU8( (v0), (v1), (v2), (v3), (v4), (v5), (v6), (v7), (v0), (v1), (v2), (v3), (v4), (v5), (v6), (v7) )

#define INIT_M256i_SET4_EPU8( v0, v1, v2, v3 )	    INIT_M256i_SET8_EPU8( (v0), (v1), (v2), (v3), (v0), (v1), (v2), (v3) )

#define INIT_M256i_SET1_EPU32( val )	            INIT_M256i_SET4_EPU8( (uint8_t) ((val) & 0xFF), (uint8_t) (((val)>>8) & 0xFF), (uint8_t) (((val) >>16) & 0xFF), (uint8_t) (((val) >>24) & 0xFF) )
#define INIT_M256i_SET1_EPU16( val )	            INIT_M256i_SET4_EPU8( (uint8_t) ((val) & 0xFF), (uint8_t) (((val)>>8) & 0xFF), (uint8_t) (((val) >> 0) & 0xFF), (uint8_t) (((val) >> 8) & 0xFF) )

#define INIT_M256i_SET8_EPU32( v0, v1, v2, v3, v4, v5, v6, v7 )	    INIT_M256i_REG(   \
                                                                            (uint8_t) ((v0) & 0xFF), (uint8_t) (((v0)>>8) & 0xFF), (uint8_t) (((v0) >>16) & 0xFF), (uint8_t) (((v0) >>24) & 0xFF), \
                                                                            (uint8_t) ((v1) & 0xFF), (uint8_t) (((v1)>>8) & 0xFF), (uint8_t) (((v1) >>16) & 0xFF), (uint8_t) (((v1) >>24) & 0xFF), \
                                                                            (uint8_t) ((v2) & 0xFF), (uint8_t) (((v2)>>8) & 0xFF), (uint8_t) (((v2) >>16) & 0xFF), (uint8_t) (((v2) >>24) & 0xFF), \
                                                                            (uint8_t) ((v3) & 0xFF), (uint8_t) (((v3)>>8) & 0xFF), (uint8_t) (((v3) >>16) & 0xFF), (uint8_t) (((v3) >>24) & 0xFF), \
                                                                            (uint8_t) ((v4) & 0xFF), (uint8_t) (((v4)>>8) & 0xFF), (uint8_t) (((v4) >>16) & 0xFF), (uint8_t) (((v4) >>24) & 0xFF), \
                                                                            (uint8_t) ((v5) & 0xFF), (uint8_t) (((v5)>>8) & 0xFF), (uint8_t) (((v5) >>16) & 0xFF), (uint8_t) (((v5) >>24) & 0xFF), \
                                                                            (uint8_t) ((v6) & 0xFF), (uint8_t) (((v6)>>8) & 0xFF), (uint8_t) (((v6) >>16) & 0xFF), (uint8_t) (((v6) >>24) & 0xFF), \
                                                                            (uint8_t) ((v7) & 0xFF), (uint8_t) (((v7)>>8) & 0xFF), (uint8_t) (((v7) >>16) & 0xFF), (uint8_t) (((v7) >>24) & 0xFF) \
                                                                    )


#define INIT_M128i_SET1_4xEPU8( v0, v1, v2, v3 )	INIT_M128i_REG( (v0), (v1), (v2), (v3), (v0), (v1), (v2), (v3), (v0), (v1), (v2), (v3), (v0), (v1), (v2), (v3) )
#define INIT_M128i_SET1_EPU32( val )	            INIT_M128i_SET1_4xEPU8( (uint8_t) ((val) & 0xFF), (uint8_t) (((val)>>8) & 0xFF), (uint8_t) (((val)>>16) & 0xFF), (uint8_t) (((val)>>24) & 0xFF) )
#define INIT_M128i_SET1_EPU16( val )	            INIT_M128i_SET1_4xEPU8( (uint8_t) ((val) & 0xFF), (uint8_t) (((val)>>8) & 0xFF), (uint8_t) (((val) >> 0) & 0xFF), (uint8_t) (((val) >> 8) & 0xFF) )

#endif // SSE_INIT_REG_H_INC__
