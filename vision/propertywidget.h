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

    PropertyWidget (QWidget *parent = 0, tcam::Property* = 0);
    ~PropertyWidget ();

    QString getName ();

    tcam::Property getProperty ();

    void setProperty (const tcam::Property&);

    void update ();

signals:
    void changed (PropertyWidget*);

private slots:

    void on_checkBox_toggled (bool val);

    void on_comboBox_activated (const QString &arg1);

    void on_horizontalSlider_sliderMoved (int position);

    void on_pushButton_clicked ();

private:

    Ui::PropertyWidget *ui;

    tcam::Property property;

    void redraw ();

    static const int precision = 1000;
};

#endif // PROPERTYWIDGET_H
