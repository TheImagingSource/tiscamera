

#ifndef TCAMSLIDER_H
#define TCAMSLIDER_H

#include <QSlider>

#include <cmath>

enum class TcamSliderScale
{
    Linear,
    Logarithmic,
};

class TcamSlider : public QSlider
{
    Q_OBJECT

public:
    TcamSlider(TcamSliderScale scale_type=TcamSliderScale::Linear);
    void setRange(double min, double max);
    void setSingleStep(double step);
    void setValue(double value);

    double value();

signals:

    void doubleClicked();
    void valueChanged(double);

private slots:

    void mouseDoubleClickEvent(QMouseEvent* event);
    void onSliderChanged(double);

private:

    int calculate_slider_value (double user_value);
    double calculate_user_value (int slider_value);


    TcamSliderScale m_scale_type = TcamSliderScale::Linear;

    double m_value_min = 0;
    double m_value_max = 100;
    double m_value_step = 1;
    double m_conversion_factor = 1000;

    double m_scale = 100.0;
};

#endif // TCAMSLIDER_H
