#include "propertywidget.h"
#include "ui_propertywidget.h"

#include <memory>

using namespace tis_imaging;

PropertyWidget::PropertyWidget (QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyWidget)
{}


PropertyWidget::PropertyWidget (QWidget *parent, tis_imaging::Property* p) :
    QWidget(parent),
    ui(new Ui::PropertyWidget),
    property(*p)
{
    ui->setupUi(this);

    ui->propertyName->setText(QString(property.getName().c_str()));

    PROPERTY_TYPE type = property.getType();

    if (type == PROPERTY_TYPE_BOOLEAN)
    {
        ui->checkBox->setVisible(true);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertySwitch& prop_sw = (PropertySwitch&) property;
        ui->checkBox->setChecked(prop_sw.getValue());
    }
    else if (type == PROPERTY_TYPE_INTEGER)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(true);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertyInteger& prop_int = (PropertyInteger&) property;

        ui->horizontalSlider->setRange(prop_int.getMin(), prop_int.getMax());
        ui->horizontalSlider->setValue(prop_int.getValue());

        ui->valueDisplay->setText(QString::number(prop_int.getValue()));
        //ui->horizontalSlider->setEnabled (false);
        //setStyleSheet("background-color:grey;");

    }
    else if (type == PROPERTY_TYPE_DOUBLE)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(true);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertyDouble& prop_d = (PropertyDouble&) property;

        double min = prop_d.getMin();
        double max =  prop_d.getMax();
        ui->valueDisplay->setText(QString::number(prop_d.getValue()));
        //ui->horizontalSlider->setRange(min * presicion, max * this->precision);
        //ui->horizontalSlider->setValue(prop_d.getValue() * precision);
    }
    else if (type == PROPERTY_TYPE_STRING)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);
    }
    else if (type == PROPERTY_TYPE_STRING_TABLE)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(true);

        PropertyStringMap& m = (PropertyStringMap&) property;

        for(auto s : m.getValues())
        {
            ui->comboBox->addItem(s.c_str());
        }
    }
    else if (type == PROPERTY_TYPE_BUTTON)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(true);
        ui->comboBox->setVisible(false);

        ui->pushButton->setText("Push");
    }

}


PropertyWidget::~PropertyWidget ()
{
    delete ui;
}


QString PropertyWidget::getName ()
{
    return property.getName().c_str();
}


tis_imaging::Property PropertyWidget::getProperty ()
{
    return property;
}


void PropertyWidget::setProperty (const tis_imaging::Property& p)
{
    this->property.setStruct( p.getStruct());
}


void PropertyWidget::on_checkBox_toggled (bool val)
{
    PropertySwitch& s = (PropertySwitch&) property;
    s.setValue(ui->checkBox->isChecked());

    emit changed(this);
}


void PropertyWidget::on_comboBox_activated (const QString &arg1)
{
    PropertyStringMap& m = (PropertyStringMap&) property;
    m.setValue(arg1.toStdString());
    emit changed(this);
}


void PropertyWidget::on_horizontalSlider_sliderMoved (int position)
{

    if (property.getType() == PROPERTY_TYPE_INTEGER)
    {
        PropertyInteger& i = (PropertyInteger&) property;
        i.setValue(position);
        ui->valueDisplay->setText(QString::number(i.getValue()));
    }
    else
    {
        PropertyDouble& d = (PropertyDouble&) property;
        d.setValue((double)(position / precision));
        ui->valueDisplay->setText(QString::number(d.getValue()));
    }

    emit changed(this);
}


void PropertyWidget::on_pushButton_clicked ()
{
    PropertyButton& b = (PropertyButton&) property;

    b.activate();
}
