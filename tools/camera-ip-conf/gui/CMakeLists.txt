
# Copyright 2016 The Imaging Source Europe GmbH
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


# Set up QT4 and required components
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_USE_FILE})


set(UIS
  infobox.ui
  mainwindow.ui)
qt4_wrap_ui(GENERATED_SOURCES ${UIS})


add_executable(camera-ip-conf gui_main.cpp InfoBox.cpp MainWindow.cpp UpdateHandler.cpp ${UIS})

target_link_libraries(camera-ip-conf tcam-network)

install(TARGETS camera-ip-conf DESTINATION bin)
