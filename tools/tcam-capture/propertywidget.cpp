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

#include "propertywidget.h"

#include <gst/gst.h>


// TODO: add device lost notification to macro

#define HANDLE_ERROR(err, action)                                       \
    if(err)                                                             \
    {                                                                   \
        qWarning("Error while handling property: %s", err->message);    \
        if (err->code == TCAM_ERROR_DEVICE_LOST)                        \
        {                                                               \
            qWarning("Device lost");                                    \
            emit device_lost(err->message);                             \
        }                                                               \
        g_error_free(err);                                              \
        action;                                                         \
    }



EnumWidget::EnumWidget(TcamPropertyEnumeration* prop,
                       QWidget* parent)
    : QWidget(parent), p_prop(prop)
{

    setup_ui();
}

void EnumWidget::update()
{
    GError* err = nullptr;
    QString value = tcam_property_enumeration_get_value(p_prop, &err);

    HANDLE_ERROR(err, return)

    p_combobox->blockSignals(true);
    p_combobox->setCurrentText(value);
    p_combobox->blockSignals(false);

    bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

    HANDLE_ERROR(err, return)

    set_locked(lock);
}


QString EnumWidget::get_name() const
{
    auto disp_name = tcam_property_base_get_display_name(TCAM_PROPERTY_BASE(p_prop));

    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(TCAM_PROPERTY_BASE(p_prop));
    }
}


std::string EnumWidget::get_category() const
{
    return tcam_property_base_get_category(TCAM_PROPERTY_BASE(p_prop));
}


void EnumWidget::set_locked(bool lock)
{
    p_combobox->setEnabled(!lock);
}


void EnumWidget::drop_down_changed(const QString& entry)
{
    auto s = entry.toStdString();

    GError* err = nullptr;
    tcam_property_enumeration_set_value(p_prop, s.c_str(), &err);

    HANDLE_ERROR(err, return)

    emit value_changed(get_name().toStdString().c_str(), entry);
}


void EnumWidget::setup_ui()
{
    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    GError* err = nullptr;

    QString value = tcam_property_enumeration_get_value(p_prop, &err);

    tcam_property_enumeration_get_default(p_prop, &err);



    GSList* entries = tcam_property_enumeration_get_enum_entries(p_prop, &err);

    // if (show_property_flags() && m_flags.is_external())
    // {
    //     auto si = style()->standardIcon(QStyle::SP_ComputerIcon);
    //     int iconSize = 32;
    //     QLabel* iconLabel = new QLabel;
    //     iconLabel->setToolTip("Property is emulated on the computer.");
    //     iconLabel->setPixmap(si.pixmap(iconSize, iconSize));
    //     iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //     iconLabel->setMargin(2);
    //     p_layout->addWidget(iconLabel);
    //     //layout->addWidget(titleLabel, 0, 1);
    // }

    p_combobox = new QComboBox();

    p_combobox->setEditable(false);

    for (auto e = entries; e != nullptr; e = e->next)
    {
        p_combobox->addItem((const char*)e->data);
    }

    g_slist_free_full(entries, g_free);

    p_combobox->setCurrentText(value);
    connect(p_combobox, &QComboBox::currentTextChanged, this, &EnumWidget::drop_down_changed);

    p_layout->addWidget(p_combobox);

    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enabled auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(TCAM_PROPERTY_BASE(p_prop));
    toolTip += QString("</font>");
    this->setToolTip(toolTip);
}


IntWidget::IntWidget(TcamPropertyInteger* prop,
                     QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}


void IntWidget::update()
{
    GError* err = nullptr;
    gint64 value = tcam_property_integer_get_value(p_prop, &err);

    HANDLE_ERROR(err, return);

    if (p_slider)
    {
        p_slider->blockSignals(true);
        p_slider->setValue(value);
        p_slider->blockSignals(false);
    }

    p_box->blockSignals(true);
    p_box->setValue(value);
    p_box->blockSignals(false);

    bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

    HANDLE_ERROR(err, return);

    set_locked(lock);
}


QString IntWidget::get_name() const
{
    auto disp_name = tcam_property_base_get_display_name(TCAM_PROPERTY_BASE(p_prop));

    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(TCAM_PROPERTY_BASE(p_prop));
    }
}


std::string IntWidget::get_category() const
{
    return tcam_property_base_get_category(TCAM_PROPERTY_BASE(p_prop));
}


void IntWidget::set_locked(bool lock)
{
    if (p_slider)
    {
        p_slider->setDisabled(lock);
    }
    p_box->setDisabled(lock);
}

