

include_directories(/home/edt/include)
include_directories(/home/edt/work/drvbuildtools/trunk/lib/dutils_image/)
include_directories(/home/edt/work/tiscamera/src)
# set(CMAKE_SHARED_LINKER_FLAGS "-L/home/edt/drvbuildtools/trunk/lib/dutils_image/build -ltis_auto_alg")

add_library(tisfilter_autopass SHARED AutoPassFilter.cpp)
target_link_libraries(tisfilter_autopass "-L/home/edt/lib -ltis_auto_alg")

add_library(tisfilter_bayer2rgb SHARED BayerRgbFilter.cpp)
target_link_libraries(tisfilter_bayer2rgb "-L/home/edt/lib -ltis_auto_alg")


install(TARGETS tisfilter_autopass tisfilter_bayer2rgb DESTINATION lib)
