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

#include "ui_propertydialog.h"
#include <QFormLayout>

#include <tcam-property-1.0.h>

// overload the function call operator
bool sort_props(const std::string& lhs, const std::string& rhs)
{
    // dereference the pointers to compare their targets
    // using the Product class's operator<(...) function
    return lhs < rhs;
}

PropertyTree::PropertyTree(QString name,
                           const std::map<std::string, Property*>& properties,
                           QWidget* parent)
    : QWidget(parent), m_properties(properties), m_name(name)
{
    setup_ui();
}

void PropertyTree::setup_ui()
{
    QFormLayout* l = new QFormLayout();
    setLayout(l);

    std::vector<std::string> names;
    for (const auto& p : m_properties) { names.push_back(p.first); }

    std::sort(names.begin(), names.end(), sort_props);

    for (const auto& name : names)
    {
        auto ptr = m_properties.at(name);
        auto widget = dynamic_cast<QWidget*>(ptr);
        l->addRow(ptr->get_name(), widget);
    }
}

PropertyDialog::PropertyDialog(TcamCollection& collection, QWidget* parent)
    : QDialog(parent), ui(new Ui::PropertyDialog), m_collection(collection)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags | Qt::Tool);
    setModal(false);

    p_work_thread = new QThread(this);
    p_work_thread->start();
    p_worker = new PropertyWorker(m_collection, {});

    p_worker->moveToThread(p_work_thread);

    initialize_dialog();

    std::vector<Property*> props;
    for (const auto& p : m_properties) { props.push_back(p.second); }

    p_worker->add_properties(props);
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


void PropertyDialog::initialize_dialog()
{
    this->ui->tabWidget->clear();

    auto names = m_collection.get_names();

    std::vector<std::string> known_categories;

    for (const std::string& name : names)
    {
        TcamPropertyBase* prop = m_collection.get_property(name);

        if (!prop)
        {
            qWarning("Unable to retrieve property: %s", name.c_str());
            continue;
        }

        std::string category = tcam_property_base_get_category(prop);

        if (category.empty())
        {
            qWarning("%s has empty category!", name.c_str());
        }

        auto iter = std::find_if(
            known_categories.begin(), known_categories.end(), [&category](const std::string& categ) {
                if (categ == category)
                    return true;
                return false;
            });

        if (iter == known_categories.end())
            known_categories.push_back(category);

        switch(tcam_property_base_get_property_type(prop))
        {
            case TCAM_PROPERTY_TYPE_FLOAT:
            {
                auto ptr = new DoubleWidget(TCAM_PROPERTY_FLOAT(prop));
                connect(ptr, &DoubleWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &DoubleWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                m_properties[name] = dynamic_cast<Property*>(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                auto ptr = new IntWidget(TCAM_PROPERTY_INTEGER(prop));
                connect(ptr, &IntWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &IntWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                m_properties[name] = dynamic_cast<Property*>(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_ENUMERATION:
            {
                auto ptr = new EnumWidget(TCAM_PROPERTY_ENUMERATION(prop));
                connect(ptr, &EnumWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &EnumWidget::device_lost, this, &PropertyDialog::notify_device_lost);

                m_properties[name] = dynamic_cast<Property*>(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                auto ptr = new BoolWidget(TCAM_PROPERTY_BOOLEAN(prop));
                connect(ptr, &BoolWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &BoolWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                m_properties[name] = dynamic_cast<Property*>(ptr);
                break;
            }
            case TCAM_PROPERTY_TYPE_COMMAND:
            {
                auto ptr = new ButtonWidget(TCAM_PROPERTY_COMMAND(prop));
                connect(ptr, &ButtonWidget::value_changed, p_worker, &PropertyWorker::write_property);
                connect(ptr, &ButtonWidget::device_lost, this, &PropertyDialog::notify_device_lost);
                m_properties[name] = dynamic_cast<Property*>(ptr);
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
        std::map<std::string, Property*> props;

        for (auto& p : m_properties)
        {
            if (p.second->get_category() == cat)
            {
                props.emplace(p);
            }
        }
        PropertyTree* tab_tree = new PropertyTree(cat.c_str(), props);

        ui->tabWidget->addTab(tab_tree, cat.c_str());
    }
}
