#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "propertywidget.h"
#include <iostream>

MainWindow::MainWindow (QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    grabber(new tcam::CaptureDevice),
    selection_dialog(NULL),
    playing(false)
{
    ui->setupUi(this);

    selection_dialog = new DeviceInfoSelectionDialog();
    connect(selection_dialog,
            SIGNAL(device_selected(tcam::DeviceInfo)),
            this,
            SLOT(my_captureDevice_selected(tcam::DeviceInfo)));

    sink = std::make_shared<ImageSink>();

    status_label = new QLabel(this);

    statusBar()->addWidget(status_label);
}


MainWindow::~MainWindow ()
{
    delete ui;
    grabber->closeDevice();

    delete grabber;

}


tcam::CaptureDevice* MainWindow::getCaptureDevice ()
{

    return grabber;
}


void MainWindow::my_captureDevice_selected (tcam::DeviceInfo device)
{
    reset_gui();
    bool ret = grabber->openDevice(device);

    if (ret == false)
    {
        return;
    }

    open_device = device;
    std::cout << "Serial " << open_device.getSerial() << std::endl;

    auto props = grabber->getAvailableProperties();

    for (auto& p : props)
    {
        PropertyWidget* pw = new PropertyWidget(ui->property_list, &p);

        connect(pw,
                SIGNAL(changed(PropertyWidget*)),
                this,
                SLOT(property_changed(PropertyWidget*)));

        QListWidgetItem* item;
        item = new QListWidgetItem(ui->property_list);
        ui->property_list->setGridSize(QSize(0, pw->height()));
        ui->property_list->addItem(item);
        ui->property_list->setItemWidget(item, pw);
    }
    ui->property_list->setDragEnabled(false);
    ui->property_list->setFlow(QListView::TopToBottom);
    ui->property_list->update();

    active_format = grabber->getActiveVideoFormat();
    available_formats = grabber->getAvailableVideoFormats();

    std::cout << "Active format: " << active_format.toString() << std::endl;

    active_format.setFourcc(FOURCC_RGB32);

    if (available_formats.empty())
    {
        std::cerr <<  "No available formats!" << std::endl;
        return;
    }

    bool fill = false;
    for ( auto& a : available_formats)
    {
        tcam_video_format_description desc = a.getFormatDescription();
        ui->format_box->addItem(desc.description);

        if (fill == false)
        {
            auto res = a.getResolutions();
            for (tcam_image_size f : a.getResolutions())
            {
                QString s = QString::number(f.width);
                s += "x";
                s += QString::number(f.height);

                ui->size_box->addItem(s);
            }
            fill = true;
        }
        else
        {
            continue;
        }


        ui->binning_box->addItem(QString::number(desc.binning));
    }
}


void MainWindow::on_actionOpen_Camera_triggered ()
{
    selection_dialog->show();
}


void MainWindow::on_actionQuit_triggered ()
{
    if (grabber->isDeviceOpen())
    {
        grabber->closeDevice();
    }
    QApplication::quit();
}


void MainWindow::on_actionPlay_Pause_triggered ()
{
    if (!grabber->isDeviceOpen())
    {
        return;
    }

    if (playing != true)
    {
        start_stream();
    }
    else
    {
        stop_stream();
    }
}


void MainWindow::my_newImage_received(std::shared_ptr<MemoryBuffer> buffer)
{
    std::cout << "working on image" << std::endl;

    update();
}


void MainWindow::property_changed (PropertyWidget* pw)
{
    std::cout << "Changing Property" << std::endl;
}


void MainWindow::reset_gui ()
{
    this->ui->format_box->clear();
    this->ui->size_box->clear();
    this->ui->framerate_box->clear();
    this->ui->binning_box->clear();
}


void MainWindow::internal_callback(MemoryBuffer* buffer)
{
    if (buffer->getData() == NULL || buffer->getImageBuffer().length == 0)
    {
        return;
    }

    if (buffer->getImageBuffer().format.height != active_format.getSize().height|| buffer->getImageBuffer().format.width != active_format.getSize().width)
    {
        std::cout << "Faulty format!!!" << std::endl;
        return;
    }

    if (buffer->getImageBuffer().pitch == 0 || buffer->getImageBuffer().pitch > (1920 * 8))
    {
        return;
    }

    auto buf = buffer->getImageBuffer();

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
    ( (uint32_t)(unsigned char)(ch0) | ( (uint32_t)(unsigned char)(ch1) << 8 ) | \
      ( (uint32_t)(unsigned char)(ch2) << 16 ) | ( (uint32_t)(unsigned char)(ch3) << 24 ) )
#endif

    unsigned int height = buffer->getImageBuffer().format.height;
    unsigned int width = buffer->getImageBuffer().format.width;

    if (buf.format.fourcc == FOURCC_Y800)
    {
        this->ui->videowidget->m = QPixmap::fromImage(QImage(buf.pData,
                                                             width, height,
                                                             QImage::Format_Indexed8));
    }
    else if (buf.format.fourcc == FOURCC_Y16)
    {
        this->ui->videowidget->m = QPixmap::fromImage(QImage(buf.pData,
                                                             width, height,
                                                             QImage::Format_Mono));
    }
    else if (buf.format.fourcc == FOURCC_RGB24)
    {
        this->ui->videowidget->m = QPixmap::fromImage(QImage(buf.pData,
                                                             width, height,
                                                             QImage::Format_RGB666));
    }
    else if (buf.format.fourcc == FOURCC_RGB32)
    {
        this->ui->videowidget->m = QPixmap::fromImage(QImage(buf.pData,
                                                             width, height,
                                                             QImage::Format_RGB32));
    }
    else
    {
        std::cout << "Unable to interpret buffer format" << std::endl;
        return;
    }

    this->ui->videowidget->new_image = true;
    this->ui->videowidget->update();

    this->status_label->setText(QString(std::to_string (buffer->getStatistics().framerate).c_str ()));

    emit newImage_received(buffer);
}


void MainWindow::callback (MemoryBuffer* buffer, void* user_data)
{
    MainWindow* win = static_cast<MainWindow*>(user_data);

    win->internal_callback(buffer);

}

void MainWindow::on_format_box_currentIndexChanged (int index)
{
    std::cout <<  "New format index " << index << std::endl;

    if (index > available_formats.size())
    {
        std::cerr << index << " is an illegal index for format selection." << std::endl;
        return;
    }

    VideoFormatDescription f = available_formats.at(index);

    ui->size_box->clear();

    for (auto s : f.getResolutions())
    {
        QString qs = QString::number(s.width) + "x" + QString::number(s.height);
        ui->size_box->addItem(qs);
    }
}

void MainWindow::on_actionClose_Camera_triggered ()
{
    grabber->closeDevice();

    available_formats.clear();

    ui->property_list->clear();
    ui->format_box->clear();
    ui->size_box->clear();
    ui->binning_box->clear();

    playing = false;
}


bool MainWindow::getActiveVideoFormat ()
{
    VideoFormat input_format;

    int format_index = ui->format_box->currentIndex ();

    if (format_index == -1)
    {
        input_format.setFourcc(FOURCC_Y800);
    }

    VideoFormatDescription active_desc = available_formats.at(format_index);
    uint32_t f = active_desc.getFormatDescription().fourcc;

    input_format.setFourcc(f);

    int size_index = ui->size_box->currentIndex();
    if (size_index == -1)
    {
        input_format.setSize(640, 480);
    }
    else
    {
        auto size_desc = active_desc.getResolutions();
        input_format.setSize(size_desc.at(size_index).width, size_desc.at(size_index).height);

        auto res = active_desc.getResolutionsFramesrates().at(size_index);

        int fps_index = ui->framerate_box->currentIndex();

        input_format.setFramerate (res.fps.at(fps_index));

        //input_format.setFramerate(10.0);
    }

    input_format.setBinning(1);
    active_format = input_format;

    return true;
}


void MainWindow::start_stream ()
{
    bool ret = getActiveVideoFormat();

    if (!ret)
    {
        return;
    }

    ret = grabber->setVideoFormat(active_format);

    if (ret == false)
    {
        std::cout << "Unable to set video format! Exiting...." << std::endl;
        return;
    }
    sink->registerCallback(this->callback, this);

    ret = grabber->startStream(sink);

    if (ret == true)
    {
        std::cout << "RUNNING..." << std::endl;
        playing = true;
    }
}


void MainWindow::stop_stream ()
{
    grabber->stopStream();
    playing = false;
}


void MainWindow::on_size_box_currentIndexChanged(int index)
{
    int format_index = ui->format_box->currentIndex();

    if (format_index < 0)
    {
        return;
    }

    VideoFormatDescription desc = available_formats.at(format_index);

    int size_index = ui->size_box->currentIndex();

    if (size_index < 0)
    {
        return;
    }

    auto res = desc.getResolutionsFramesrates();

    ui->framerate_box->clear();

    for (auto& fps : res.at(size_index).fps)
    {
        QString qs = QString::number(fps);
        ui->framerate_box->addItem(qs);
    }
}
