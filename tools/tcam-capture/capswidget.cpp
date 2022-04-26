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

void CapsWidget::set_caps(GstCaps* caps, GstElement& element)
{
    if (!caps || !gst_caps_is_fixed(caps))
    {
        qErrnoWarning("Caps are not fixed");
        return;
    }

    auto c = Caps(caps, element);

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    auto format = c.get_formats().at(0);

    this->combo_format_changed(format.c_str());

    if (p_combo_binning && gst_structure_has_field(struc, "binning"))
    {
        const char* binning = gst_structure_get_string(struc, "binning");
        p_combo_binning->setCurrentText(binning);
    }

    if (p_combo_skipping && gst_structure_has_field(struc, "skipping"))
    {
        const char* skipping = gst_structure_get_string(struc, "skipping");
        p_combo_skipping->setCurrentText(skipping);
    }

    int width, height;
    gst_structure_get_int(struc, "width", &width);
    gst_structure_get_int(struc, "height", &height);

    QString value = QString::number(width) + "x" + QString::number(height);

    for (int i = 0; i < p_combo_resolution->count(); i++)
    {
        if (p_combo_resolution->itemText(i) == value)
        {
            p_combo_resolution->setCurrentIndex(i);
            break;
        }
    }
    combo_res_changed(value);

    int num, denom;
    gst_structure_get_fraction(struc, "framerate", &num, &denom);

    value = QString::number(num/denom);
    for (int i = 0; i < p_combo_framerate->count(); i++)
    {
        if (p_combo_framerate->itemText(i) == value)
        {
            p_combo_framerate->setCurrentIndex(i);
            break;
        }
    }
    combo_fps_changed(value);
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
    {
        p_combo_framerate->setCurrentText(current_entry);
    }
    else
    {
        // either select 30 FPS or the highest
        auto f = std::find(res.begin(), res.end(), 30.0 );
        if (f == res.end())
        {
            f = std ::max_element(res.begin(), res.end());
        }
        if (f != res.end())
        {
            p_combo_framerate->setCurrentText(QString::number(*f));
        }
    }

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
    tcam_image_size ret = {};

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
