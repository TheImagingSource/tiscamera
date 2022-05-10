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

#include "propertydialog.h"

#include "propertywidget.h"
#include "ui_propertydialog.h"

#include <QFormLayout>
#include <tcam-property-1.0.h>

#include <QAction>
#include <QKeyEvent>

PropertyTree::PropertyTree(const std::vector<Property*>& properties, QWidget* parent)
    : QWidget(parent), m_properties(properties)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    setup_ui();
}

void PropertyTree::setup_ui()
{
    QFormLayout* l = new QFormLayout();
    setLayout(l);

    for (auto* ptr : m_properties)
    {
        auto widget = dynamic_cast<QWidget*>(ptr);

        QLabel* name_label = new QLabel(ptr->get_name());

        static const int min_height = 50;

        name_label->setMinimumHeight(min_height);
        widget->setMinimumHeight(min_height);

        l->addRow(name_label, widget);
    }
}

PropertyDialog::PropertyDialog(TcamCollection& collection, QWidget* parent)
    : QDialog(parent), ui(new Ui::PropertyDialog)
{
    setMinimumSize(720, 640);

    ui->setupUi(this);
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags | Qt::Tool);
    setModal(false);

    p_work_thread = new QThread(this);
    p_work_thread->start();
    p_worker = new PropertyWorker();

    p_worker->moveToThread(p_work_thread);

    initialize_dialog(collection);

    p_worker->add_properties(m_properties);
    update();
}

PropertyDialog::~PropertyDialog()
{
    delete ui;

    if (p_work_thread->isRunning())
    {
        p_work_thread->quit();
    }
    p_work_thread->wait();
}


void PropertyDialog::notify_device_lost(const QString& info)
{
    emit device_lost(info);
}


void PropertyDialog::update_tab(int index)
{
    auto name = ui->tabWidget->tabText(index);
    emit this->update_category(name);
}


void PropertyDialog::refresh()
{
    int index = ui->tabWidget->currentIndex();
    update_tab(index);
}


void PropertyDialog::keyPressEvent(QKeyEvent* event)
{
    // this is to ensure the property dialog behaves
    // like a normal dialog
    // esc == quit
    if (event->key() == Qt::Key_Escape)
    {
        this->close();
    }
}


void PropertyDialog::initialize_dialog(TcamCollection& collection)
{
    this->ui->tabWidget->clear();

    auto names = collection.get_names();

    std::vector<std::string> known_categories;

    std::vector<Property*>& prop_list = m_properties;

    for (const std::string& name : names)
    {
        TcamPropertyBase* prop = collection.get_property(name);

        if (!prop)
        {
            qWarning("Unable to retrieve property: %s", name.c_str());
            continue;
        }

        std::string category;
        if (auto cat = tcam_property_base_get_category(prop); cat)
        {
            category = cat;
        }

        if (category.empty())
        {
            qWarning("%s has empty category!", name.c_str());
        }

        auto is_known_category =
            std::any_of(known_categories.begin(),
                        known_categories.end(),
                        [&category](const std::string& categ) { return categ == category; });

        if (!is_known_category)
            known_categories.push_back(category);

        switch (tcam_property_base_get_property_type(prop))
        {
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                auto ptr = new DoubleWidget(TCAM_PROPERTY_FLOAT(prop));
                connect(ptr, &DoubleWidget::value_changed, p_worker, &PropertyWorker::write_property, Qt::QueuedConnection);
                connect(ptr, &DoubleWidget::update_category, p_worker, &PropertyWorker::update_category, Qt::QueuedConnection);
                connect(ptr, &DoubleWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                auto ptr = new IntWidget(TCAM_PROPERTY_INTEGER(prop));
                connect(ptr, &IntWidget::value_changed, p_worker, &PropertyWorker::write_property, Qt::QueuedConnection);
                connect(ptr, &IntWidget::update_category, p_worker, &PropertyWorker::update_category, Qt::QueuedConnection);
                connect(ptr, &IntWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                auto ptr = new EnumWidget(TCAM_PROPERTY_ENUMERATION(prop));
                connect(ptr, &EnumWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &EnumWidget::update_category, p_worker, &PropertyWorker::update_category);
                connect(ptr, &EnumWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                auto ptr = new BoolWidget(TCAM_PROPERTY_BOOLEAN(prop));
                connect(ptr, &BoolWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &BoolWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                auto ptr = new ButtonWidget(TCAM_PROPERTY_COMMAND(prop));
                connect(
                    ptr, &ButtonWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &ButtonWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING:
            {
                auto ptr = new StringWidget(TCAM_PROPERTY_STRING(prop));
                connect(ptr, &StringWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
        }
    }

    static const std::string best_order[] =
    {
        "Exposure",
        "Color",
        "Auto ROI",
        "Image",
        "Lens",
        "Color Correction",
        "Trigger",
        "DigitalIO",
        "Partial Scan",
        "WDR",
        "Multi-Frame Output Mode",
        "Startup",
    };

    std::vector<std::string> added_tabs;
    added_tabs.reserve(known_categories.size());

    for (const auto& o : best_order)
    {
        std::vector<Property*> props;

        for (auto& p : prop_list)
        {
            if (p->get_category() == o)
            {
                props.push_back(p);
            }
        }
        if (props.empty())
        {
            continue;
        }

        PropertyTree* tab_tree = new PropertyTree(props);

        ui->tabWidget->addTab(tab_tree, o.c_str());
        added_tabs.push_back(o);
    }

    // sort rest of the tabs alphabetically
    // to get more predictable behavior
    std::sort(known_categories.begin(), known_categories.end());

    for (const auto& cat : known_categories)
    {
        if (std::any_of(added_tabs.begin(),
                        added_tabs.end(),
                        [&cat](const std::string& o){return cat == o;}))
        {
            continue;
        }

        std::vector<Property*> props;

        for (auto& p : prop_list)
        {
            if (p->get_category() == cat)
            {
                props.push_back(p);
            }
        }

        if (props.empty())
        {
            continue;
        }

        PropertyTree* tab_tree = new PropertyTree(props);

        ui->tabWidget->addTab(tab_tree, cat.c_str());
    }

    // tab change causes update for affected properties
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PropertyDialog::update_tab);
    // updates shall be done in another context
    connect(this, &PropertyDialog::update_category, p_worker, &PropertyWorker::update_category);

    // connect buttons
    connect(ui->button_update, &QPushButton::clicked, this, &PropertyDialog::refresh);
    connect(ui->button_close, &QPushButton::clicked, this, &PropertyDialog::close);


}
