#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tcam_qt4.h>

#include <iostream>

using namespace tcam;

MainWindow::MainWindow (QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    grabber(nullptr),
    selection_dialog(NULL),
    playing(false),
    area(nullptr)
{
    ui->setupUi(this);

    selection_dialog = new DeviceSelectionDialog();
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

    if (grabber != nullptr)
    {
        delete grabber;
        grabber = nullptr;
    }

    if (area != nullptr)
    {
        delete area;
        area = nullptr;
    }
}


tcam::CaptureDevice* MainWindow::getCaptureDevice ()
{

    return grabber;
}


void MainWindow::my_captureDevice_selected (tcam::DeviceInfo device)
{
    reset_gui();

    area = create_video_area(this, &device);

    // to move area into own window add
    // area->setWindowFlags(Qt::Window);
    // and do not add the widget to the rest of the application

    ui->horizontalLayout->insertWidget(0,area);

    grabber = new CaptureDevice(device);

    open_device = device;
    std::cout << "Serial " << open_device.getSerial() << std::endl;

    auto props = grabber->getAvailableProperties();

    ui->property_list = tcam::create_property_tree(ui->property_list, props);

    ui->property_list->update();
    ui->property_list->show();

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
        tcam_video_format_description desc = a.getStruct();
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
    }
}


void MainWindow::on_actionOpen_Camera_triggered ()
{
    selection_dialog->show();
}


void MainWindow::on_actionQuit_triggered ()
{

    if (grabber != nullptr)
    {
        delete grabber;
    }

    if (area != nullptr)
    {
        delete area;
        area = nullptr;
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
    update_properties();
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
    this->status_label->setText(QString(std::to_string (buffer->getStatistics().framerate).c_str ()));
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

    if (grabber != nullptr)
    {
        delete grabber;
        grabber = nullptr;
    }
    available_formats.clear();

    ui->property_list->clear();
    ui->format_box->clear();
    ui->size_box->clear();
    ui->binning_box->clear();

    playing = false;


    if (area != nullptr)
    {
        delete area;
        area = nullptr;
    }
}


bool MainWindow::getActiveVideoFormat ()
{
    VideoFormat input_format;

    int format_index = ui->format_box->currentIndex ();

    if (format_index == -1)
    {
        input_format.setFourcc(FOURCC_Y800);
    }

    if (format_index > available_formats.size())
    {
        std::cerr << "Faulty format index(" << format_index << ") max is "
                  << available_formats.size() << std::endl;
        return false;
    }

    VideoFormatDescription active_desc = available_formats.at(format_index);
    uint32_t f = active_desc.getFourcc();

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
    }

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


    if (area != nullptr)
    {
        area->setVideoFormat(active_format);
        area->start();
        area->show();
    }
    else
    {
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
}


void MainWindow::stop_stream ()
{
    if (area != nullptr)
    {
        area->stop();
    }
    playing = false;
}


void MainWindow::on_size_box_currentIndexChanged (int index)
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


void MainWindow::update_properties ()
{
    for (unsigned int i = 0; i < ui->property_list->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = ui->property_list->topLevelItem(i);

        PropertyWidget* pw = dynamic_cast<PropertyWidget*>(ui->property_list->itemWidget(item, 0));
        if (pw != nullptr)
        {
            pw->update();
        }
        else
        {
            std::cout << "NULLPTR" << std::endl;
        }
    }
}


void MainWindow::on_actionContact_triggered()
{

}
