
set( gst-helper-include-dir "${libraries-include-path}/gst-helper" )

##########################
# Library tcam::gst-helper

add_library( gst-helper STATIC 

	"${gst-helper-include-dir}/g_signal_helper.h"
	"${gst-helper-include-dir}/gst_element_chain.h"
	"${gst-helper-include-dir}/gst_ptr.h"
	"${gst-helper-include-dir}/gst_signal_helper.h"
	"${gst-helper-include-dir}/gvalue_helper.h"
	"${gst-helper-include-dir}/gvalue_wrapper.h"
	"${gst-helper-include-dir}/helper_functions.h"

	"gst_element_chain.cpp"
	"gvalue_helper.cpp"
)
target_include_directories( gst-helper
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/
	${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries( gst-helper PUBLIC ${GSTREAMER_LIBRARIES} )

add_library( tcam::gst-helper ALIAS gst-helper )

##########################
# Library tcam::gst-helper-dutils

add_library( gst-helper-dutils STATIC 

	"${gst-helper-include-dir}/gstcaps_dutils_interop.h"

	"gstcaps_dutils_interop.cpp"
)

target_link_libraries( gst-helper-dutils PUBLIC dutils_img::base tcam::gst-helper )

add_library( tcam::gst-helper-dutils ALIAS gst-helper-dutils )
