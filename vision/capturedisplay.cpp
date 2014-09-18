#include "capturedisplay.h"
#include "ui_capturedisplay.h"

#include <iostream>

CaptureDisplay::CaptureDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaptureDisplay)
{
    ui->setupUi(this);
}

CaptureDisplay::~CaptureDisplay()
{
    delete ui;
}


void CaptureDisplay::paintEvent(QPaintEvent * e)
{

    if (img.size().width() == 0)
    {
        return;
    }

    //std::cout << "paint event! ========================================================================" << std::endl;

//    unsigned int w = ui->label->width();
//    unsigned int h = ui->label->height();


    unsigned int w = ui->label->width();
    unsigned int h = ui->label->height();
    auto l = this->layout();

    int margin = l->margin();

    QPixmap map = QPixmap::fromImage(img);
//     QPixmap scaled= map.scaled ( w - margin,
//                                  h -margin,
//                                  Qt::KeepAspectRatio,
//                                  Qt::FastTransformation );
    //imageLabel->setBaseSize (width(), height());

ui->label->setPixmap(map);

//     ui->label->setPixmap(scaled);

    //adjustSize();
    ui->label->adjustSize();

    new_image=false;

}
