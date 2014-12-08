#include "propertyintegerwidget.h"
#include "ui_propertyintegerwidget.h"

PropertyIntegerWidget::PropertyIntegerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyIntegerWidget)
{
    ui->setupUi(this);
}

PropertyIntegerWidget::~PropertyIntegerWidget()
{
    delete ui;
}
