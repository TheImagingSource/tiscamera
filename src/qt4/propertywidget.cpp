#include "propertywidget.h"
#include "ui_propertywidget.h"

#include <memory>
#include <iostream>

using namespace tcam;

PropertyWidget::PropertyWidget (QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyWidget)
{}


PropertyWidget::PropertyWidget (QWidget *parent, tcam::Property* p) :
    QWidget(parent),
    ui(new Ui::PropertyWidget),
    property(*p)
{
    ui->setupUi(this);
    redraw();


}


PropertyWidget::~PropertyWidget ()
{
    delete ui;
}


QString PropertyWidget::getName ()
{
    return property.get_name().c_str();
}


tcam::Property PropertyWidget::getProperty ()
{
    return property;
}


void PropertyWidget::setProperty (const tcam::Property& p)
{
    this->property.set_struct(p.get_struct());
}


void PropertyWidget::update ()
{
    this->property.update();

    redraw();
}


void PropertyWidget::on_checkBox_toggled (bool val)
{
    PropertyBoolean& s = (PropertyBoolean&) property;
    s.set_value(ui->checkBox->isChecked());

    emit changed(this);
}


void PropertyWidget::on_comboBox_activated (const QString &arg1)
{
    PropertyStringMap& m = (PropertyStringMap&) property;
    m.set_value(arg1.toStdString());
    emit changed(this);
}


void PropertyWidget::on_horizontalSlider_sliderMoved (int position)
{

    if (property.get_type() == TCAM_PROPERTY_TYPE_INTEGER)
    {
        PropertyInteger& i = (PropertyInteger&) property;
        i.set_value(position);
        ui->valueDisplay->setText(QString::number(i.get_value()));
    }
    else
    {
        PropertyDouble& d = (PropertyDouble&) property;
        d.set_value((double) (position / precision));
        ui->valueDisplay->setText(QString::number(d.get_value()));
    }

    emit changed(this);
}


void PropertyWidget::on_pushButton_clicked ()
{
    PropertyButton& b = (PropertyButton&) property;

    b.activate();

    emit changed(this);
}


void PropertyWidget::redraw ()
{
    ui->propertyName->setText(QString(property.get_name().c_str()));

    TCAM_PROPERTY_TYPE type = property.get_type();

    if (property.get_ID() == TCAM_PROPERTY_WB_RED)
    {
        std::cout << "BLA" << std::endl;
    }

    if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        ui->checkBox->setVisible(true);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertyBoolean& prop_sw = (PropertyBoolean&) property;
        ui->checkBox->setChecked(prop_sw.get_value());
    }
    else if (type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        ui->horizontalSlider->setDisabled(false);
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(true);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertyInteger& prop_int = (PropertyInteger&) property;

        ui->horizontalSlider->setRange(prop_int.get_min(), prop_int.get_max());
        ui->horizontalSlider->setValue(prop_int.get_value());

        ui->valueDisplay->setText(QString::number(prop_int.get_value()));

        if (prop_int.is_read_only())
        {
            std::cout << "Is read only..."<< std::endl;
            ui->horizontalSlider->setDisabled(true);
            setStyleSheet("background-color:grey;");
        }
        else
        {
            ui->horizontalSlider->setDisabled(false);
            setStyleSheet("background-color:white;");
        }
        //ui->horizontalSlider->setEnabled (false);
        //setStyleSheet("background-color:grey;");
    }
    else if (type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(true);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);

        PropertyDouble& prop_d = (PropertyDouble&) property;

        double min = prop_d.get_min();
        double max = prop_d.get_max();
        ui->valueDisplay->setText(QString::number(prop_d.get_value()));
        //ui->horizontalSlider->setRange(min * presicion, max * this->precision);
        //ui->horizontalSlider->set_value(prop_d.get_value() * precision);
        if (prop_d.is_read_only())
        {
            std::cout << "Is read only..."<< std::endl;
            ui->horizontalSlider->setDisabled(true);
            setStyleSheet("background-color:grey;");
        }
        else
        {
            std::cout << "Is read NOT only..."<< std::endl;
            ui->horizontalSlider->setDisabled(false);
            setStyleSheet("background-color:white;");
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(false);
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING_TABLE)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        ui->comboBox->setVisible(true);

        PropertyStringMap& m = (PropertyStringMap&) property;

        for(auto s : m.get_values())
        {
            ui->comboBox->addItem(s.c_str());
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_BUTTON)
    {
        ui->checkBox->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(true);
        ui->comboBox->setVisible(false);

        ui->pushButton->setText("Push");
    }

}
