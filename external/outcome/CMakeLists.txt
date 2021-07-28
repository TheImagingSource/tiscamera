

add_library( outcome INTERFACE )
target_sources( outcome 
INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/include/outcome/outcome.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/outcome/result.hpp"
)
target_include_directories( outcome INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" )
add_library( outcome::outcome ALIAS outcome )

