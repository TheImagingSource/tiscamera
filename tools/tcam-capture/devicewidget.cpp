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

#include "devicewidget.h"

DeviceWidget::DeviceWidget(const Device& dev, QListWidget* parent)
    : QListWidgetItem(parent), _dev(dev)
{
    setIcon(QIcon(QString(icon_path.c_str())));
    setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    setText((_dev.model() + "\n" + _dev.serial() + "\n" + _dev.type()).c_str());
    setSizeHint({160,140});
}

bool DeviceWidget::operator==(const Device& dev) const
{
    if (_dev == dev)
    {
        return true;
    }
    return false;
}

Device DeviceWidget::get_device() const
{
    return _dev;
}


QSize DeviceWidget::minimumSizeHint() const
{
    QSize s = sizeHint();

    return s;
}
