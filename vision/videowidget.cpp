#include "videowidget.h"

#include <iostream>

videowidget::videowidget(QWidget *parent) :
    QWidget(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->addWidget(imageLabel);
}


void videowidget::paintEvent(QPaintEvent *event)
{
    try
    {
        if (img.size().width() == 0)
        {
            return;
        }

        //resize()

        QPixmap map = QPixmap::fromImage(img);
        imageLabel->setPixmap(map);
        imageLabel->adjustSize ();
        imageLabel->resize (640,480);
        //imageLabel->resize (this->size ());
    }
    catch (...)
    {}
}
