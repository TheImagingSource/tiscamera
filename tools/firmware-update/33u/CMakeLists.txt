
# Copyright 2017 The Imaging Source Europe GmbH
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

# find_package(libusb-1.0 REQUIRED)
# include_directories(${LIBUSB_1_INCLUDE_DIRS})

include_directories("..")

set(files
  Archive.cpp
  Camera.cpp
  Crc32.cpp
  DevicePortFlash.cpp
  Firmware.cpp
  FirmwarePackage.cpp
  Flash.cpp
  ForwardProgress.cpp
  GenCPCommand.cpp
  GenCPCommand.h
  GenCPDevice.cpp
  LibUsbShowDevice.cpp
  LibUsbShowEnumerator.cpp
  ReportProgress.cpp
  Flash.cpp
  GenCPFacade.cpp
  lib/7z/7zAlloc.c
  lib/7z/7zCrc.c
  lib/7z/7zFile.c
  lib/7z/Bra86.c
  lib/7z/CpuArch.c
  lib/7z/LzmaDec.c
  lib/7z/7zArcIn.c
  lib/7z/7zCrcOpt.c
  lib/7z/7zStream.c
  lib/7z/Bra.c
  lib/7z/Delta.c
  lib/7z/Ppmd7.c
  lib/7z/7zBuf.c
  lib/7z/7zDec.c
  lib/7z/Bcj2.c
  lib/7z/BraIA64.c
  lib/7z/Lzma2Dec.c
  lib/7z/Ppmd7Dec.c
  )

add_library(33update OBJECT ${files})

# target_link_libraries(33updqate ${LIBUSB_1_LIBRARIES})
