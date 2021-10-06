/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include <QString>

#include "definitions.h"

struct TcamCaptureConfig
{
    FormatHandling format_selection_type = FormatHandling::Auto;
    bool use_dutils = true;
    // expectations
    // output element name: sink
    // if a capsfilter element named device-caps exists it will have the configured caps set
    // all tcamprop elements are named: tcam0, tcam1, etc
    // tcam0 is always source
    QString pipeline = "tcambin name=tcam0 ! video/x-raw,format=BGRx ! videoconvert ! queue ! fpsdisplaysink video-sink=xvimagesink sync=false name=sink "
        "text-overlay=false signal-fps-measurements=true";
    //QString pipeline = "";


};
