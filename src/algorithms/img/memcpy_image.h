
#ifndef memcpy_image_h__
#define memcpy_image_h__

#include "../image_transform_base.h"

namespace img
{
    void	memcpy_image( img::img_descriptor& dest, const img::img_descriptor& src, unsigned int cpu_features );
	void	memcpy_image( img::img_descriptor& dest, const img::img_descriptor& src, bool bFlip, unsigned int cpu_features );
	void	memcpy_image( byte* dst_ptr, int dst_pitch, byte* src_ptr, int src_pitch, int bytes_per_line, int dim_y, bool bFlip, unsigned int cpu_features );
};


#endif // memcpy_image_h__
