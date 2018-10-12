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

#
# Convenience functions for accessing git information in cmake
#


# git_is_repository
# verifies that cmake is in a git repository
# useful when people use the 'download as zip' functionality from github, et al
# sets given argument to TRUE if git is used, otherwise it will be set to FALSE
function (git_is_repository is_repo)

  execute_process(
    COMMAND git rev-parse --git-dir 2> /dev/null
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ROOT_DIR
    )

  if (ROOT_DIR)
    set(${is_repo} TRUE PARENT_SCOPE)
  else ()
    set(${is_repo} FALSE PARENT_SCOPE)
  endif (ROOT_DIR)

endfunction (git_is_repository)

# git_commit_hash
# fills the given argument with the hash of the current commit
#
function (git_commit_hash return_value)
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GIT_COMMIT_HASH)
    set(${return_value} "${GIT_COMMIT_HASH}" PARENT_SCOPE)
  endif (GIT_COMMIT_HASH)

endfunction (git_commit_hash)

# git_commit_count
# fills the given argument with the number of commits on this branch
function (git_commit_count return_value)

  execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_COUNT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GIT_COMMIT_COUNT)
    set(${return_value} "${GIT_COMMIT_COUNT}" PARENT_SCOPE)
  endif (GIT_COMMIT_COUNT)

endfunction (git_commit_count)

# git_commit_tag
# fills the given argument with the tag the current commit has
function (git_commit_tag return_value)
  execute_process(
    COMMAND git describe --exact-match HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    ERROR_VARIABLE GIT_TAG_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GIT_TAG)
    set(${return_value} "${GIT_TAG}" PARENT_SCOPE)
  endif (GIT_TAG)

endfunction (git_commit_tag)

# git_branch
# fills the given argument with the name of the current branch
#
function (git_branch return_value)

  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GIT_BRANCH)
    set(${return_value} "${GIT_BRANCH}" PARENT_SCOPE)
  endif (GIT_BRANCH)

endfunction (git_branch)

# defines the following variables in the parent scope
# GIT_COMMIT_HASH
# GIT_COMMIT_COUNT
# GIT_TAG
# GIT_BRANCH
#
# the variables will not defined should error occur or should
# the project not exist within a git repo
#
function (find_git_settings)

  git_commit_hash(hash)
  if (hash)
    set(GIT_COMMIT_HASH "${hash}" PARENT_SCOPE)
  endif (hash)

  git_commit_count(count)
  if (count)
    set(GIT_COMMIT_COUNT "${count}" PARENT_SCOPE)
  endif (count)

  git_commit_tag(tag)
  if (tag)
    set(GIT_TAG "${tag}" PARENT_SCOPE)
  endif (tag)

  git_branch(branch)
  if (branch)
    set(GIT_BRANCH "${branch}" PARENT_SCOPE)
  endif (branch)

endfunction ()
