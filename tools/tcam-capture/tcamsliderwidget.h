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

#ifndef TCAMSLIDERWIDGET_H
#define TCAMSLIDERWIDGET_H

#include "tcamslider.h"

#include <QLayout>
#include <QSpinBox>
#include <QWidget>

class TcamSliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TcamSliderWidget(QWidget* parent = nullptr);

    bool is_logarithmic() const;
    void setRange(int min, int max);
    void setSingleStep(int step);
    void setValue(int value);
    void set_locked(bool lock);
signals:

    void value_changed(int);

private slots:

    void slider_changed(int new_value);

    void spinbox_changed(int new_value);

private:
    void setup_ui();

    QLayout* p_layout;
    TcamSlider* p_slider;
    QSpinBox* p_box;
};

#endif // TCAMSLIDERWIDGET_H
