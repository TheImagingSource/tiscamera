#include "capturedisplay.h"
#include "ui_capturedisplay.h"

#include <iostream>

CaptureDisplay::CaptureDisplay (QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CaptureDisplay)
{
    ui->setupUi(this);
}

CaptureDisplay::~CaptureDisplay ()
{
    delete ui;
}


void CaptureDisplay::paintEvent (QPaintEvent* e)
{

    if (m.width() == 0)
    {
        return;
    }

    if (new_image)
    {
        unsigned int w = ui->label->width();
        unsigned int h = ui->label->height();
        auto l = this->layout();

        int margin = l->margin();

        // ui->label->setPixmap(m.scaled(w - margin,
        //                               h -margin,
        //                               Qt::KeepAspectRatio,
        //                               Qt::FastTransformation ));


        ui->label->setPixmap(m.scaled(w,
                                      h,
                                      Qt::KeepAspectRatio));
                                      // ,
                                      // Qt::FastTransformation ));

        new_image=false;
    }
}


void CaptureDisplay::resizeEvent (QResizeEvent* event)
{
    //std::cout << "RESIZE!!!" << std::endl;


  // get label dimensions
  int w = ui->label->width();
  int h = ui->label->height();

  // set a scaled pixmap to a w x h window keeping its aspect ratio
  ui->label->setPixmap(m.scaled(w, h, Qt::KeepAspectRatio));

  ui->label->adjustSize();

}
