#include "tcamslider.h"

TcamSlider::TcamSlider()
{
    setOrientation(Qt::Orientation::Horizontal);
}

void TcamSlider::mouseDoubleClickEvent(QMouseEvent* event)
{
    //emit this->doubleClicked();

    QSlider::mouseDoubleClickEvent(event);
}

TcamDoubleSlider::TcamDoubleSlider() {}


void TcamDoubleSlider::setRange(double min, double max)
{
    QSlider::setRange(min * m_conversion_factor, max * m_conversion_factor);
}


void TcamDoubleSlider::setSingleStep(double step)
{
    QSlider::setSingleStep(step * m_conversion_factor);
}


void TcamDoubleSlider::setValue(double value)
{
    QSlider::setValue(value * m_conversion_factor);
}

double TcamDoubleSlider::value()
{
    return QSlider::value() / m_conversion_factor;
}
