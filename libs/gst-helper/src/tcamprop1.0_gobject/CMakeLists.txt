
set( proj-include-dir "${libraries-include-path}/tcamprop1.0_gobject" )

add_library( tcamprop1_provider_gobject STATIC
	
	"${proj-include-dir}/tcam_gerror.h"
	"${proj-include-dir}/tcam_property_provider.h"
	"${proj-include-dir}/tcam_property_provider_simple_functions.h"
	"${proj-include-dir}/tcam_property_serialize.h"
	
	"tcam_gerror.cpp"
	"tcam_property_provider.cpp"
	"tcam_property_provider_simple_functions.cpp"
	"tcam_property_serialize.cpp"

	"tcam_propnode_impl.h"
	"tcam_propboolean_impl.cpp"
	"tcam_propinteger_impl.cpp"
	"tcam_propfloat_impl.cpp"
	"tcam_propenumeration_impl.cpp"
	"tcam_propcommand_impl.cpp"
	"tcam_propstring.cpp"
	"tcam_propnode_impl.cpp"

	"guard_state.h"
	"guard_state.cpp"
)

target_compile_features( tcamprop1_provider_gobject PUBLIC cxx_std_17 )
target_link_libraries( tcamprop1_provider_gobject
PUBLIC
	tcamprop1::provider 
PRIVATE
	tcam::tcam-property 
	fmt::fmt 
	project-warnings 
	tcam::gst-helper
)
target_include_directories( tcamprop1_provider_gobject PRIVATE ${GSTREAMER_INCLUDE_DIRS} )
add_library( tcamprop1::provider_gobject ALIAS tcamprop1_provider_gobject )
