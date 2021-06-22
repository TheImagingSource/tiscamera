

add_library( dutils_img_filter_sse41 STATIC 

	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_ssse3_v0.cpp"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_ssse3_v0.cpp"

#	"transform/fcc8_fcc16/transform_fcc8_fcc16_sse4_v0.cpp"

#	"filter/whitebalance/wb_apply.h"
#	"filter/whitebalance/wb_apply_sse41.cpp"
#	"filter/whitebalance/wb_apply_by16_sse4_1.cpp"
#	"filter/whitebalance/wb_apply_by8_sse2.cpp"
  
#	"transform/pwl/transform_fccfloat_to_fcc8_sse41_v0.cpp"

#	"transform/mono_to_bgr/transform_mono_to_bgr_sse41.cpp"
)

target_link_libraries( dutils_img_filter_sse41
PUBLIC
#	dutils_img::project_options
PRIVATE
#	dutils_img::project_warnings
PUBLIC
	dutils::dutils_img_filter_c
)

target_compile_options( dutils_img_filter_sse41 PUBLIC -msse4.1 )

add_library( dutils::dutils_img_filter_optimized ALIAS dutils_img_filter_sse41 )