void IntWidget::slider_changed(int new_value)
{
    p_box->blockSignals(true);
    p_box->setValue(new_value);
    p_box->blockSignals(false);

    write_value(new_value);

    emit value_changed(get_name().toStdString().c_str(), new_value);
}


void IntWidget::spinbox_changed(int new_value)
{
    if (p_slider)
    {
        p_slider->blockSignals(true);
        p_slider->setValue(new_value);
        p_slider->blockSignals(false);
    }
    write_value(new_value);

    emit value_changed(get_name().toStdString().c_str(), new_value);
}


void IntWidget::setup_ui()
{
    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    gint64 min;
    gint64 max;
    gint64 step;
    GError* err = nullptr;
    tcam_property_integer_get_range(p_prop, &min, &max, &step, &err);
    gint64 value = tcam_property_integer_get_value(p_prop, &err);

    TcamPropertyIntRepresentation representation = tcam_property_integer_get_representation(p_prop);

    if (representation != TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER)
    {
        if (representation == TCAM_PROPERTY_INTREPRESENTATION_LINEAR)
        {
            p_slider = new TcamSlider();
        }
        else if (representation == TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC)
        {
            p_slider = new TcamSlider(TcamSliderScale::Logarithmic);
        }
        else
        {
            throw std::runtime_error("representation not implemented.");
        }

        p_slider->setRange(min, max);
        p_slider->setSingleStep(step);
        p_slider->setValue(value);

        connect(p_slider, &TcamSlider::valueChanged, this, &IntWidget::slider_changed);
    }

    p_box = new QSpinBox();

    p_box->setRange(min, max);
    p_box->setSingleStep(step);
    p_box->setValue(value);

    connect(p_box, QOverload<int>::of(&QSpinBox::valueChanged), this, &IntWidget::spinbox_changed);

    p_layout->addWidget(p_slider);
    p_layout->addWidget(p_box);

    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enabled auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(TCAM_PROPERTY_BASE(p_prop));
    toolTip += QString("</font>");
    this->setToolTip(toolTip);
}


void IntWidget::write_value(int64_t new_value)
{
    GError* err = nullptr;

    tcam_property_integer_set_value(p_prop, new_value, &err);

    HANDLE_ERROR(err, return)
}




DoubleWidget::DoubleWidget(TcamPropertyFloat* prop,
                           QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}


void DoubleWidget::update()
{
    GError* err = nullptr;
    gdouble value = tcam_property_float_get_value(p_prop, &err);

    HANDLE_ERROR(err, return);

    if (p_slider)
    {
        p_slider->blockSignals(true);
        p_slider->setValue(value);
        p_slider->blockSignals(false);
    }

    p_box->blockSignals(true);
    p_box->setValue(value);
    p_box->blockSignals(false);

    bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

    HANDLE_ERROR(err, return);

    set_locked(lock);
}


QString DoubleWidget::get_name() const
{
    auto disp_name = tcam_property_base_get_display_name(TCAM_PROPERTY_BASE(p_prop));

    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(TCAM_PROPERTY_BASE(p_prop));
    }
}


std::string DoubleWidget::get_category() const
{
    return tcam_property_base_get_category(TCAM_PROPERTY_BASE(p_prop));
}


void DoubleWidget::set_locked(bool lock)
{
    if (p_slider)
    {
        p_slider->setDisabled(lock);
    }

    p_box->setDisabled(lock);
}

void DoubleWidget::slider_changed(double new_value)
{
    p_box->blockSignals(true);
    p_box->setValue(new_value);
    p_box->blockSignals(false);

    write_value(new_value);

    emit value_changed(get_name().toStdString().c_str(), new_value);
}

void DoubleWidget::spinbox_changed(double new_value)
{

    if (p_slider)
    {
        p_slider->blockSignals(true);
        p_slider->setValue(new_value);
        p_slider->blockSignals(false);
    }

    write_value(new_value);

    emit value_changed(get_name().toStdString().c_str(), new_value);
}


