#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tcam_qt4.h>

#include <iostream>

using namespace tcam;

MainWindow::MainWindow (QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
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

    status_label = new QLabel(this);

    statusBar()->addWidget(status_label);
}


MainWindow::~MainWindow ()
{
    delete ui;

    if (area != nullptr)
    {
        delete area;
        area = nullptr;
    }
}


void MainWindow::my_captureDevice_selected (tcam::DeviceInfo device)
{
    reset_gui();

    area = create_video_area(this, &device);

    // to move area into own window add
    // area->setWindowFlags(Qt::Window);
    // and do not add the widget to the rest of the application

    ui->horizontalLayout->insertWidget(0,area);

    open_device = device;

    ui->property_list = area->get_property_tree(ui->property_list);

    active_format = area->getVideoFormat();
    available_formats = area->getAvailableVideoFormats();

    // TODO: get rid of static definition
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
    if (area != nullptr)
    {
        delete area;
        area = nullptr;
    }

    QApplication::quit();
}


void MainWindow::on_actionPlay_Pause_triggered ()
{
    if (playing != true)
    {
        start_stream();
    }
    else
    {
        stop_stream();
    }
}


void MainWindow::reset_gui ()
{
    this->ui->format_box->clear();
    this->ui->size_box->clear();
    this->ui->framerate_box->clear();
    this->ui->binning_box->clear();
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


void MainWindow::update_single_property (QTreeWidgetItem* item)
{
        if (item->childCount() > 0)
        {
            for (unsigned int j = 0; j < item->childCount(); ++j)
            {
                update_single_property(item->child(j));
            }
        }

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


void MainWindow::update_properties ()
{
    for (unsigned int i = 0; i < ui->property_list->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = ui->property_list->topLevelItem(i);

        update_single_property(item);
    }
}


void MainWindow::on_actionContact_triggered()
{

}


void MainWindow::on_actionRefresh_everything_triggered ()
{
    update_properties ();
}
