
add_library( dutils_img_base_lib STATIC 
	"cpu_features.cpp"
	"cpu_features.h"
	"img_overlay.cpp"
	"img_overlay.h"
	"memcpy_image.cpp"
	"memcpy_image.h"

	"alignment_helper.h"
	"interop_private.h"

	"img_rect_tools.h"
	"img_rect_tools.cpp"
	"dutils_gst_interop.cpp"

	"img_string_helper.cpp"
)

target_link_libraries( dutils_img_base_lib 
PUBLIC 
	dutils_img::img
PRIVATE
	dutils_img::project_options
	dutils_img::project_warnings
)

add_library(dutils_img::base ALIAS dutils_img_base_lib)
