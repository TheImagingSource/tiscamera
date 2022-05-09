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

#include <QTimer>
#include <cassert>
#include <gst/gst.h>

using namespace tcam::tools::capture;

QString Property::get_name() const
{
    auto prop = get_property_base();
    if (!prop)
    {
        return {};
    }

    auto disp_name = tcam_property_base_get_display_name(prop);
    if (disp_name && strcmp(disp_name, "") != 0)
    {
        return disp_name;
    }
    else
    {
        return tcam_property_base_get_name(prop);
    }
}


std::string Property::get_category() const
{
    auto prop = get_property_base();
    if (!prop || tcam_property_base_get_category(prop) == nullptr)
    {
        return {};
    }
    return tcam_property_base_get_category(prop);
}


template<class TWidget> static void emit_error_stuff(TWidget* widget, GError* err)
{
    qWarning("Error while handling property '%s'. Message: %s",
             qUtf8Printable(widget->get_name()),
             err->message);
    if (err->code == TCAM_ERROR_DEVICE_LOST)
    {
        qWarning("Device lost");
        emit widget->device_lost(err->message);
    }
    g_error_free(err);
    err = nullptr;
}


#define HANDLE_ERROR(err, action)    \
    if (err)                         \
    {                                \
        emit_error_stuff(this, err); \
        action;                      \
    }


static QString generate_tooltip(TcamPropertyBase* p_prop)
{
    // the font tagging is used to make the
    // tool tip text 'feature rich'
    // this enables auto wrapping
    QString toolTip = QString("<font>");
    toolTip += tcam_property_base_get_description(p_prop);
    toolTip += "<p>API ID: ";
    toolTip += tcam_property_base_get_name(p_prop);
    toolTip += "</p></font>";
    return toolTip;
}

EnumWidget::EnumWidget(TcamPropertyEnumeration* prop, QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void EnumWidget::update()
{
    GError* err = nullptr;

    bool available = tcam_property_base_is_available(TCAM_PROPERTY_BASE(p_prop), &err);
    HANDLE_ERROR(err, return );

    if (!available)
    {
        p_combobox->blockSignals(true);
        p_combobox->setEnabled(false);
        p_combobox->setCurrentIndex(-1); // this shows the placeholder text
        p_combobox->blockSignals(false);
    }
    else if (is_readonly_)
    {
        QString value = tcam_property_enumeration_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        p_combobox->blockSignals(true);
        p_combobox->setEnabled(false);

        for (int index = 0; index < p_combobox->count(); index++)
        {
            if (p_combobox->itemText(index) == value)
            {
                p_combobox->setCurrentIndex(index);
                break;
            }
        }

        p_combobox->blockSignals(false);
    }
    else
    {
        QString value = tcam_property_enumeration_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);
        HANDLE_ERROR(err, return );

        p_combobox->blockSignals(true);
        p_combobox->setEnabled(!lock);

        // setCurrentText caused problems on some developer systems
        // by selecting the entry via index this is circumvented
        for (int index = 0; index < p_combobox->count(); index++)
        {
            if (p_combobox->itemText(index) == value)
            {
                p_combobox->setCurrentIndex(index);
                break;
            }
        }

        if (value == "Once")
        {
            QTimer::singleShot(500,
                               [this]() { emit this->update_category(get_category().c_str()); });
        }

        p_combobox->blockSignals(false);
    }
}

void EnumWidget::drop_down_changed(const QString& entry)
{
    // This property is in a state where
    // other properties in the same category are still
    // actively being changed and the property
    // itself will also soon have a different value
    // update the category until the property has a different value
    if (entry == "Once")
    {
        QTimer::singleShot(500, [this]() { emit this->update_category(get_category().c_str()); });
    }
    emit value_changed(this);
}

void EnumWidget::set_in_backend()
{
    auto s = p_combobox->currentText().toStdString();

    GError* err = nullptr;
    tcam_property_enumeration_set_value(p_prop, s.c_str(), &err);

    HANDLE_ERROR(err, return );
}

void EnumWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    TcamPropertyAccess access = tcam_property_base_get_access(TCAM_PROPERTY_BASE(p_prop));
    if (access == TCAM_PROPERTY_ACCESS_RO)
        is_readonly_ = true;

    GError* err = nullptr;
    GSList* entries = tcam_property_enumeration_get_enum_entries(p_prop, &err);
    HANDLE_ERROR(err, return );

    p_combobox = new QComboBox();

    p_combobox->setEditable(false);

    for (auto e = entries; e != nullptr; e = e->next) { p_combobox->addItem((const char*)e->data); }

    g_slist_free_full(entries, g_free);

    update();

    connect(p_combobox, &QComboBox::currentTextChanged, this, &EnumWidget::drop_down_changed);

    p_layout->addWidget(p_combobox);

    this->setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}

