
include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -flto")

set( TCAM_ALG_BACKEND_SRC
  auto_alg/auto_alg.cpp
  auto_alg/auto_alg_pass.cpp
  auto_alg/auto_exposure.cpp
  auto_alg/auto_focus.cpp
  auto_alg/auto_wb_cam.cpp
  auto_alg/auto_wb.cpp

  by8/by8_transform_base.cpp
  by8/by8torgb_conv.cpp
  by8/by8_transform_c_code.cpp
  by8/by8_apply_whitebalance_c.cpp

  img/cpu_features.cpp
  img/img_overlay.cpp
  img/memcpy_image.cpp
  img/memcpy_opt.cpp
  img/img_desc_tools.cpp

  yuv/yuv_planar_apply_params.cpp
  yuv/yuv_planar_transform_impl.cpp
  )


set(TCAM_ALG_SRC
  tcam_transform.cpp
  tcam_auto_alg.cpp
  )

set(TCAM_ALG_HEADER tcam_auto_alg.h tcam_auto_alg_c.h)

add_library(alg_back OBJECT ${TCAM_ALG_BACKEND_SRC})

# assure private symbols stay private
target_compile_options(alg_back PRIVATE "-fvisibility=hidden")

add_library(tcam_algorithms SHARED ${TCAM_ALG_SRC} $<TARGET_OBJECTS:alg_back>)
