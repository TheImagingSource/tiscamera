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

function (create_package_name return_value name version)

  find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
  if(DPKG_PROGRAM)
    execute_process(
      COMMAND ${DPKG_PROGRAM} --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

    git_commit_tag(GIT_TAG)

    if (GIT_TAG)

      set(${return_value}
        "${name}_${version}_${GIT_TAG}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}"
        PARENT_SCOPE)

    else ()

      git_commit_hash(GIT_COMMIT_HASH)

      if (GIT_COMMIT_HASH)

        git_commit_count(GIT_COMMIT_COUNT)
        git_branch(GIT_BRANCH)

        set(${return_value}
          "${name}_${version}.${GIT_COMMIT_COUNT}~${GIT_BRANCH}_${GIT_COMMIT_HASH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}"
          PARENT_SCOPE)

      else () # this will be the case when users download the github zip and compile on deb systems

        set(${return_value}
          "${name}_${version}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" PARENT_SCOPE)

      endif(GIT_COMMIT_HASH)

    endif(GIT_TAG)

  else(DPKG_PROGRAM)

    set(${return_value}
      "${name}_${version}_${CMAKE_SYSTEM_NAME}" PARENT_SCOPE)

  endif(DPKG_PROGRAM)

endfunction ()