IntWidget::IntWidget(TcamPropertyInteger* prop, QWidget* parent) : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void IntWidget::update()
{
    GError* err = nullptr;

    bool available = tcam_property_base_is_available(TCAM_PROPERTY_BASE(p_prop), &err);
    HANDLE_ERROR(err, return );

    if (!available)
    {
        if (p_slider)
            p_slider->setDisabled(true);
        if (p_box)
            p_box->setDisabled(true);
    }
    else if (is_readonly_)
    {
        gint64 value = tcam_property_integer_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        assert(p_slider == nullptr);
        assert(p_box != nullptr);

        p_box->blockSignals(true);
        p_box->setDisabled(false);

        p_box->setRange(value - 1, value);
        p_box->setSingleStep(1);

        p_box->setValue(value);
        p_box->setReadOnly(true);

        p_box->blockSignals(false);
    }
    else
    {
        // !read-only && available

        gint64 min = INT_MIN;
        gint64 max = INT_MAX;
        gint64 step = 1;
        tcam_property_integer_get_range(p_prop, &min, &max, &step, &err);
        HANDLE_ERROR(err, return );

        gint64 value = tcam_property_integer_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        // fix behavior of QSlider/QBox to only show 0, when the range is extremely large
        if (min <= INT_MIN && max >= INT_MAX)
        {
            min = value - 1;
            max = value;
        }
        else if (std::abs(max - min) == 0) // Fix Qt behavior when range is 0
        {
            min = value - 1;
            max = value;
        }

        bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);
        HANDLE_ERROR(err, return );

        if (p_slider && !p_slider->isSliderDown())
        {
            const QSignalBlocker blocker(p_slider);

            p_slider->setRange(min, max, step);
            p_slider->setValue(value);
            p_slider->setDisabled(lock);
        }
        if (p_box)
        {
            const QSignalBlocker blocker(p_box);

            p_box->setDisabled(false);
            p_box->setRange(min, max);
            p_box->setSingleStep(step);

            p_box->setValue(value);
            p_box->setReadOnly(lock);
        }
    }
}


void IntWidget::slider_changed(int new_value)
{
    const QSignalBlocker blocker(p_box);

    p_box->setValue(new_value);

    if (!is_readonly_)
    {
        emit value_changed(this);
        repaint();
    }
}


void IntWidget::spinbox_changed(qlonglong new_value)
{
    if (p_slider)
    {
        const QSignalBlocker blocker(p_slider);

        p_slider->setValue(new_value);
    }

    if (!is_readonly_)
    {
        emit value_changed(this);
        repaint();
    }
}


void IntWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    TcamPropertyAccess access = tcam_property_base_get_access(TCAM_PROPERTY_BASE(p_prop));
    if (access == TCAM_PROPERTY_ACCESS_RO)
    {
        is_readonly_ = true;
    }

    TcamPropertyIntRepresentation representation = tcam_property_integer_get_representation(p_prop);

    // TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER
    // only box for display
    // TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER
    // box with different base
    // rest box and slider but with different scale
    if (!is_readonly_)
    {
        TcamSliderScale scale;
        if (representation == TCAM_PROPERTY_INTREPRESENTATION_LINEAR)
        {
            scale = TcamSliderScale::Linear;
        }
        else if (representation == TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC)
        {
            scale = TcamSliderScale::Logarithmic;
        }

        p_slider = new TcamSlider(scale);
    }

    p_box = new TcamSpinBox();
    p_box->setCorrectionMode(QAbstractSpinBox::CorrectionMode::CorrectToNearestValue);

    if (representation == TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER)
    {
        p_box->setDisplayIntegerBase(16);
    }

    if (auto unit_ptr = tcam_property_integer_get_unit(p_prop); unit_ptr)
    {
        p_box->setSuffix(QString::asprintf(" %s", unit_ptr));
    }

    update();

    if (p_slider)
    {
        connect(p_slider, &TcamSlider::valueChanged, this, &IntWidget::slider_changed, Qt::QueuedConnection);
        p_layout->addWidget(p_slider);
    }
    if (p_box)
    {
        connect(p_box,
                QOverload<qlonglong>::of(&TcamSpinBox::valueChanged),
                this,
                &IntWidget::spinbox_changed);
        p_layout->addWidget(p_box);
    }
    this->setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}

