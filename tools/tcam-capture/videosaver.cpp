/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "videosaver.h"
#include "gst/gstbin.h"
#include "gst/gstelement.h"
#include "gst/gstevent.h"
#include "gst/gstmessage.h"
#include "gst/gstobject.h"
#include "gst/gstparse.h"
#include "gst/gstutils.h"

namespace

{

QString find_codec_pipeline(VideoCodec codec)
{
    // the following parts have to be kept identical between codecs
    // begin with a queue named save-queue
    // end with a filesink named save-sink

    QString start = "queue name=save-queue max-size-time=0 max-size-bytes=0 max-size-buffers=0 "
        "leaky=downstream"
        " ! videoconvert n-threads=4 "
        " ! queue ";
    QString end = " ! queue max-size-time=0 max-size-bytes=0 max-size-buffers=0 "
          " ! filesink async=true sync=false name=save-sink";

    switch (codec)
    {
        case VideoCodec::H264:
        {
            return start
                   + " ! x264enc name=save-enc "
                   + " ! mp4mux "
                   + end;
        }
        // case VideoCodec::H265:
        // {
        //     return "queue name=save-queue max-size-time=0 max-size-bytes=0 max-size-buffers=0 "
        //            "leaky=downstream"
        //            " ! videoconvert n-threads=4 "
        //            " ! queue "
        //            " ! x265enc name=save-enc "
        //            " ! mp4mux ! queue max-size-time=0 max-size-bytes=0 max-size-buffers=0 "
        //            " ! filesink async=true sync=false name=save-sink";
        // }
        case VideoCodec::MJPEG:
        {
            return "queue name=save-queue leaky=1 max-size-time=0 max-size-bytes=0 "
                   "max-size-buffers=0 "
                   " ! videoconvert n-threads=4 "
                   " ! jpegenc name=save-enc "
                   " ! avimux "
                   " ! queue max-size-time=0 max-size-bytes=0 max-size-buffers=0 "
                   " ! filesink async=true name=save-sink";
        }
    }

    return nullptr;
}


} // namespace

tcam::tools::capture::VideoSaver::VideoSaver(GstPipeline* pipeline,
                                             const QString& filename,
                                             VideoCodec codec)
    : pipeline_(GST_ELEMENT(pipeline)), target_file_(filename), codec_(codec)
{}


void tcam::tools::capture::VideoSaver::start_saving()
{
    auto pipeline_str = find_codec_pipeline(codec_);

    save_pipeline_ = gst_parse_launch(pipeline_str.toStdString().c_str(), nullptr);

    g_object_set(G_OBJECT(save_pipeline_), "message-forward", TRUE, nullptr);

    tee_ = gst_bin_get_by_name(GST_BIN(pipeline_), "capture-tee");
    queue_ = gst_bin_get_by_name(GST_BIN(save_pipeline_), "save-queue");

    auto file_sink = gst_bin_get_by_name(GST_BIN(save_pipeline_), "save-sink");

    g_object_set(file_sink, "location", target_file_.toStdString().c_str(), nullptr);

    gst_bin_add(GST_BIN(pipeline_), save_pipeline_);

    if (!gst_element_link(tee_, queue_))
    {
        qWarning("ERROR linking elements");
        if (gst_pad_link(tee_pad_, queue_pad_) != GST_PAD_LINK_OK)
        {
            qWarning("Unable to link pipeline correctly");
        }
    }

    // set playing

    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    qDebug("Started video saving");
}


void tcam::tools::capture::VideoSaver::stop_saving()
{
    gst_element_set_state(save_pipeline_, GST_STATE_PAUSED);


    gst_element_unlink(tee_, queue_);

    gst_element_set_state(pipeline_, GST_STATE_PLAYING);

    gst_element_set_state(save_pipeline_, GST_STATE_PLAYING);
    auto enc = gst_bin_get_by_name(GST_BIN(save_pipeline_), "save-enc");

    gst_element_send_event(enc, gst_event_new_eos());

    gst_object_unref(enc);
    qDebug("Stopped video saving. Waiting for EOS.");
}

void tcam::tools::capture::VideoSaver::destroy_pipeline()
{
    //qDebug("destroy");
    if (save_pipeline_)
    {
        gst_element_set_state(save_pipeline_, GST_STATE_NULL);

        gst_bin_remove(GST_BIN(pipeline_), save_pipeline_);

        gst_object_unref(save_pipeline_);
        save_pipeline_ = nullptr;
    }
}


GstObject* tcam::tools::capture::VideoSaver::gst_pointer() const
{
    if (!save_pipeline_)
    {
        return nullptr;
    }

    GstElement* sink = gst_bin_get_by_name(GST_BIN(save_pipeline_), "save-sink");

    return GST_OBJECT(sink);
}
