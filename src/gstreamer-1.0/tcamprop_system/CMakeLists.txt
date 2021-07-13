
find_package(PkgConfig REQUIRED)


# If this errors, you have not installed all necessary libraries
# This generates the variables GSTREAMER_INCLUDE_DIRS and GSTREAMER_LIBRARIES
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

add_library( tcamprop_system_base STATIC EXCLUDE_FROM_ALL
	tcamprop_system_base.h
	
	tcamprop_provider_base.h
	tcamprop_provider_funcbased.h
	tcamprop_provider_funcbased.cpp
)
target_compile_features( tcamprop_system_base PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_base PUBLIC project-warnings outcome::outcome )
add_library( tcamprop_system::base ALIAS tcamprop_system_base )

add_library( tcamprop_system_provider STATIC EXCLUDE_FROM_ALL
	tcamprop_provider_itf_impl.h
	tcamprop_provider_itf_impl.cpp
)

target_compile_features( tcamprop_system_provider PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_provider PUBLIC tcamprop_system::base PRIVATE spdlog::spdlog )
target_include_directories( tcamprop_system_provider PRIVATE ${GSTREAMER_INCLUDE_DIRS} )
add_library( tcamprop_system::provider ALIAS tcamprop_system_provider )

add_library( tcamprop_system_consumer STATIC EXCLUDE_FROM_ALL
	tcamprop_consumer.h
	tcamprop_consumer.cpp
)

target_compile_features( tcamprop_system_consumer PUBLIC cxx_std_17 )
target_link_libraries( tcamprop_system_consumer PUBLIC tcamprop_system::base PRIVATE tcam::tcamprop )
target_include_directories( tcamprop_system_consumer PRIVATE ${GSTREAMER_INCLUDE_DIRS} )

add_library( tcamprop_system::consumer ALIAS tcamprop_system_consumer )
