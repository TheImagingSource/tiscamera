
set( proj-include-dir "${libraries-include-path}/tcamprop_system" )

add_library( tcamprop_system_base STATIC EXCLUDE_FROM_ALL
	"${proj-include-dir}/tcamprop_system_base.h"
	
	"${proj-include-dir}/tcamprop_provider_base.h"
	"${proj-include-dir}/tcamprop_provider_funcbased.h"
	
	"tcamprop_provider_funcbased.cpp"
)
target_compile_features( tcamprop_system_base PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_base 
PRIVATE 
	project-warnings 
PUBLIC 
	outcome::outcome
)
target_include_directories( tcamprop_system_base PUBLIC ${libraries-include-path} )
add_library( tcamprop_system::base ALIAS tcamprop_system_base )

add_library( tcamprop_system_provider STATIC EXCLUDE_FROM_ALL
	"${proj-include-dir}/tcamprop_provider_itf_impl.h"
	
	"tcamprop_provider_itf_impl.cpp"
)

target_compile_features( tcamprop_system_provider PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_provider PUBLIC tcamprop_system::base PRIVATE fmt::fmt project-warnings  )
target_include_directories( tcamprop_system_provider PRIVATE ${GSTREAMER_INCLUDE_DIRS} )
add_library( tcamprop_system::provider ALIAS tcamprop_system_provider )

add_library( tcamprop_system_consumer STATIC EXCLUDE_FROM_ALL
	"${proj-include-dir}/tcamprop_consumer.h"
	
	"tcamprop_consumer.cpp"
)

target_compile_features( tcamprop_system_consumer PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_consumer PUBLIC tcamprop_system::base PRIVATE tcam::tcamprop project-warnings )
target_include_directories( tcamprop_system_consumer PRIVATE ${GSTREAMER_INCLUDE_DIRS} )

add_library( tcamprop_system::consumer ALIAS tcamprop_system_consumer )
