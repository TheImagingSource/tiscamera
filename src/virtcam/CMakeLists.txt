# Copyright 2022 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


add_library(tcam-backend-virtcam STATIC
	virtcam_api.h
	virtcam_api.cpp
	virtcam_device.h
	virtcam_device.cpp
    virtcam_properties_impl.h
    virtcam_properties_impl.cpp
    virtcam_properties.cpp
    virtcam_generator.h
    virtcam_generator.cpp
    generator/generator_base.h
    generator/pattern_generator.h
    generator/mono_generator.h
    generator/mono_generator.cpp
    generator/rgb_generator.h
    generator/rgb_generator.cpp
    generator/bayer_generator.h
    generator/bayer_generator.cpp
  )

set_project_warnings(tcam-backend-virtcam)


target_link_libraries(tcam-backend-virtcam
  PUBLIC
  tcam-base
  PRIVATE
  tcamprop1::base
  )

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
  target_precompile_headers(tcam-backend-virtcam REUSE_FROM tcam-base)
endif()
