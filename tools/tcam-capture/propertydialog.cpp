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

PropertyTree::PropertyTree(QString name,
                           const std::vector<Property*>& properties,
                           QWidget* parent)
    : QWidget(parent), m_properties(properties), m_name(name)
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
    setMinimumSize(640, 480);

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
}

PropertyDialog::~PropertyDialog()
{
    delete ui;

    p_worker->stop();
    if (p_work_thread->isRunning())
    {
        p_work_thread->quit();
    }
    p_work_thread->wait();
}


void PropertyDialog::notify_device_lost(const QString& info)
{
    p_worker->stop();
    emit device_lost(info);
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

        auto is_known_category = std::any_of(known_categories.begin(),
                                 known_categories.end(),
                                 [&category](const std::string& categ)
                                 {
                                     return categ == category;
                                 });

        if (!is_known_category)
            known_categories.push_back(category);

        switch (tcam_property_base_get_property_type(prop))
        {
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                auto ptr = new DoubleWidget(TCAM_PROPERTY_FLOAT(prop));
                connect(
                    ptr, &DoubleWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &DoubleWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                auto ptr = new IntWidget(TCAM_PROPERTY_INTEGER(prop));
                connect(ptr, &IntWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &IntWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                prop_list.push_back(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                auto ptr = new EnumWidget(TCAM_PROPERTY_ENUMERATION(prop));
                connect(ptr, &EnumWidget::value_changed, p_worker, &PropertyWorker::write_property);
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
            default:
            {
                qWarning("Property Type not implemented");
                break;
            }
        }
    }

    for (const auto& cat : known_categories)
    {
        std::vector<Property*> props;

        for (auto& p : prop_list)
        {
            if (p->get_category() == cat)
            {
                props.push_back(p);
            }
        }
        PropertyTree* tab_tree = new PropertyTree(cat.c_str(), props);

        ui->tabWidget->addTab(tab_tree, cat.c_str());
    }
}
