# Copyright @ Members of the EMI Collaboration, 2010.
# See www.eu-emi.eu for details on the copyright holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific
#
# This module detects if gsoap is installed and determines where the
# include files and libraries are.
#
# This code sets the following variables:
#
#  LIBUUID_FOUND               True if libuuid got found
#  LIBUUID_INCLUDE_DIRS        Location of libuuid headers
#  LIBUUID_LIBRARIES           List of libraries to use libuuid

# -----------------------------------------------------
# LIBUUID Libraries
# -----------------------------------------------------
#find_library(LIBUUID_LIBRARIES
#    NAMES uuid
#    DOC "The main libuuid library"
#)

# -----------------------------------------------------
# LIBUUID Include Directories
# -----------------------------------------------------
#find_path(LIBUUID_INCLUDE_DIR
#    NAMES uuid/uuid.h
#    DOC "The libuuid include directory"
#)

#SET(LIBUUID_FOUND 0)
#IF(LIBUUID_INCLUDE_DIR)
#  IF(LIBUUID_LIBRARIES)
#    SET(LIBUUID_FOUND 1)
#    MESSAGE(STATUS "Found libuuid")
#  ENDIF(LIBUUID_LIBRARIES)
#ENDIF(LIBUUID_INCLUDE_DIR)


FIND_PATH(LIBUUID_INCLUDE_DIRS uuid/uuid.h)
FIND_LIBRARY(LIBUUID_LIBRARIES uuid)

IF (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)
  SET(LIBUUID_FOUND 1)
  IF (NOT LibUuid_FIND_QUIETLY)
	MESSAGE(STATUS "Found libuuid: ${LIBUUID_LIBRARIES}")
  ENDIF ( NOT LibUuid_FIND_QUIETLY )
ELSE (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)
  IF (LibUuid_FIND_REQUIRED)
	MESSAGE(SEND_ERROR "Could NOT find libuuid")
  ELSE (LibUuid_FIND_REQUIRED)
	IF (NOT LIBUUID_FIND_QUIETLY)
	  MESSAGE(STATUS "Could NOT find libuuid")
	ENDIF (NOT LIBUUID_FIND_QUIETLY)
  ENDIF (LibUuid_FIND_REQUIRED)
ENDIF (LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS)

MARK_AS_ADVANCED(LIBUUID_LIBRARIES LIBUUID_INCLUDE_DIRS)
