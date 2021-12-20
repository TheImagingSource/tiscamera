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

#include "propertyworker.h"

#include "propertywidget.h"
#include <QThread>

PropertyWorker::PropertyWorker()
{}


void PropertyWorker::add_properties(const std::vector<Property*>& new_props)
{
    m_properties.insert(m_properties.end(), new_props.begin(), new_props.end());
}


void PropertyWorker::write_property(Property* p)
{
    p->set_in_backend();

    const auto cat = p->get_category();
    const auto name = p->get_name();

    for (auto& prop : m_properties)
    {
        if (name != prop->get_name() && prop->get_category() == cat)
        {
            prop->update();
        }
    }
}


void PropertyWorker::update_category(QString category)
{
    for (auto& prop : m_properties)
    {
        if (prop->get_category() == category.toStdString())
        {
            prop->update();
        }
    }
}
