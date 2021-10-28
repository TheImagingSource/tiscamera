# Copyright 2018 The Imaging Source Europe GmbH
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

include(git-helper)
include(CPackComponent)

# The package name is created according to the following rules:
#
# If is is an official release on the master branch:
#     tiscamera_MAJ.MIN.PAT_arch.deb
#
# If it is a preview release:
#     tiscamera_MAJ.MIN.PAT.COMMIT_COUNT~branch_COMMIT_HASH_arch.deb
#
# If a git tag is present it will replace branch and commit hash
#     tiscamera_MAJ.MIN.PAT.COMMIT_COUNT_tag_arch.deb
#
# If the built version is NOT a Release build,
# the build type will be additionally appended.
# this will always be lower case
#     tiscamera_MAJ.MIN.PAT_arch_buildtype.deb
#
# Manually setting CPACK_DEBIAN_PACKAGE_ARCHITECTURE will overwrite auto detection
# CPACK_DEBIAN_PACKAGE_ARCHITECTURE is set to `dpkg --print-architecture`.
# For Raspberry Pis the arch is appended with piX i.e. armhf_piX
# where X is the pi version that is detected
# To overwrite this set `PACKAGE_NAME_ONLY_ARCH` to ON

function (create_package_name return_value name version)

  if(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
    find_program(EXECUTABLE_DPKG dpkg DOC "dpkg program of Debian-based systems")
    if(EXECUTABLE_DPKG)
      execute_process(
        COMMAND ${EXECUTABLE_DPKG} --print-architecture
        OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif(EXECUTABLE_DPKG)

    string(COMPARE EQUAL "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" "armhf" IS_ARMHF)

    # this check is to identify raspberry pi
    # these systems receive special optimizations that are not always
    # compatible to other armhf systems, thus needs special identification
    if(IS_ARMHF AND NOT PACKAGE_NAME_ONLY_ARCH)

      file(READ "/proc/cpuinfo" CPUINFO)

      # cpuinfo should contain a model description
      # if the OS does not offer that line use
      # Hardware	: BCM2835 for PI4
      # Hardware    : BCM2708 for PI1
      # Hardware    : BCM2709 for PI2 and PI3
      string(FIND "${CPUINFO}" "Raspberry Pi 3" IS_PI3)

      # find returns -1 if not found
      # -1 is not considered false
      if(IS_PI3 GREATER 0)
        string(APPEND CPACK_DEBIAN_PACKAGE_ARCHITECTURE "_pi3")
      endif(IS_PI3 GREATER 0)

      string(FIND "${CPUINFO}" "Raspberry Pi 4" IS_PI4)

      if(IS_PI4 GREATER 0)
        string(APPEND CPACK_DEBIAN_PACKAGE_ARCHITECTURE "_pi4")
      endif(IS_PI4 GREATER 0)

    endif(IS_ARMHF AND NOT PACKAGE_NAME_ONLY_ARCH)

  endif(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

  if(CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

    git_commit_tag(GIT_TAG)

    set(build_type "")

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

      # empty since 'Release' is considered the default
      set(build_type "")

    else ()

      # prepend a '_' this makes the rest of the name generation easier
      # build_type will always be the last property in the string
      # so no problems should be created by this
      set(build_type "_${CMAKE_BUILD_TYPE}")

    endif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

    # we do not want '_Debug' as a string but '_debug'
    string(TOLOWER "${build_type}" build_type)

    set(distribution "")
    if (TCAM_DISTRIBUTION_DESCRIPTION)

      set(distribution "_${TCAM_DISTRIBUTION_DESCRIPTION}")


    endif (TCAM_DISTRIBUTION_DESCRIPTION)

    if (GIT_TAG)

      set(${return_value}
        "${name}_${version}_${GIT_TAG}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}${build_type}"
        PARENT_SCOPE)

    else ()

      git_commit_hash(GIT_COMMIT_HASH)

      if (GIT_COMMIT_HASH)

        git_commit_count(GIT_COMMIT_COUNT)
        git_branch(GIT_BRANCH)

        if ("${GIT_BRANCH}" STREQUAL "master")
          set(${return_value}
            "${name}_${version}.${GIT_COMMIT_COUNT}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}${distribution}${build_type}" PARENT_SCOPE)
        else ()
          set(${return_value}
            "${name}_${version}.${GIT_COMMIT_COUNT}~${GIT_BRANCH}_${GIT_COMMIT_HASH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}${distribution}${build_type}"
            PARENT_SCOPE)

        endif ("${GIT_BRANCH}" STREQUAL "master")

      else () # this will be the case when users download the github zip and compiles on deb systems

        set(${return_value}
          "${name}_${version}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}${distribution}${build_type}" PARENT_SCOPE)

      endif(GIT_COMMIT_HASH)

    endif(GIT_TAG)

  else(CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

    set(${return_value}
      "${name}_${version}_${CMAKE_SYSTEM_NAME}${build_type}" PARENT_SCOPE)

  endif(CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

endfunction ()
