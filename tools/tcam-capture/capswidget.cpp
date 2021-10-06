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

#include "capswidget.h"

#include "tcamgstbase.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>

namespace  {

std::pair<int, int> split_qstring(const QString& s)
{
    std::pair<int, int> ret;
    auto tmp = s.split("x");
    ret.first = tmp.at(0).toInt();
    ret.second = tmp.at(1).toInt();

    return ret;
}


std::vector<tcam_image_size> get_standard_resolutions(const tcam_image_size& min,
                                                      const tcam_image_size& max)
{
    static const tcam_image_size resolutions[] = {
        { 128, 96 },    { 320, 240 },   { 360, 280 },   { 544, 480 },   { 640, 480 },
        { 352, 288 },   { 576, 480 },   { 720, 480 },   { 960, 720 },   { 1280, 720 },
        { 1440, 1080 }, { 1920, 1080 }, { 1920, 1200 }, { 2048, 1152 }, { 2048, 1536 },
        { 2560, 1440 }, { 3840, 2160 }, { 4096, 3072 }, { 7680, 4320 }, { 7680, 4800 },
    };

    std::vector<struct tcam_image_size> ret;
    ret.reserve(std::size(resolutions));
    for (const auto& r : resolutions)
    {
        if ((min < r) && (r < max))
        {
            ret.push_back(r);
        }
    }

    return ret;
}


std::vector<double> create_steps_for_range(double min, double max)
{
    std::vector<double> vec;

    if (max <= min)
        return vec;

    vec.push_back(min);

    // we do not want every framerate to have unnecessary decimals
    // e.g. 1.345678 instead of 1.00000
    double current_step = (int)min;

    // 0.0 is not a valid framerate
    if (current_step < 1.0)
        current_step = 1.0;

    while (current_step < max)
    {

        if (current_step < 20.0)
        {
            current_step += 1;
        }
        else if (current_step < 100.0)
        {
            current_step += 10.0;
        }
        else if (current_step < 1000.0)
        {
            current_step += 50.0;
        }
        else
        {
            current_step += 100.0;
        }
        if (current_step < max)
        {
            vec.push_back(current_step);
        }
    }

    if (vec.back() != max)
    {
        vec.push_back(max);
    }
    return vec;
}

}

CapsWidget::CapsWidget(const Caps& caps, QWidget* parent)
    : QWidget(parent), m_caps(caps)
{
    setup_ui();
}

GstCaps* CapsWidget::get_caps() const
{
    auto entries = p_combo_resolution->currentText().split("x");
    int width = 0;
    int height = 0;

    if (entries.size() == 2)
    {
        width = entries.at(0).toInt();
        height = entries.at(1).toInt();
    }

    auto scale = get_scaling();

    double framerate = p_combo_framerate->currentText().toDouble();
    std::string name = m_caps.get_gst_name(p_combo_format->currentText().toStdString());

    GstCaps* caps = gst_caps_from_string(name.c_str());

    int num, denom;

    gst_util_double_to_fraction (framerate,
                                 &num,
                                 &denom);

    gst_caps_set_simple(caps,
                        "width",
                        G_TYPE_INT,
                        width,
                        "height",
                        G_TYPE_INT,
                        height,
                        "framerate",
                        GST_TYPE_FRACTION,
                        num,denom,
                        "binning",
                        G_TYPE_STRING,
                        scale.binning_str().c_str(),
                        "skipping",
                        G_TYPE_STRING,
                        scale.skipping_str().c_str(),
                        nullptr);

    return caps;
}

void CapsWidget::setup_ui()
{
    p_layout = new QFormLayout();

    setLayout(p_layout);

    QLabel* label_format = new QLabel();
    label_format->setText("Format:");
    p_combo_format = new QComboBox();
    auto formats = m_caps.get_formats();

    p_layout->addRow(label_format, p_combo_format);

    for (const auto& f : formats)
    {
        p_combo_format->addItem(QString(f.c_str()));
    }

    if (m_caps.has_binning())
    {
        QLabel* label_bin = new QLabel();
        label_bin->setText("Binning");
        p_combo_binning= new QComboBox();
        p_layout->addRow(label_bin, p_combo_binning);

        connect(p_combo_binning,
                &QComboBox::currentTextChanged,
                this,
                &CapsWidget::combo_binning_changed);
    }

    if (m_caps.has_skipping())
    {
        QLabel* label_skip = new QLabel();
        label_skip->setText("Skipping");
        p_layout->addWidget(label_skip);
        p_combo_skipping = new QComboBox();
        p_layout->addRow(label_skip, p_combo_skipping);

        connect(p_combo_skipping,
                &QComboBox::currentTextChanged,
                this,
                &CapsWidget::combo_skipping_changed);
    }

    QLabel* label_resolution = new QLabel();
    label_resolution->setText("Resolution:");
    p_layout->addWidget(label_resolution);

    p_combo_resolution = new QComboBox();
    p_layout->addRow(label_resolution, p_combo_resolution);

    connect(p_combo_resolution,
            &QComboBox::currentTextChanged,
            this,
            &CapsWidget::combo_res_changed);

    QLabel* label_fps = new QLabel();
    label_fps->setText("FPS:");

    p_combo_framerate = new QComboBox();
    p_layout->addRow(label_fps, p_combo_framerate);

    connect(p_combo_framerate,
            &QComboBox::currentTextChanged,
            this,
            &CapsWidget::combo_fps_changed);

    fill_combo_binning(p_combo_format->currentText());
    fill_combo_skipping(p_combo_format->currentText());

    fill_combo_resolution(p_combo_format->currentText(), get_scaling());

    auto res = get_resolution();

    fill_combo_framerate(p_combo_format->currentText(), get_scaling(), res.width, res.height);
}

