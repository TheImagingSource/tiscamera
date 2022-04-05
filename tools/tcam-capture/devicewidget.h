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

#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include "device.h"

#include <QListWidgetItem>
#include <QWidget>

class DeviceWidget : public QListWidgetItem
{
public:
    DeviceWidget(const Device& dev, QListWidget* parent = nullptr);

    bool operator==(const Device& dev) const;

    Device get_device() const;

    virtual QSize minimumSizeHint() const;

private:
    Device _dev;
    std::string icon_path = ":/images/camera.png";
};

#endif // DEVICEWIDGET_H