void IntWidget::write_value(int64_t new_value)
{
    GError* err = nullptr;

    tcam_property_integer_set_value(p_prop, new_value, &err);

    HANDLE_ERROR(err, return )
}

void IntWidget::set_in_backend()
{
    GError* err = nullptr;

    tcam_property_integer_set_value(p_prop, p_box->value(), &err);

    HANDLE_ERROR(err, return )
}

DoubleWidget::DoubleWidget(TcamPropertyFloat* prop, QWidget* parent) : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void DoubleWidget::update()
{
    GError* err = nullptr;

    bool is_available = tcam_property_base_is_available(TCAM_PROPERTY_BASE(p_prop), &err);
    HANDLE_ERROR(err, return );

    if (!is_available)
    {
        if (p_slider)
            p_slider->setDisabled(true);
        if (p_box)
            p_box->setDisabled(true);
    }
    else if (is_readonly_)
    {
        gdouble value = tcam_property_float_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        assert(p_slider == nullptr);
        assert(p_box != nullptr);

        p_box->blockSignals(true);
        p_box->setDisabled(false);

        p_box->setRange(value - 1, value);
        p_box->setSingleStep(1);

        p_box->setValue(value);
        p_box->setReadOnly(true);

        p_box->blockSignals(false);
    }
    else
    {
        gdouble min = 0;
        gdouble max = 0;
        gdouble step = 1;
        tcam_property_float_get_range(p_prop, &min, &max, &step, &err);
        HANDLE_ERROR(err, return );

        gdouble value = tcam_property_float_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);
        HANDLE_ERROR(err, return );

        if (p_slider && !p_slider->isSliderDown())
        {
            const QSignalBlocker blocker(p_slider);

            p_slider->setRange(min, max, step);
            p_slider->setValue(value);
            p_slider->setDisabled(lock);
        }
        if (p_box)
        {
            const QSignalBlocker blocker(p_box);

            p_box->setDisabled(false);
            p_box->setRange(min, max);
            p_box->setSingleStep(step);

            p_box->setValue(value);
            p_box->setReadOnly(lock);
        }
    }
}

void DoubleWidget::slider_changed(double new_value)
{
    const QSignalBlocker blocker(p_box);

    p_box->setValue(new_value);

    if (!is_readonly_)
    {
        emit value_changed(this);
        repaint();
    }
}

void DoubleWidget::spinbox_changed(double new_value)
{
    if (p_slider)
    {
        const QSignalBlocker blocker(p_slider);

        p_slider->setValue(new_value);
    }

    if (!is_readonly_)
    {
        emit value_changed(this);
        repaint();
    }
}


void DoubleWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    TcamPropertyAccess access = tcam_property_base_get_access(TCAM_PROPERTY_BASE(p_prop));
    if (access == TCAM_PROPERTY_ACCESS_RO)
        is_readonly_ = true;

    auto representation = tcam_property_float_get_representation(p_prop);

    if (!is_readonly_)
    {
        if (representation != TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER)
        {
            if (representation == TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR)
            {
                p_slider = new TcamSlider();
            }
            else if (representation == TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC)
            {
                p_slider = new TcamSlider(TcamSliderScale::Logarithmic);
            }
            else
            {
                throw std::runtime_error("representation not implemented.");
            }
        }
    }

    p_box = new QDoubleSpinBox();
    p_box->setCorrectionMode(QAbstractSpinBox::CorrectionMode::CorrectToNearestValue);

    if (auto unit_ptr = tcam_property_float_get_unit(p_prop); unit_ptr)
    {
        p_box->setSuffix(QString::asprintf(" %s", unit_ptr));
    }

    update();

    if (p_slider)
    {
        connect(p_slider, &TcamSlider::valueChanged, this, &DoubleWidget::slider_changed, Qt::QueuedConnection);
        p_layout->addWidget(p_slider);
    }
    if (p_box)
    {
        connect(p_box,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this,
                &DoubleWidget::spinbox_changed, Qt::QueuedConnection);
        p_layout->addWidget(p_box);
    }
    this->setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}

void DoubleWidget::write_value(double new_value)
{
    GError* err = nullptr;

    tcam_property_float_set_value(p_prop, new_value, &err);

    HANDLE_ERROR(err, return )
}

void DoubleWidget::set_in_backend()
{
    GError* err = nullptr;

    tcam_property_float_set_value(p_prop, p_box->value(), &err);

    HANDLE_ERROR(err, return )
}


