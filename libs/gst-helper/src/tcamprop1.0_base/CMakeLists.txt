
set( proj-include-dir "${libraries-include-path}/tcamprop1.0_base" )

add_library( tcamprop1_base STATIC
	"${proj-include-dir}/tcamprop_base.h"
	"${proj-include-dir}/tcamprop_errors.h"

	"${proj-include-dir}/tcamprop_property_info.h"
	"${proj-include-dir}/tcamprop_property_info_list.h"

	"tcamprop_errors.cpp"
	"tcamprop_property_info.cpp"
	"tcamprop_property_info_list.cpp"
)

target_compile_features( tcamprop1_base PUBLIC cxx_std_17 )
target_link_libraries( tcamprop1_base 
PRIVATE 
	project-warnings 
)
target_include_directories( tcamprop1_base PUBLIC ${libraries-include-path} )
add_library( tcamprop1::base ALIAS tcamprop1_base )

add_library( tcamprop1_provider STATIC
	"${proj-include-dir}/tcamprop_property_interface.h"

	"${proj-include-dir}/tcamprop_property_list_impl.h"
	
	"tcamprop_property_list_impl.cpp"
	"tcamprop_property_interface_impl.cpp"
)

target_compile_features( tcamprop1_provider PUBLIC cxx_std_17 )
target_link_libraries( tcamprop1_provider PUBLIC tcamprop1::base outcome::outcome PRIVATE project-warnings )
add_library( tcamprop1::provider ALIAS tcamprop1_provider )
