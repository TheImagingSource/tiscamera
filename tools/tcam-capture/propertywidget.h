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

#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include "tcamslider.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QWidget>
#include <gst/gst.h>
#include <string>
#include <tcam-property-1.0.h>

class Property
{
public:
    virtual ~Property() = default;

    virtual void update() = 0;

    QString get_name() const;
    std::string get_category() const;

    virtual void set_locked(bool) = 0;

    virtual void set_in_backend() = 0;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept = 0;
};

class EnumWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    EnumWidget(TcamPropertyEnumeration* prop, QWidget* parent = nullptr);

    virtual void update() final;

    virtual void set_locked(bool) final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void drop_down_changed(const QString& entry);

signals:

    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();

    QBoxLayout* p_layout;
    QLabel* p_name;

    QComboBox* p_combobox;

    TcamPropertyEnumeration* p_prop;
};

class IntWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    IntWidget(TcamPropertyInteger* prop, QWidget* parent = nullptr);


    virtual void update() final;

    virtual void set_locked(bool) final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void slider_changed(int);
    void spinbox_changed(int new_value);

signals:

    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();
    void write_value(int64_t new_value);

    QBoxLayout* p_layout = nullptr;
    QLabel* p_name = nullptr;
    TcamSlider* p_slider = nullptr;
    QSpinBox* p_box = nullptr;

    TcamPropertyInteger* p_prop = nullptr;
};


class DoubleWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    DoubleWidget(TcamPropertyFloat* prop, QWidget* parent = nullptr);


    virtual void update() final;

    virtual void set_locked(bool) final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void slider_changed(double);
    void spinbox_changed(double);

signals:

    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();
    void write_value(double new_value);

    QBoxLayout* p_layout = nullptr;
    QLabel* p_name = nullptr;

    TcamSlider* p_slider = nullptr;
    QDoubleSpinBox* p_box = nullptr;

    TcamPropertyFloat* p_prop = nullptr;
};


class BoolWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    BoolWidget(TcamPropertyBoolean* prop, QWidget* parent = nullptr);

    virtual void update() final;

    virtual void set_locked(bool) final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void checkbox_changed(bool);

signals:

    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();

    QBoxLayout* p_layout;
    QLabel* p_name;

    QCheckBox* p_checkbox;

    TcamPropertyBoolean* p_prop;
};


class ButtonWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    ButtonWidget(TcamPropertyCommand* prop, QWidget* parent = nullptr);


    virtual void update() final;

    virtual void set_locked(bool) final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void got_clicked();

signals:

    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();

    QBoxLayout* p_layout;
    QLabel* p_name;

    QPushButton* p_button;

    TcamPropertyCommand* p_prop;
};


#endif // PROPERTYWIDGET_H
