

add_library( dutils_img_filter_neon STATIC

	"by_edge/by_edge.h"
	"by_edge/by_edge_internal.h"
	"by_edge/by8_edge_neonv8_v0.cpp"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_neon_v0.cpp"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_neon_v0.cpp"

	"transform/fcc1x_packed/transform_fcc1x_to_fcc8.h"
	"transform/fcc1x_packed/transform_fcc1x_to_fcc8_neon.cpp"



	"filter/whitebalance/wb_apply.h"
	"filter/whitebalance/wb_apply_neon.cpp"
	"filter/whitebalance/wb_apply_by8_neon.cpp"
	"filter/whitebalance/wb_apply_by16_neon.cpp"

	"transform/mono_to_bgr/transform_mono_to_bgr_neon.cpp"

	"transform/fcc8_fcc16/transform_fcc8_fcc16_neon_v0.cpp"
)

target_link_libraries( dutils_img_filter_neon
PUBLIC
	dutils_img::dutils_img_filter_c
PRIVATE
	dutils_img::project_options
	dutils_img::project_warnings
)

if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" )

elseif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" )

target_compile_options( dutils_img_filter_neon PUBLIC -mfpu=neon-vfpv4 )	# This is needed as a minimum for building this neon code in arm32 mode

endif()


add_library( dutils_img::img_filter_optimized ALIAS dutils_img_filter_neon )
