
#include "fpscounter.h"

#include <QTimer>


FPSCounter::FPSCounter()
{
    p_timer = new QTimer();

    QObject::connect(p_timer, &QTimer::timeout, this, &FPSCounter::tick);
}


FPSCounter::~FPSCounter()
{
    if (m_running)
    {
        stop();
    }
    delete p_timer;
}


void FPSCounter::fps_callback(GstElement* /*fps_sink*/,
                              gdouble fps,
                              gdouble /*droprate*/,
                              gdouble /*avgfps*/,
                              gpointer user_data)
{

    FPSCounter* self = static_cast<FPSCounter*>(user_data);

    self->m_mutex.lock();

    self->m_queue.push(fps);

    self->m_mutex.unlock();
}

void FPSCounter::start()
{
    m_running = true;
    p_timer->start(1000);
}


void FPSCounter::stop()
{
    p_timer->stop();
    m_running = false;
}


void FPSCounter::tick()
{
    update_values();
}


void FPSCounter::update_values()
{
    m_mutex.lock();

    auto count = m_queue.size();
    double fps = 0.0;
    while(!m_queue.empty())
    {
        fps += m_queue.front();
        m_queue.pop();
    }

    m_mutex.unlock();

    if (fps != 0.0 && count != 0)
    {
        fps /= count;
    }

    emit this->new_fps_measurement(fps);
}
