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

PropertyWorker::PropertyWorker(TcamCollection& collection, const std::vector<Property*>& props)
    : m_properties(props), m_collection(collection)
{

    p_timer = new QTimer(this);

    connect(p_timer, SIGNAL(timeout()), this, SLOT(run()));

    p_timer->start(2000);


    //qInfo("started timer\n");
}

PropertyWorker::~PropertyWorker()
{
    stop();

    p_timer->stop();

    delete p_timer;
}


void PropertyWorker::stop()
{
    m_run = false;
    m_cv.notify_all();
}

void PropertyWorker::add_properties(const std::vector<Property*>& new_props)
{
    m_properties.insert(m_properties.end(), new_props.begin(), new_props.end());
}


void PropertyWorker::set_button(const std::string& name)
{
    GValue set = {};
    g_value_init(&set, G_TYPE_BOOLEAN);
    g_value_set_boolean(&set, true);

    m_collection.set_property(name, &set);

    g_value_unset(&set);
}

void PropertyWorker::set_bool(const std::string& name, bool value)
{
    GValue set = {};
    g_value_init(&set, G_TYPE_BOOLEAN);
    g_value_set_boolean(&set, value);

    m_collection.set_property(name, &set);

    g_value_unset(&set);
}

void PropertyWorker::set_int(const std::string& name, int value)
{
    GValue set = {};
    g_value_init(&set, G_TYPE_INT);
    g_value_set_int(&set, value);

    m_collection.set_property(name, &set);

    g_value_unset(&set);
}

void PropertyWorker::set_double(const std::string& name, double value)
{
    GValue set = {};
    g_value_init(&set, G_TYPE_DOUBLE);
    g_value_set_double(&set, value);

    m_collection.set_property(name, &set);

    g_value_unset(&set);
}

void PropertyWorker::set_enum(const std::string& name, QString value)
{
    GValue set = {};
    g_value_init(&set, G_TYPE_STRING);
    g_value_set_string(&set, value.toStdString().c_str());

    m_collection.set_property(name, &set);

    g_value_unset(&set);
}

void PropertyWorker::set_property(const std::string& name, GValue* value)
{

    GValue set = {};
    g_value_init(&set, G_TYPE_STRING);
    g_value_set_string(&set, "Off");
    //g_value_copy(value, &set);
    //g_print("received set_property %s '%s'\n", name, g_value_get_string(value));

    auto ret = m_collection.set_property(name, value);
    if (ret)
    {
        //g_print("Set %s successfully\n", n.c_str());
    }
    else
    {
        //g_print("Set %s failed\n", n.c_str());
    }

    g_value_unset(&set);
}


void PropertyWorker::run()
{
    if (!m_run)
    {
        return;
    }

    for (auto& p : m_properties)
    {
        p->update();
        if (!m_run)
        {
            break;
        }
    }
}
