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

#include "tcamsliderwidget.h"

TcamSliderWidget::TcamSliderWidget(QWidget* parent) : QWidget(parent)
{
    setup_ui();
}


void TcamSliderWidget::setup_ui()
{
    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    p_slider = new TcamSlider();
    p_layout->addWidget(p_slider);

    connect(p_slider, &TcamSlider::valueChanged, this, &TcamSliderWidget::slider_changed);

    p_box = new QSpinBox();
    p_layout->addWidget(p_box);

    connect(p_box,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &TcamSliderWidget::spinbox_changed);
}

bool TcamSliderWidget::is_logarithmic() const
{
    return false;
}


void TcamSliderWidget::setRange(int min, int max)
{
    p_slider->setRange(min, max);
    p_box->setRange(min, max);
}


void TcamSliderWidget::setSingleStep(int step)
{
    p_slider->setSingleStep(step);
    p_box->setSingleStep(step);
}


void TcamSliderWidget::setValue(int value)
{
    p_slider->blockSignals(true);
    p_slider->setValue(value);
    p_slider->blockSignals(false);

    p_box->blockSignals(true);
    p_box->setValue(value);
    p_box->blockSignals(false);
}

void TcamSliderWidget::set_locked(bool lock)
{
    p_slider->setEnabled(!lock);
    p_box->setEnabled(!lock);
}

void TcamSliderWidget::slider_changed(int new_value)
{
    p_box->blockSignals(true);
    p_box->setValue(new_value);
    p_box->blockSignals(false);
    emit value_changed(new_value);
}


void TcamSliderWidget::spinbox_changed(int new_value)
{
    p_slider->blockSignals(true);
    p_slider->setValue(new_value);
    p_slider->blockSignals(false);
    emit value_changed(new_value);
}
