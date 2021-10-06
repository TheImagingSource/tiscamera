

#pragma once

#include "stdint.h"
#include <gst/gst.h>

#include <QObject>
#include <QMutex>
#include <chrono>
#include <queue>

class QTimer;

class FPSCounter : public QObject
{

    Q_OBJECT

public:

    explicit FPSCounter();
    ~FPSCounter();

    static void fps_callback(GstElement* fps_sink,
                             gdouble fps,
                             gdouble droprate,
                             gdouble avgfps,
                             gpointer user_data);

    void start();

    void stop();

public slots:

    void tick();

signals:

    void new_fps_measurement(double fps);

private:

    void update_values();

    // std::chrono::high_resolution_clock::time_point m_start_time;
    //std::chrono::high_resolution_clock::time_point m_last_time;

    std::queue<double> m_queue;

    bool m_running = false;
    // uint64_t m_framecount = 0;

    // double m_actual_fps = 0.0;
    // double m_last_fps = 0.0;
    // unsigned int m_frames_in_last_second = 0;

    QMutex m_mutex;

    QTimer* p_timer;

};
