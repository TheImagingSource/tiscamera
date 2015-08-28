/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_COMPILER_DEFINES_H
#define TCAM_COMPILER_DEFINES_H

#if defined (__GNUC__)

#define VISIBILITY_INTERNAL _Pragma("GCC visibility push (internal)")
#define VISIBILITY_POP      _Pragma("GCC visibility pop")

#else

#define VISIBILITY_INTERNAL
#define VISIBILITY_POP

#endif

#endif /* TCAM_COMPILER_DEFINES_H */
