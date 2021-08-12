
add_library( dutils_img_filter_c STATIC 

	"dutils_img_base.h"

	"transform/transform_base.h"
	"transform/fcc8_fcc16/transform_fcc8_fcc16.h"
	"transform/fcc8_fcc16/transform_fcc8_fcc16_internal.h"
	"transform/fcc8_fcc16/transform_fcc8_fcc16_c.cpp"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc_internal.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_internal_loop.h"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc.h"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_pixel.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_internal.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc16_c.cpp"

	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_internal.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_internal_loop.h"
	"transform/fcc1x_packed/fcc1x_packed_to_fcc8_c.cpp"

	"transform/fcc1x_packed/transform_fcc1x_to_fcc8.h"
	"transform/fcc1x_packed/transform_fcc1x_to_fcc8_c.cpp"

	"filter/whitebalance/wb_apply.h"
	"filter/whitebalance/wb_apply_c.cpp"
	"filter/whitebalance/wb_apply_by16_c.cpp"
	"filter/whitebalance/wb_apply_by8_c.cpp"

	"transform/mono_to_bgr/transform_mono_to_bgr.h"
	"transform/mono_to_bgr/transform_mono_to_bgr_internal.h"
	"transform/mono_to_bgr/transform_mono_to_bgr_c.cpp"

	"transform/pwl/transform_pwl_to_bayerfloat_internal.h"
	"transform/pwl/transform_pwl_to_bayerfloat_internal.cpp"
	"transform/pwl/transform_pwl_functions.h"
	"transform/pwl/transform_pwl_to_bayerfloat_c.cpp"


	"by_edge/by_edge.h"
	"by_edge/by_edge_internal.h"
	"by_edge/by8_edge_c.cpp"
	"by_edge/by8_pixelops.h"

)

target_link_libraries( dutils_img_filter_c
PRIVATE
	dutils_img::project_warnings
	dutils_img::project_options
PUBLIC
	dutils_img_base_lib
)

add_library( dutils_img::dutils_img_filter_c ALIAS dutils_img_filter_c )

if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" )

include( dutils_img_filter_arm.cmake )

elseif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" )

include( dutils_img_filter_arm.cmake )

else()

include( dutils_img_filter_intel.cmake )

endif()