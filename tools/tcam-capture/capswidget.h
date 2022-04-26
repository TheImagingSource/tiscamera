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

#ifndef CAPSWIDGET_H
#define CAPSWIDGET_H

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>
#include "caps.h"


#ifndef tcam_image_size

struct tcam_image_size
{
    uint32_t width;
    uint32_t height;

    bool operator<(const struct tcam_image_size& other) const
    {
        if (height <= other.height && width <= other.width)
        {
            return true;
        }
        return false;
    }

    bool operator==(const struct tcam_image_size& other) const
    {
        if (height == other.height && width == other.width)
        {
            return true;
        }
        return false;
    }

    bool operator!=(const struct tcam_image_size& other) const
    {
        return !operator==(other);
    }
};

#endif


class CapsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CapsWidget(const Caps& caps, QWidget* parent = nullptr);

    GstCaps* get_caps() const;
    void set_caps(GstCaps*, GstElement& element);

public slots:
    void combo_fps_changed(const QString& new_value);
    void combo_res_changed(const QString& new_value);
    void combo_skipping_changed(const QString& new_value);
    void combo_binning_changed(const QString& new_value);
    void combo_format_changed(const QString& new_value);

signals:

private:
    void setup_ui();
    void fill_combo_binning(const QString& format);
    void fill_combo_skipping(const QString& format);
    void fill_combo_resolution(const QString& format, const scaling &scale = {});
    void fill_combo_framerate(const QString& format, const scaling& scale, int width, int height);

    scaling get_scaling() const;
    tcam_image_size get_resolution() const;

    Caps m_caps;

    QFormLayout* p_layout = nullptr;

    QComboBox* p_combo_format = nullptr;
    QComboBox* p_combo_binning = nullptr;
    QComboBox* p_combo_skipping = nullptr;

    QSlider* p_slider_width = nullptr;
    QSpinBox* p_box_width = nullptr;
    QSlider* p_slider_height = nullptr;
    QSpinBox* p_box_height = nullptr;
    QSlider* p_slider_framerate = nullptr;
    QSpinBox* p_box_fps = nullptr;

    QComboBox* p_combo_resolution = nullptr;
    QComboBox* p_combo_height = nullptr;
    QComboBox* p_combo_framerate = nullptr;

};

#endif // CAPSWIDGET_H
