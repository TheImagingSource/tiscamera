#ifndef TCAMSLIDER_H
#define TCAMSLIDER_H

#include <QSlider>

class TcamSlider : public QSlider
{
public:
    TcamSlider();

signals:

    void doubleClicked();

private slots:

    void mouseDoubleClickEvent(QMouseEvent* event);
};

class TcamDoubleSlider : public TcamSlider
{
public:
    TcamDoubleSlider();
    void setValue(double value);
    void setSingleStep(double step);
    void setRange(double min, double max);

    double value();

private:
    int m_conversion_factor = 1000;
};

#endif // TCAMSLIDER_H
