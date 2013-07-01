///
/// @file UpdateHandler.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "UpdateHandler.h"

#include <thread>
#include <functional>
#include <mutex>

UpdateHandler::UpdateHandler(QObject *parent) :
    QThread(parent)
{
    QObject::moveToThread(this);
}


UpdateHandler::~UpdateHandler ()
{
    this->quit();
    this->wait();
}


void UpdateHandler::run()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeCycle()));
    timer.start(2000);

    exec();
}


void UpdateHandler::timeCycle()
{
    // cameras from durrent discovery cycle
    camera_list new_cameras;

    // completely new cameras
    camera_list to_add;

    std::mutex cam_mutex;

    // callback function
    std::function<void(std::shared_ptr<Camera>)> func = [&] (std::shared_ptr<Camera> cam)
    {
        // compare function to find existing cameras
        auto find_cam =  [&cam] (std::shared_ptr<Camera> checkCam)
        {
            if(cam->getSerialNumber().compare(checkCam->getSerialNumber()) == 0)
            {
                return true;
            }
            return false;
        };


        cam_mutex.lock();

        // check if camera is unknown and trigger notify if so
        if (std::find_if(cameras.begin(), cameras.end(), find_cam) == cameras.end() )
        {
            //this->newCamera(cam);

            to_add.push_back(cam);
        }

        new_cameras.push_back(cam);
        cam_mutex.unlock();
    };

    std::thread t1([&func](){discoverCameras(func);});

    t1.join();

    // dead cameras
    camera_list toBeDeleted;

    // cleanup for deprecated cameras
    // not found => reduce counter
    // counter == 0 delete camera
    for (auto& cam : cameras)
    {
        bool found = false;
        for (auto& new_cam : new_cameras)
        {
            if (cam->getSerialNumber().compare(new_cam->getSerialNumber()) == 0)
            {
                cam->resetCounter();
                cam->updateCamera(new_cam);
                found = true;
            }
        }
        if (!found)
        {
            if (cam->reduceCounter() == 0)
            {
                toBeDeleted.push_back(cam);
            }
        }
    }

    for (auto& cam : toBeDeleted)
    {
        deprecatedCamera (cam);
    }

    cameras.insert(cameras.end(), to_add.begin(), to_add.end());
    emit update(cameras);
}


void UpdateHandler::deprecatedCamera (std::shared_ptr<Camera> camera)
{
    cameras.erase(std::remove(cameras.begin(),cameras.end(), camera), cameras.end());
    emit deprecated(camera);
}
