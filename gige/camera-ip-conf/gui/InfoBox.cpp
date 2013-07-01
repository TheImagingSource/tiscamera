///
/// @file InfoBox.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "InfoBox.h"
#include "ui_infobox.h"

InfoBox::InfoBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoBox)
{
    ui->setupUi(this);
    QString s = "Info Box";
    this->setWindowTitle(s);
}

InfoBox::~InfoBox()
{
    delete ui;
}

void InfoBox::showMessage (QString& message)
{
    this->ui->infoText->setText(message);
    this->setVisible(true);
}

void InfoBox::hideBox()
{
    this->setVisible(false);

}