void DoubleWidget::setup_ui()
{
    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    m_representation = tcam_property_float_get_representation(p_prop);

    gdouble min;
    gdouble max;
    gdouble step;
    GError* err = nullptr;
    tcam_property_float_get_range(p_prop, &min, &max, &step, &err);

    gdouble value = tcam_property_float_get_value(p_prop, &err);

    if (m_representation != TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER)
    {
        if (m_representation == TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR)
        {
            p_slider = new TcamSlider();
        }
        else if (m_representation == TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC)
        {
            p_slider = new TcamSlider(TcamSliderScale::Logarithmic);
        }
        else
        {
            throw std::runtime_error("representation not implemented.");
        }

        p_slider->setRange(min, max);
        p_slider->setSingleStep(step);
        p_slider->setValue(value);

        connect(p_slider, &TcamSlider::valueChanged, this, &DoubleWidget::slider_changed);
    }

    QString unit = " ";
    unit += tcam_property_float_get_unit(p_prop);

    p_box = new QDoubleSpinBox();

    p_box->setSuffix(unit);
    p_box->setRange(min, max);
    p_box->setSingleStep(step);
    p_box->setValue(value);

    connect(p_box,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DoubleWidget::spinbox_changed);

    if (p_slider)
    {
        p_layout->addWidget(p_slider);
    }
    p_layout->addWidget(p_box);

    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enabled auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(TCAM_PROPERTY_BASE(p_prop));
    toolTip += QString("</font>");
    this->setToolTip(toolTip);
}


void DoubleWidget::write_value(double new_value)
{
    GError* err = nullptr;

    tcam_property_float_set_value(p_prop, new_value, &err);

    HANDLE_ERROR(err, return)
}


BoolWidget::BoolWidget(TcamPropertyBoolean* prop,
                       QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}


void BoolWidget::update()
{
    GError* err = nullptr;
    bool value = tcam_property_boolean_get_value(p_prop, &err);

    HANDLE_ERROR(err, return);

    p_checkbox->blockSignals(true);
    p_checkbox->toggled(value);
    p_checkbox->blockSignals(false);

    bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

    HANDLE_ERROR(err, return);

    set_locked(lock);
}



QString BoolWidget::get_name() const
{
    auto disp_name = tcam_property_base_get_display_name(TCAM_PROPERTY_BASE(p_prop));

    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(TCAM_PROPERTY_BASE(p_prop));
    }
}


std::string BoolWidget::get_category() const
{
    return tcam_property_base_get_category(TCAM_PROPERTY_BASE(p_prop));
}


void BoolWidget::set_locked(bool lock)
{
    p_checkbox->setEnabled(!lock);
}

void BoolWidget::checkbox_changed(bool new_value)
{
    GError* err = nullptr;
    tcam_property_boolean_set_value(p_prop, new_value, &err);

    HANDLE_ERROR(err, return)

    emit value_changed(get_name().toStdString().c_str(), new_value);
}

void BoolWidget::setup_ui()
{
    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    p_checkbox = new QCheckBox();

    GError* err = nullptr;

    bool value = tcam_property_boolean_get_value(p_prop, &err);

    if (value)
    {
        p_checkbox->toggle();
    }
    connect(p_checkbox, &QCheckBox::clicked, this, &BoolWidget::checkbox_changed);

    p_layout->addWidget(p_checkbox);

    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enabled auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(TCAM_PROPERTY_BASE(p_prop));
    toolTip += QString("</font>");
    this->setToolTip(toolTip);
}


ButtonWidget::ButtonWidget(TcamPropertyCommand* prop,
                           QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void ButtonWidget::update()
{
    GError* err = nullptr;
    bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

    HANDLE_ERROR(err, return);

    set_locked(lock);
}


QString ButtonWidget::get_name() const
{
    auto disp_name = tcam_property_base_get_display_name(TCAM_PROPERTY_BASE(p_prop));

    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(TCAM_PROPERTY_BASE(p_prop));
    }
}


std::string ButtonWidget::get_category() const
{
    return tcam_property_base_get_category(TCAM_PROPERTY_BASE(p_prop));
}


void ButtonWidget::set_locked(bool lock)
{
    p_button->setEnabled(!lock);
}


void ButtonWidget::got_clicked()
{
    GError* err = nullptr;

    tcam_property_command_set_command(p_prop, &err);

    HANDLE_ERROR(err, return)

    emit value_changed(get_name().toStdString().c_str());
}


void ButtonWidget::setup_ui()
{

    p_layout = new QHBoxLayout();

    setLayout(p_layout);

    p_button = new QPushButton();

    connect(p_button, &QPushButton::pressed, this, &ButtonWidget::got_clicked);

    p_layout->addWidget(p_button);

    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enabled auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(TCAM_PROPERTY_BASE(p_prop));
    toolTip += QString("</font>");
    this->setToolTip(toolTip);
}
