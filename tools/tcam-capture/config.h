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
#include <QSettings>

#include "definitions.h"

struct TcamCaptureConfig
{
    FormatHandling format_selection_type = FormatHandling::Auto;
    ConversionElement conversion_element = ConversionElement::Auto;
    // expectations
    // output element name: sink
    // if a capsfilter element named device-caps exists it will have the configured caps set
    // all tcam-property elements are named: tcam0, tcam1, etc
    // tcam0 is always source
    QString pipeline = "tcambin name=tcam0 ! video/x-raw,format=BGRx ! queue ! videoconvert "
        "! fpsdisplaysink video-sink=xvimagesink sync=false name=sink text-overlay=false signal-fps-measurements=true";


    void save()
    {
        QSettings s;

        s.setValue("format_selection_type", (int)format_selection_type);
        s.setValue("conversion_element", (int)conversion_element);

        // do not safe pipeline
        // if in doubt we always have the default value
    }

    void load()
    {
        QSettings s;

        format_selection_type = (FormatHandling)s.value("format_selection_type", (int)format_selection_type).toInt();
        conversion_element = (ConversionElement)s.value("conversion_element", (int)conversion_element).toInt();

        auto tmp = s.value("pipeline", "").toString();
        if (!tmp.isEmpty())
        {
            pipeline = tmp;

            pipeline.replace(QString("{display-sink}"),
                             QString("fpsdisplaysink video-sink=xvimagesink sync=false name=sink text-overlay=false signal-fps-measurements=true"));
            qInfo("Pipeline string: %s", pipeline.toStdString().c_str());
        }

    }
};
