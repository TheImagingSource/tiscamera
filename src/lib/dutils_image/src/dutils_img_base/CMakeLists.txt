
add_library( dutils_img_base STATIC 
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
)

target_link_libraries( dutils_img_base 
PUBLIC 
	dutils::dutils_img
PRIVATE
	dutils::project_options
	dutils::project_warnings
)

add_library( dutils::dutils_img_base ALIAS dutils_img_base )