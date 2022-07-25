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

#pragma once

#include "config.h"

#include <gst/gst.h>

namespace tcam::tools::capture
{

QString caps_to_file_str(const GstCaps&);

class FileNameGenerator
{
public:

    FileNameGenerator(const QString& serial, const QString& caps);

    static QString get_help_text();

    void set_base_pattern(const QString& pattern)
    {
        base_pattern_ = pattern;
    }

    QString get_base_pattern() const
    {
        return base_pattern_;
    }

    void set_file_extension(const QString& ext)
    {
        file_extension_ = ext;
    };

    QString get_file_extension() const
    {
        return file_extension_;
    };



    QString preview(const QString& format);

    QString generate();

    void reset_sequence();

private:

    QString base_pattern_ = "tcam-capture-image.{extension}";

    QString serial_ = "";
    QString caps_ = "";
    QString file_extension_ = "";

    unsigned int sequence_counter_ = 0;

}; // class FileNameGenerator

} // namespace tcam::tools::capture