BoolWidget::BoolWidget(TcamPropertyBoolean* prop, QWidget* parent) : QWidget(parent), p_prop(prop)
{
    setup_ui();
}


void BoolWidget::update()
{
    GError* err = nullptr;

    bool is_available = tcam_property_base_is_available(TCAM_PROPERTY_BASE(p_prop), &err);
    HANDLE_ERROR(err, return );

    if (!is_available)
    {
        p_checkbox->setEnabled(false);
    }
    else if (is_readonly_)
    {
        bool value = tcam_property_boolean_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        p_checkbox->blockSignals(true);
        p_checkbox->setChecked(value);
        p_checkbox->setEnabled(false);
        p_checkbox->blockSignals(false);
    }
    else
    {
        bool value = tcam_property_boolean_get_value(p_prop, &err);
        HANDLE_ERROR(err, return );

        bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);
        HANDLE_ERROR(err, return );

        p_checkbox->blockSignals(true);
        p_checkbox->setChecked(value);
        p_checkbox->setEnabled(!lock);
        p_checkbox->blockSignals(false);
    }
}

void BoolWidget::checkbox_changed(bool /*new_value*/)
{
    if (!is_readonly_)
    {
        emit value_changed(this);
    }
}

void BoolWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    TcamPropertyAccess access = tcam_property_base_get_access(TCAM_PROPERTY_BASE(p_prop));
    if (access == TCAM_PROPERTY_ACCESS_RO)
        is_readonly_ = true;

    p_checkbox = new QCheckBox();

    update();

    connect(p_checkbox, &QCheckBox::clicked, this, &BoolWidget::checkbox_changed);

    p_layout->addWidget(p_checkbox);

    this->setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}

void BoolWidget::set_in_backend()
{
    GError* err = nullptr;
    tcam_property_boolean_set_value(p_prop, p_checkbox->isChecked(), &err);

    HANDLE_ERROR(err, return )
}

ButtonWidget::ButtonWidget(TcamPropertyCommand* prop, QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void ButtonWidget::update()
{
    GError* err = nullptr;

    bool is_available = tcam_property_base_is_available(TCAM_PROPERTY_BASE(p_prop), &err);
    HANDLE_ERROR(err, return );

    if (!is_available)
    {
        p_button->setEnabled(false);
    }
    else
    {
        bool lock = tcam_property_base_is_locked(TCAM_PROPERTY_BASE(p_prop), &err);

        HANDLE_ERROR(err, return );

        p_button->setEnabled(!lock);
    }
}

void ButtonWidget::got_clicked()
{
    emit value_changed(this);
}

void ButtonWidget::set_in_backend()
{
    GError* err = nullptr;

    tcam_property_command_set_command(p_prop, &err);

    HANDLE_ERROR(err, return );
}

void ButtonWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    p_button = new QPushButton();

    update();

    connect(p_button, &QPushButton::pressed, this, &ButtonWidget::got_clicked);

    p_layout->addWidget(p_button);

    this->setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}


StringWidget::StringWidget(TcamPropertyString* prop, QWidget* parent)
    : QWidget(parent), p_prop(prop)
{
    setup_ui();
}

void StringWidget::update()
{
    GError* err = nullptr;

    auto access = tcam_property_base_get_access(TCAM_PROPERTY_BASE(p_prop));

    static bool issue_ro_warning;

    if (access != TCAM_PROPERTY_ACCESS_RO && !issue_ro_warning)
    {
        qWarning("Property '%s' is not read-only. String values are not writeable from tcam-capture.", get_name().toStdString().c_str());
        issue_ro_warning = true;
    }

    const char* value = tcam_property_string_get_value(p_prop, &err);

    HANDLE_ERROR(err, return)

    if (value)
    {
        p_label->setText(value);
    }
    else
    {
        // this prevents a segfault (yes, really)
        // setting a long text into an empty label causes
        // a layout change, which seems to have a library
        // bug (tested qt 5.15). Adding a longer empty text
        // prevents reformatting
        p_label->setText("                    ");
    }
}

void StringWidget::set_in_backend()
{}

void StringWidget::setup_ui()
{
    auto p_layout = new QHBoxLayout();

    setLayout(p_layout);

    p_label = new QLabel();
    // values should be copyable
    p_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    update();

    p_layout->addWidget(p_label);

    setToolTip(generate_tooltip(TCAM_PROPERTY_BASE(p_prop)));
}
