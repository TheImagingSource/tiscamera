
#ifndef BY8_BASE_H_INC_
#define BY8_BASE_H_INC_

#pragma once

#include "../image_transform_base.h"

#pragma warning ( disable : 4201 ) // nonstandard extension used : nameless struct/union

namespace img {

    inline bool		isBayer16FCC( uint32_t fcc )
    {
        switch( fcc )
        {
        case FOURCC_BGGR16:
        case FOURCC_GBRG16:
        case FOURCC_RGGB16:
        case FOURCC_GRBG16:
            return true;
        default:
            return false;
        };
    }
    inline bool		isBayerFCC( uint32_t fcc )      // maybe move this into by8_transform
    {
        switch( fcc )
        {
        case FOURCC_BY8:		// this is the default
        case FOURCC_BGGR8:
        case FOURCC_GBRG8:
        case FOURCC_RGGB8:
        case FOURCC_GRBG8:
            return true;
        default:
            return false;
        };
    }
}

namespace by8_transform
{
	enum tBY8Pattern
	{
		BG = 0,
		GB,
		GR,
		RG,
	};

	inline tBY8Pattern	convertFCCToPattern( uint32_t fcc )
	{
		switch( fcc )
		{
		case FOURCC_BGGR8:	return by8_transform::BG;
		case FOURCC_GBRG8:	return by8_transform::GB;
		case FOURCC_RGGB8:	return by8_transform::RG;
		case FOURCC_GRBG8:	return by8_transform::GR;

		case FOURCC_BGGR16:	return by8_transform::BG;
		case FOURCC_GBRG16:	return by8_transform::GB;
		case FOURCC_RGGB16:	return by8_transform::RG;
		case FOURCC_GRBG16:	return by8_transform::GR;
		};
		return GB;
	}

	inline uint32_t	convertPatternToFCC( by8_transform::tBY8Pattern pattern )
	{
		switch( pattern )
		{
		case by8_transform::BG:	return FOURCC_BGGR8;
		case by8_transform::GB:	return FOURCC_GBRG8;
		case by8_transform::RG: return FOURCC_RGGB8;
		case by8_transform::GR: return FOURCC_GRBG8;
		};
		return 0;
	}


    inline uint32_t	convertPatternToBY16FCC( by8_transform::tBY8Pattern pattern )
    {
        switch( pattern )
        {
        case by8_transform::BG:	return FOURCC_BGGR16;
        case by8_transform::GB:	return FOURCC_GBRG16;
        case by8_transform::RG: return FOURCC_RGGB16;
        case by8_transform::GR: return FOURCC_GRBG16;
        };
        return 0;
    }

	struct color_matrix 
	{
		union
		{
			struct  
			{
				int16_t r_rfac, r_gfac, r_bfac,
					g_rfac, g_gfac, g_bfac,
					b_rfac, b_gfac, b_bfac;
			};
			int16_t fac[9];
		};
	};


	namespace by8_pattern {
		inline tBY8Pattern	next_pixel( tBY8Pattern pattern )
		{
			switch( pattern )
			{
			case BG:	return GB;
			case GB:	return BG;
			case GR:	return RG;
			case RG:	return GR;
			};
			return BG;
		}
		inline tBY8Pattern	next_line( tBY8Pattern pattern )
		{
			switch( pattern )
			{
			case BG:	return GR;
			case GB:	return RG;
			case GR:	return BG;
			case RG:	return GB;
			};
			return BG;
		}

        template<tBY8Pattern TLinePattern>
        struct		traits	{};

        template<>		struct traits < BG > {
            static const tBY8Pattern	cur_pixel = BG;
            static const tBY8Pattern	cur_line = BG;
            static const tBY8Pattern	next_pixel = GB;
            static const tBY8Pattern	prev_pixel = GB;
            static const tBY8Pattern	prev_line = GR;
            static const tBY8Pattern	next_line = GR;
        };
        template<>		struct traits < GB > {
            static const tBY8Pattern	cur_pixel = GB;
            static const tBY8Pattern	cur_line = GB;
            static const tBY8Pattern	next_pixel = BG;
            static const tBY8Pattern	prev_pixel = BG;
            static const tBY8Pattern	prev_line = RG;
            static const tBY8Pattern	next_line = RG;
        };

        template<>		struct traits < GR > {
            static const tBY8Pattern	cur_pixel = GR;
            static const tBY8Pattern	cur_line = GR;
            static const tBY8Pattern	next_pixel = RG;
            static const tBY8Pattern	prev_pixel = RG;
            static const tBY8Pattern	prev_line = BG;
            static const tBY8Pattern	next_line = BG;
        };
        template<>		struct traits < RG > {
            static const tBY8Pattern	cur_pixel = RG;
            static const tBY8Pattern	cur_line = RG;
            static const tBY8Pattern	next_pixel = GR;
            static const tBY8Pattern	prev_pixel = GR;
            static const tBY8Pattern	prev_line = GB;
            static const tBY8Pattern	next_line = GB;
        };

	};
};

#endif // BY8_BASE_H_INC_