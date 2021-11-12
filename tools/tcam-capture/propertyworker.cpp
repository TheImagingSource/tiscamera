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
