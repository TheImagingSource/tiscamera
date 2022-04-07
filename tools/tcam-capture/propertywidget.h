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
#include <string>

#include "tcamspinbox.h"
#include <tcam-property-1.0.h>

class Property
{
public:
    virtual ~Property() = default;

    virtual void update() = 0;
    virtual void set_in_backend() = 0;

    QString get_name() const;
    std::string get_category() const;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept = 0;
};

class EnumWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    EnumWidget(TcamPropertyEnumeration* prop, QWidget* parent = nullptr);

    virtual void update() final;

    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void drop_down_changed(const QString& entry);

signals:

    // update_category signal is
    // used for QTimer::singleShot to update Once actions
    void update_category(QString category);
    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();

    QComboBox* p_combobox = nullptr;

    TcamPropertyEnumeration* p_prop = nullptr;
    bool is_readonly_ = false;
};

class IntWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    IntWidget(TcamPropertyInteger* prop, QWidget* parent = nullptr);

    virtual void update() final;
    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

public slots:

    void slider_changed(int);
    void spinbox_changed(qlonglong new_value);

signals:

    void update_category(QString category);
    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();
    void write_value(int64_t new_value);

    TcamSlider* p_slider = nullptr;
    tcam::tools::capture::TcamSpinBox* p_box = nullptr;

    TcamPropertyInteger* p_prop = nullptr;
    bool is_readonly_ = false;
};


class DoubleWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    DoubleWidget(TcamPropertyFloat* prop, QWidget* parent = nullptr);

    virtual void update() final;
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

    void update_category(QString category);
    void value_changed(Property*);
    void device_lost(const QString& info);

private:
    void setup_ui();
    void write_value(double new_value);

    TcamSlider* p_slider = nullptr;
    QDoubleSpinBox* p_box = nullptr;

    TcamPropertyFloat* p_prop = nullptr;

    bool is_readonly_ = false;
};


class BoolWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    BoolWidget(TcamPropertyBoolean* prop, QWidget* parent = nullptr);

    virtual void update() final;
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

    QCheckBox* p_checkbox = nullptr;

    TcamPropertyBoolean* p_prop = nullptr;
    bool is_readonly_ = false;
};


class ButtonWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    ButtonWidget(TcamPropertyCommand* prop, QWidget* parent = nullptr);


    virtual void update() final;
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

    QPushButton* p_button = nullptr;

    TcamPropertyCommand* p_prop = nullptr;
};


class StringWidget : public QWidget, public Property
{
    Q_OBJECT
public:
    explicit StringWidget(TcamPropertyString* prop, QWidget* parent = nullptr);

    virtual void update() final;
    virtual void set_in_backend() final;

protected:
    virtual TcamPropertyBase* get_property_base() const noexcept final
    {
        return TCAM_PROPERTY_BASE(p_prop);
    }

signals:

    void device_lost(const QString& info);

private:
    void setup_ui();

    QLabel* p_label = nullptr;

    TcamPropertyString* p_prop = nullptr;
};

#endif // PROPERTYWIDGET_H