void CapsWidget::set_caps(GstCaps* caps)
{
    if (!caps || !gst_caps_is_fixed(caps))
    {
        qErrnoWarning("Caps are not fixed");
        return;
    }

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    if (gst_structure_has_field(struc, "binning"))
    {
        const char* binning = gst_structure_get_string(struc, "binning");

        p_combo_binning->setCurrentText(binning);
    }

    if (gst_structure_has_field(struc, "skipping"))
    {
        const char* skipping = gst_structure_get_string(struc, "skipping");
        p_combo_skipping->setCurrentText(skipping);
    }

    int width, height;

    gst_structure_get_int(struc, "width", &width);
    gst_structure_get_int(struc, "height", &height);

    int num, denom;

    gst_structure_get_fraction(struc, "framerate", &num, &denom);

}


void CapsWidget::fill_combo_binning(const QString& format)
{
    if (!p_combo_binning)
    {
        return;
    }

    auto bin = m_caps.get_binning(format.toStdString());

    QString current_entry;
    if (p_combo_binning->count() > 0)
    {
        current_entry = p_combo_binning->currentText();
    }
    p_combo_binning->blockSignals(true);
    p_combo_binning->clear();

    for (const auto& b : bin)
    {
        p_combo_binning->addItem(b.c_str());
    }

    if (!current_entry.isEmpty())
    {
        p_combo_binning->setCurrentText(current_entry);
    }
    p_combo_binning->blockSignals(false);
}


void CapsWidget::fill_combo_skipping(const QString& format)
{
    if (!p_combo_skipping)
    {
        return;
    }

    auto bin = m_caps.get_skipping(format.toStdString());

    QString current_entry;
    if (p_combo_skipping->count() > 0)
    {
        current_entry = p_combo_skipping->currentText();
    }
    p_combo_skipping->blockSignals(true);
    p_combo_skipping->clear();

    for (const auto& b : bin)
    {
        p_combo_skipping->addItem(b.c_str());
    }

    if (!current_entry.isEmpty())
    {
        p_combo_skipping->setCurrentText(current_entry);
    }
    p_combo_skipping->blockSignals(false);
}


void CapsWidget::fill_combo_resolution(const QString& format, const scaling& scale)
{
    auto res = m_caps.get_resolutions(format.toStdString(), scale);

    QString current_entry;
    if (p_combo_resolution->count() > 0)
    {
        current_entry = p_combo_resolution->currentText();
    }
    p_combo_resolution->blockSignals(true);

    p_combo_resolution->clear();

    for (const auto& r : res)
    {
        p_combo_resolution->addItem(QString::number(r.first) + "x" + QString::number(r.second));
    }

    if (!current_entry.isEmpty())
    {
        p_combo_resolution->setCurrentText(current_entry);
    }
    p_combo_resolution->blockSignals(false);
}

void CapsWidget::fill_combo_framerate(const QString& format, const scaling& scale, int width, int height)
{
    auto res = m_caps.get_framerates(format.toStdString(), scale, width, height);

    QString current_entry;
    if (p_combo_framerate->count() > 0)
    {
        current_entry = p_combo_framerate->currentText();
    }
    p_combo_framerate->blockSignals(true);

    p_combo_framerate->clear();

    for (const auto& r : res)
    {
        p_combo_framerate->addItem(QString::number(r));
    }

    if (!current_entry.isEmpty())
    p_combo_framerate->setCurrentText(current_entry);

    p_combo_framerate->blockSignals(false);
}

void CapsWidget::combo_format_changed(const QString& new_value)
{
    fill_combo_binning(new_value);
    fill_combo_skipping(new_value);
    fill_combo_resolution(new_value, get_scaling());

    auto res = get_resolution();
    fill_combo_framerate(new_value, get_scaling(), res.width, res.height);
}

void CapsWidget::combo_binning_changed(const QString& /*new_value*/)
{
    fill_combo_resolution(p_combo_format->currentText(), get_scaling());

    auto res = get_resolution();
    fill_combo_framerate(p_combo_format->currentText(), get_scaling(), res.width, res.height);
}

void CapsWidget::combo_skipping_changed(const QString& /*new_value*/)
{
    fill_combo_resolution(p_combo_format->currentText(), get_scaling());

    auto res = get_resolution();
    fill_combo_framerate(p_combo_format->currentText(), get_scaling(), res.width, res.height);
}

void CapsWidget::combo_res_changed(const QString& new_value)
{
    auto entries = new_value.split("x");

    fill_combo_framerate(p_combo_format->currentText(),
                         get_scaling(),
                         entries.at(0).toInt(), entries.at(1).toInt());
}

void CapsWidget::combo_fps_changed(const QString& /* new_value*/)
{}


scaling CapsWidget::get_scaling() const
{
    scaling ret = {};

    if (p_combo_binning)
    {
        auto entry = p_combo_binning->currentText();
        auto h_v = entry.split("x");

        if (h_v.size() == 2)
        {
            ret.binning_h = h_v.at(0).toInt();
            ret.binning_v = h_v.at(1).toInt();
        }
    }

    if (p_combo_skipping)
    {
        auto entry = p_combo_skipping->currentText();
        auto h_v = entry.split("x");

        if (h_v.size() == 2)
        {
            ret.skipping_h = h_v.at(0).toInt();
            ret.skipping_v = h_v.at(1).toInt();
        }
    }

    return ret;
}


tcam_image_size CapsWidget::get_resolution() const
{
    tcam_image_size ret;

if (!p_combo_format)
{
    return ret;
}
    auto entry = p_combo_resolution->currentText();

    if (entry.isEmpty())
    {
        return {};

    }

    auto w_h = entry.split("x");

    ret.width = w_h.at(0).toInt();
    ret.height = w_h.at(1).toInt();

    return ret;
}
