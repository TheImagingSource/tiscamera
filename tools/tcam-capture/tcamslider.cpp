#include "tcamslider.h"

#include <cmath>
#include <limits>

namespace
{

double log_(double value)
{
    if (value <= 0.0)
    {
        return 0;
    }

    return std::log(value);
}

}



TcamSlider::TcamSlider(TcamSliderScale scale_type)
    : m_scale_type(scale_type)
{
    setOrientation(Qt::Orientation::Horizontal);
    connect(this, &QSlider::valueChanged, this, &TcamSlider::onSliderChanged);

}

void TcamSlider::mouseDoubleClickEvent(QMouseEvent* event)
{
    // TODO: verify that double click can be received

    QSlider::mouseDoubleClickEvent(event);
}


void TcamSlider::setRange(double min, double max)
{
    m_value_min = min;
    m_value_max = max;

    if (m_scale_type == TcamSliderScale::Linear)
    {
        QSlider::setRange(min * m_scale, max * m_scale);
    }
    else
    {
        double min_ = calculate_slider_value(min);
        double max_ = calculate_slider_value(max);
        QSlider::setRange(min_, max_);
    }
}


void TcamSlider::setSingleStep(double step)
{
    if( step == 0. )
        step = 1.;
    
    m_value_step = step;
    m_scale = 1 / step;

    if (m_scale_type == TcamSliderScale::Linear)
    {
        QSlider::setSingleStep(step * m_scale);
    }
    else
    {
        QSlider::setSingleStep(step);
    }
}


void TcamSlider::setValue(double value)
{
    if (m_scale_type == TcamSliderScale::Linear)
    {
        QSlider::setValue(value * m_scale);
    }
    else
    {
        QSlider::setValue(calculate_slider_value(value));
    }
}

double TcamSlider::value()
{
    if (m_scale_type == TcamSliderScale::Linear)
    {
        return QSlider::value() / m_scale;
    }
    else
    {
        return calculate_user_value(QSlider::value());
    }
}

void TcamSlider::onSliderChanged(double value)
{
    if (m_scale_type == TcamSliderScale::Linear)
    {
        emit valueChanged(value / m_scale);
    }
    else
    {
        emit valueChanged(calculate_user_value(value));
    }
}


int TcamSlider::calculate_slider_value (double user_value)
{
    double minval = log_(m_value_min);
    double rangelen = log_(m_value_max) - minval;

    double val = log_(user_value);

    return 10000 / rangelen * (val - minval);
}


double TcamSlider::calculate_user_value (int slider_value)
{
    double minval = log_(m_value_min);

    double rangelen = log_(m_value_max) - minval;

    double val = std::exp(minval + rangelen / 10000 * slider_value);

    val -= fmod(val, m_value_step);

    return std::max(std::min(m_value_max, val), minval);
}
