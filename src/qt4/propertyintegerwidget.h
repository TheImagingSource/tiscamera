#ifndef PROPERTYINTEGERWIDGET_H
#define PROPERTYINTEGERWIDGET_H

#include <QWidget>

//#include "propertywidgetbase.h"

namespace Ui {
class PropertyIntegerWidget;
}

class PropertyIntegerWidget : public QWidget//, PropertyWidgetBase
{
    Q_OBJECT

public:
    explicit PropertyIntegerWidget(QWidget *parent = 0);
    ~PropertyIntegerWidget();

private:
    Ui::PropertyIntegerWidget *ui;
};

#endif // PROPERTYINTEGERWIDGET_H
