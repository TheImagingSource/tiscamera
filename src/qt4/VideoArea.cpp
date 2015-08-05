#include "VideoArea.h"
#include "ui_VideoArea.h"

#include "tcam_qt4.h"

#include <iostream>

using namespace tcam;


VideoArea::VideoArea (QWidget* parent, DeviceInfo* i)
    : QWidget(parent),
      ui(new Ui::VideoArea),
      device(nullptr),
      received_new_image(false),
      playing(false)
{
    ui->setupUi(this);

    if (i != nullptr)
    {
        device = std::make_shared<tcam::CaptureDevice>(*i);
    }


    //new QShortcut(QKeySequence(tr("Ctrl-SPACE")), this, SLOT(action()), 0, Qt::WidgetShortcut);
}


VideoArea::~VideoArea ()
{
    delete ui;
}


void VideoArea::start ()
{
    if (device == nullptr)
    {
        return;
    }

    sink = std::make_shared<ImageSink>();

    sink->registerCallback(this->callback, this);

    bool ret = device->start_stream(sink);

    if (ret == true)
    {
        std::cout << "RUNNING..." << std::endl;
        playing = true;
    }
}


void VideoArea::stop ()
{

    device->stop_stream();
    playing = false;
}


bool VideoArea::setVideoFormat (const tcam::VideoFormat& format)
{
    if (playing)
    {
        return false;
    }

    return device->set_video_format(format);
}


tcam::VideoFormat VideoArea::getVideoFormat () const
{
    return device->get_active_video_format();
}


std::vector<tcam::VideoFormatDescription> VideoArea::getAvailableVideoFormats () const
{
    return device->get_available_video_formats();
}


QTreeWidget* VideoArea::get_property_tree (QWidget* p)
{
    return create_property_tree(p, device->get_available_properties());
}


void VideoArea::callback (MemoryBuffer* buffer, void* user_data)
{
    VideoArea* self = static_cast<VideoArea*>(user_data);

    self->internal_callback(buffer);
}


void VideoArea::internal_callback(MemoryBuffer* buffer)
{
    if (buffer->get_data() == NULL || buffer->getImageBuffer().length == 0)
    {
        return;
    }

    // if (buffer->getImageBuffer().format.height != active_format.getSize().height|| buffer->getImageBuffer().format.width != active_format.getSize().width)
    // {
    //     std::cout << "Faulty format!!!" << std::endl;
    //     return;
    // }

    // TODO:
    if (buffer->getImageBuffer().pitch == 0 )
    {
        return;
    }

    auto buf = buffer->getImageBuffer();
    unsigned int height = buffer->getImageBuffer().format.height;
    unsigned int width = buffer->getImageBuffer().format.width;

    if (buf.format.fourcc == FOURCC_Y800)
    {
        this->m = QPixmap::fromImage(QImage(buf.pData,
                                            width, height,
                                            QImage::Format_Indexed8));
    }
    else if (buf.format.fourcc == FOURCC_Y16)
    {
        this->m = QPixmap::fromImage(QImage(buf.pData,
                                            width, height,
                                            QImage::Format_Mono));
    }
    else if (buf.format.fourcc == FOURCC_RGB24)
    {
        QImage image = QImage(buf.pData,
                              width, height,
                              QImage::Format_RGB888);
        // rgb24 is really bgr, thus swap r <-> b
        this->m = QPixmap::fromImage(image.rgbSwapped());
    }
    else if (buf.format.fourcc == FOURCC_RGB32)
    {
        this->m = QPixmap::fromImage(QImage(buf.pData,
                                            width, height,
                                            QImage::Format_RGB32));
    }
    else
    {
        std::cout << "Unable to interpret buffer format" << std::endl;
        return;
    }

    this->received_new_image = true;
    this->update();

}



void VideoArea::paintEvent (QPaintEvent* e)
{

    if (m.width() == 0)
    {
        return;
    }

    if (received_new_image)
    {
        unsigned int w = ui->label->width();
        unsigned int h = ui->label->height();
        auto l = this->layout();

        int margin = l->margin();

        // ui->label->setPixmap(m.scaled(w - margin,
        //                               h -margin,
        //                               Qt::KeepAspectRatio,
        //                               Qt::FastTransformation ));


        ui->label->setPixmap(m);

        // ui->label->setPixmap(m.scaled(w,
        //                               h,
        //                               Qt::KeepAspectRatio));

        // ,
                                      // Qt::FastTransformation ));

        new_image=false;
        received_new_image = false;
    }
}


void VideoArea::resizeEvent (QResizeEvent* event)
{
    //std::cout << "RESIZE!!!" << std::endl;


  // get label dimensions
  int w = ui->label->width();
  int h = ui->label->height();

  // set a scaled pixmap to a w x h window keeping its aspect ratio
  // ui->label->setPixmap(m.scaled(w, h, Qt::KeepAspectRatio));
  ui->label->setPixmap(m);

  //ui->label->adjustSize();

}
