

add_library( dutils_img_filter_sse41 STATIC 

	"by_edge/by_edge.h"
	"by_edge/by_edge_internal.h"
	"by_edge/by8_edge_sse4_1_v0.cpp"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_ssse3_v0.cpp"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_ssse3_v0.cpp"

	"transform/fcc8_fcc16/transform_fcc8_fcc16_sse4_v0.cpp"

	"filter/whitebalance/wb_apply.h"
	"filter/whitebalance/wb_apply_sse41.cpp"
	"filter/whitebalance/wb_apply_by16_sse4_1.cpp"
	"filter/whitebalance/wb_apply_by8_sse2.cpp"

	"transform/mono_to_bgr/transform_mono_to_bgr_sse41.cpp"
)

target_link_libraries( dutils_img_filter_sse41
PRIVATE
	dutils_img::dutils_img_filter_c
PRIVATE
	dutils_img::project_options
	dutils_img::project_warnings
)

target_compile_options( dutils_img_filter_sse41 PUBLIC -msse4.1 )

add_library( dutils_img::img_filter_optimized ALIAS dutils_img_filter_sse41 )
