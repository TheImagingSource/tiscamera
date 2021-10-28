
set( proj-include-dir "${libraries-include-path}/tcamprop1.0_consumer" )

add_library( tcamprop1_consumer STATIC
	"${proj-include-dir}/tcamprop1_consumer.h"
	
	"tcamprop1_consumer.cpp"

	"consumer_prop_impl.h"
	"consumer_prop_impl.cpp"
)

target_compile_features( tcamprop1_consumer PUBLIC cxx_std_17 )
target_link_libraries( tcamprop1_consumer PUBLIC tcamprop1::base outcome::outcome tcam::tcam-property  PRIVATE project-warnings tcam::gst-helper )
target_include_directories( tcamprop1_consumer PRIVATE ${GSTREAMER_INCLUDE_DIRS} )

add_library( tcamprop1::consumer ALIAS tcamprop1_consumer )
