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

#include "filename_generator.h"
#include "gst/gstcaps.h"
#include "gst/gstobject.h"
#include "gst/gststructure.h"

#include <QDateTime>

namespace tcam::tools::capture
{

QString caps_to_file_str(const GstCaps& caps)
{
    const GstCaps* c = &caps;
    if (!gst_caps_is_fixed(c))
    {
        return "";
    }

    QString ret;

    GstStructure* struc = gst_caps_get_structure(&caps, 0);

    auto fmt = gst_structure_get_string(struc, "format");
    int width;
    int height;
    gst_structure_get_int(struc, "width", &width);
    gst_structure_get_int(struc, "height", &height);

    int num;
    int denom;
    gst_structure_get_fraction(struc, "framerate", &num, &denom);

    //gst_u
    ret = fmt;
    ret += "_" + QString::number(width) + "x" + QString::number(height) ;

    if (gst_structure_has_field(struc, "binning"))
    {
        auto binning = gst_structure_get_string(struc, "binning");
        ret += "_b";
        ret += binning;
    }
    if (gst_structure_has_field(struc, "skipping"))
    {
        auto skipping = gst_structure_get_string(struc, "skipping");
        ret += "_b";
        ret += skipping;
    }

    ret += "_" + QString::number(num) + "_" + QString::number(denom);

    //gst_object_unref(struc);

    return ret;
}

FileNameGenerator::FileNameGenerator(const QString& serial, const QString& caps)
    : serial_(serial), caps_(caps)
{
}


QString FileNameGenerator::get_help_text()
{
    return "The following fields are supported:\n"
        "\t{serial} - Serial number of the used device\n"
        "\t{caps} - Used GstCaps\n"
        "\t{timestamp} - ISO datetime with ms YYYYmmDDTHHMMSS_zzz\n"
        "\t{extension} - File format";

}

QString FileNameGenerator::preview(const QString &format)
{
    QString ret = format;

    ret.replace(QString("{serial}"), QString("19931234"));
    auto r = ret.replace(QString("{caps}"), QString("rggb_1920x1080@30_1"));
    qInfo("%s", r.toStdString().c_str());
    ret.replace(QString("{timestamp}"), QString("19990229T185534_456"));

    ret.replace(QString("{extension}"), file_extension_);

    return ret;
}


QString FileNameGenerator::generate()
{
    QString ret = base_pattern_;

    ret.replace(QString("{serial}"), serial_);
    auto r = ret.replace(QString("{caps}"), caps_);

    QDateTime current = QDateTime::currentDateTime();
    //current.toString("YYYYMMDDThhmmss_zzz");

    ret.replace(QString("{timestamp}"), current.toString("yyyyMMddThhmmss_zzz"));

    ret.replace(QString("{extension}"), file_extension_);

    return ret;
}

}
