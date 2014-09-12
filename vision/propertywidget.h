#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QWidget>

#include <tis.h>

namespace Ui {
class PropertyWidget;
}

class PropertyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyWidget (QWidget *parent = 0);

    PropertyWidget (QWidget *parent = 0, tis_imaging::Property* = 0);
    ~PropertyWidget ();

    QString getName ();

    tis_imaging::Property getProperty ();

    void setProperty (const tis_imaging::Property&);

signals:
    void changed (PropertyWidget*);

private slots:
    void on_checkBox_released ();

    void on_comboBox_activated (const QString &arg1);

    void on_horizontalSlider_sliderMoved (int position);

    void on_pushButton_clicked ();

private:

    Ui::PropertyWidget *ui;

    tis_imaging::Property property;

    static const int precision = 1000;
};

#endif // PROPERTYWIDGET_H
