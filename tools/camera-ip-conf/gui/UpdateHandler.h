/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

#ifndef UPDATEHANDLER_H
#define UPDATEHANDLER_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "../CameraDiscovery.h"
#include "../Camera.h"

using namespace tis;

class UpdateHandler : public QThread
{
    Q_OBJECT
public:
    explicit UpdateHandler(QObject *parent = 0);

    ~UpdateHandler();

    void run ();

signals:
    void finished ();

    void update (camera_list);

    void error (QString err);

    void deprecated (std::shared_ptr<Camera> camera);

public slots:
    void timeCycle();

private:

    void deprecatedCamera (std::shared_ptr<Camera> camera);

    QTimer timer;

    camera_list cameras;
};

#endif /* UPDATEHANDLER_H */
