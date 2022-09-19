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
#include "spdlog/spdlog.h"

#include <gst/gst.h>


struct TcamCaptureConfig
{
    FormatHandling format_selection_type = FormatHandling::Auto;
    ConversionElement conversion_element = ConversionElement::Auto;
    QString video_sink_element = "qwidget5videosink";

    // expectations
    // output element name: sink
    // tee name: capture-tee
    // queue has disabled caching
    // if a capsfilter element named device-caps exists it will have the configured caps set
    // all tcam-property elements are named: tcam0, tcam1, etc
    // tcam0 is always source
    QString pipeline =
        "tcambin name=tcam0 ! video/x-raw,format=BGRx "
        " ! tee name=capture-tee "
        " ! queue leaky=1 max-size-time=0 max-size-bytes=0 max-size-buffers=0"
        " ! videoconvert n-threads=4 "
        " ! fpsdisplaysink video-sink={video-sink-element} sync=false name=sink "
        "text-overlay=false signal-fps-measurements=true";

    ImageSaveType save_image_type = ImageSaveType::BMP;
    QString save_image_location = "/tmp/";
    QString save_image_filename_structure = "tcam-capture-{serial}-{caps}-{timestamp}.{extension}";

    QString save_video_location = "/tmp/";
    QString save_video_filename_structure = "tcam-capture-{serial}-{caps}-{timestamp}.{extension}";
    VideoCodec save_video_type = VideoCodec::H264;

    void save()
    {
        QSettings s;

        s.setValue("format_selection_type", (int)format_selection_type);
        s.setValue("conversion_element", (int)conversion_element);

        s.setValue("save_image_type", (int)save_image_type);
        s.setValue("save_image_location", save_image_location);
        s.setValue("save_image_filename_structure", save_image_filename_structure);

        s.setValue("save_video_type", (int)save_video_type);
        s.setValue("save_video_location", save_video_location);
        s.setValue("save_video_filename_structure", save_video_filename_structure);

        // do not save pipeline
        // if in doubt we always have the default value
    }

    void load()
    {
        QSettings s;

        save_image_type = (ImageSaveType)s.value("save_image_type", (int)save_image_type).toInt();
        save_image_location = s.value("save_image_location", save_image_location).toString();
        save_image_filename_structure = s.value("save_image_filename_structure", save_image_filename_structure).toString();

        save_video_type = (VideoCodec)s.value("save_video_type", (int)save_video_type).toInt();
        save_video_location = s.value("save_video_location", save_video_location).toString();
        save_video_filename_structure = s.value("save_video_filename_structure",
                                                save_video_filename_structure).toString();

        format_selection_type = (FormatHandling)s.value("format_selection_type", (int)format_selection_type).toInt();
        conversion_element = (ConversionElement)s.value("conversion_element", (int)conversion_element).toInt();

        auto tmp = s.value("pipeline", "").toString();
        if (!tmp.isEmpty())
        {
            pipeline = tmp;

            pipeline.replace(QString("{display-sink}"),
                             QString("fpsdisplaysink video-sink={video-sink-element} sync=false name=sink text-overlay=false signal-fps-measurements=true"));
            qInfo("Pipeline string: %s", pipeline.toStdString().c_str());
        }


        auto factory = gst_element_factory_find(video_sink_element.toStdString().c_str());

        if (factory != nullptr)
        {

            gst_object_unref(factory);
            auto tmp_sink_element = s.value("video-sink-element", video_sink_element).toString();
            pipeline.replace(QString("{video-sink-element}"), tmp_sink_element);
        }
        else
        {
            SPDLOG_ERROR("Unable to find Gstreamer element: {}\n"
                         "\t\tUsing fallback. this disabled some functionalities.\n"
                         "\t\tAre display elements installed?   sudo apt install qtgstreamer-plugins-qt5",
                         video_sink_element.toStdString().c_str());
            auto tmp_sink_element = s.value("video-sink-element", "xvimagesink").toString();
            pipeline.replace(QString("{video-sink-element}"), tmp_sink_element);
        }


    }
};
