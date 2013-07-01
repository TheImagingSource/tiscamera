///
/// @file Updatehandler.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///
/// @brief Handles updates of listed cameras
///

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
