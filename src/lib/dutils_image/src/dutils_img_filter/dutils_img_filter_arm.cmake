

add_library( dutils_img_filter_neon STATIC 

	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_neon_v0.cpp"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_neon_v0.cpp"

	"transform/fcc8_fcc16/transform_fcc8_fcc16_neon_v0.cpp"

#	"filter/whitebalance/wb_apply.h"
#	"filter/whitebalance/wb_apply_sse41.cpp"
#	"filter/whitebalance/wb_apply_by16_sse4_1.cpp"
#	"filter/whitebalance/wb_apply_by8_sse2.cpp"
  
#	"transform/pwl/transform_fccfloat_to_fcc8_sse41_v0.cpp"

#	"transform/mono_to_bgr/transform_mono_to_bgr_sse41.cpp"
)

target_link_libraries( dutils_img_filter_neon
PUBLIC
#	dutils_img::project_options
PRIVATE
#	dutils_img::project_warnings
PUBLIC
	dutils::dutils_img_filter_c
)

if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" )

elseif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" )

target_compile_options( dutils_img_filter_neon PUBLIC -mfpu=neon-vfpv4 )	# This is needed as a minimum for building this neon code in arm32 mode

endif()


add_library( dutils::dutils_img_filter_optimized ALIAS dutils_img_filter_neon )
