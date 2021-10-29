/*
 * Copyright 2021 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QMenu>
#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QMessageBox>
// #include <QVideoWidget>
#include <gst/gst.h>

#include "caps.h"
#include "device.h"
#include "aboutdialog.h"
#include "devicedialog.h"
#include "propertydialog.h"
#include "optionsdialog.h"
#include "tcamcollection.h"

#include <regex>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <gst/video/videooverlay.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    p_property_dialog = nullptr;
    setWindowTitle("TCam Capture");
    setWindowIcon(QIcon(":/images/logo.png"));
    m_index = std::make_shared<Indexer>();
    connect(m_index.get(), &Indexer::device_lost, this, &MainWindow::device_lost_cb);

    p_pipeline = nullptr;

    load_settings();

    // toolbar stuff

    // ownership of p_action* lies with the toolbar
    // we retain pointer to them to enable/disable them later
    // of no concern otherwise

    p_toolbar = new QToolBar("Main");

    auto action_quit = new QAction(QIcon::fromTheme(":/images/close.png"), "Quit");
    action_quit->setShortcut(QKeySequence("Ctrl+Shift+Q"));

    connect(action_quit, &QAction::triggered, this, &MainWindow::on_actionQuit_triggered);
    p_toolbar->addAction(action_quit);

    auto action_open = new QAction(QIcon(":/images/camera.png"), "Open Device");
    action_open->setShortcut(QKeySequence("Ctrl+O"));
    connect(action_open, &QAction::triggered, this, &MainWindow::on_actionOpen_Device_triggered);
    p_toolbar->addAction(action_open);

    p_action_property_dialog = new QAction(QIcon(":/images/dialog.png"), "Open Property Dialog");
    p_action_property_dialog->setShortcut(QKeySequence("Ctrl+P"));
    connect(p_action_property_dialog, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);

    p_toolbar->addAction(p_action_property_dialog);

    p_action_format_dialog = new QAction(QIcon(":/images/VideoFormat.png"), "Open Format Dialog");
    p_action_format_dialog->setShortcut(QKeySequence("Ctrl+U"));
    connect(p_action_format_dialog, &QAction::triggered, this, &MainWindow::open_format_triggered);

    p_toolbar->addAction(p_action_format_dialog);


    auto action_about = new QAction(QIcon::fromTheme("SP_BrowserStop"), "Info");
    action_about->setShortcut(QKeySequence("Ctrl+I"));
    connect(action_about, &QAction::triggered, this, &MainWindow::open_about_triggered);
    p_toolbar->addAction(action_about);

    auto action_options = new QAction(QIcon(":/images/Settings.png"), "Options");
    connect(action_options, &QAction::triggered, this, &MainWindow::open_options_triggered);
    p_toolbar->addAction(action_options);

    enable_device_gui_elements(false);

    p_toolbar->setMovable(false);
    this->addToolBar(p_toolbar);

    // status bar stuff

    p_fps_label = new QLabel();
    p_settings_label = new QLabel();

    p_settings_label->setText("");

    this->statusBar()->addPermanentWidget(p_settings_label);
    this->statusBar()->addPermanentWidget(p_fps_label);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    close_pipeline();
    save_settings();
    event->accept();
}


void MainWindow::use_dutils(bool use_it)
{
    m_use_dutils = use_it;
}

static gboolean bus_callback(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    switch(GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_INFO:
        {
            char* str;
            GError* err = nullptr;
            gst_message_parse_info(message, &err, &str);

            qInfo("INFO: %s", str);

            QString s = str;

            // infos concerning the caps that are actually set
            // set infos for users
            if (s.startsWith("Working with src caps:"))
            {
                s = s.remove( QRegExp( "\\(\\w*\\)" ) );
                s = s.section(":",1);

                // qInfo("`````````````````````````````````````````````%s", s.toStdString().c_str());

                ((MainWindow*)user_data)->set_settings_string(s);
            }

            if (err)
            {
                qInfo("%s", err->message);
                g_clear_error(&err);
            }
            break;
        }
        case GST_MESSAGE_ERROR:
        {

            char* str = nullptr;
            GError* err = nullptr;
            gst_message_parse_error(message, &err, &str);
            QString s = err->message;
            if (s.startsWith("Device lost ("))
            {
                QString serial = s.mid(13, s.length()-2);

                ((MainWindow*)user_data)->device_lost(serial);
            }

            g_error_free(err);
            g_free(str);

            break;
        }
        case GST_MESSAGE_EOS:
        {
            qInfo("Received EOS");

            break;
        }
        case GST_MESSAGE_STREAM_START:
        {
            // all sink elements are playing.
            // stream actually started

            break;
        }
        case GST_MESSAGE_STREAM_STATUS:
        {
            GstStreamStatusType type;
            GstElement* element = nullptr;
            gst_message_parse_stream_status(message, &type, &element);

            //qInfo("STREAM status: %s: %s", GST_ELEMENT_NAME(element), type);

            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
        {
            // GstState oldstate;
            // GstState newstate;
            // GstState pending;
            // gst_message_parse_state_changed(message, &oldstate, &newstate, &pending);

            // qInfo("State change: old: %s new: %s pend: %s",
            //       gst_element_state_get_name(oldstate),
            //       gst_element_state_get_name(newstate),
            //       gst_element_state_get_name(pending));

            break;
        }
        case GST_MESSAGE_ELEMENT:
        {
            //auto struc = gst_message_get_structure(message);

            //gst_message_

            break;
        }
        default:
        {
            qInfo("Message handling not implemented: %s", GST_MESSAGE_TYPE_NAME(message));
            break;
        }

    }

    return TRUE;
}

void MainWindow::on_actionOpen_Device_triggered()
{
    DeviceDialog dialog(m_index);
    dialog.set_format_handling(m_config.format_selection_type);

    auto res = dialog.exec();

    m_config.format_selection_type = dialog.get_format_handling();

    if (res == QDialog::Accepted)
    {
        if (p_selected_caps)
        {
            gst_caps_unref(p_selected_caps);
        }
        if (p_pipeline)
        {
            if (p_property_dialog)
            {
                p_property_dialog->hide();

                p_property_dialog->close();
                p_property_dialog = nullptr;
            }
            close_pipeline();
        }

        m_selected_device = dialog.get_selected_device();

        qInfo("device selected: %s\n", m_selected_device.str().c_str());

        open_pipeline(dialog.get_format_handling());

    }
    else
    {
        qInfo("No device selected\n");
    }
}

void MainWindow::on_actionQuit_triggered()
{
    // qInfo("quit triggered");

    close_pipeline();
    QApplication::quit();
}


void MainWindow::open_pipeline(FormatHandling handling)
{
    std::string serial = m_selected_device.serial_long();

    std::string pipeline_string = m_config.pipeline.toStdString();
    GError* err = nullptr;

    bool set_device = false;

    if (p_pipeline)
    {
        GstState state;
        GstState pending;
        // wait 0.1 sec
        GstStateChangeReturn change_ret = gst_element_get_state(p_pipeline, &state, &pending, 100000000);

        if (change_ret == GST_STATE_CHANGE_SUCCESS)
        {
            if (state == GST_STATE_PAUSED || state == GST_STATE_PLAYING)
            {
                gst_element_set_state(p_pipeline, GST_STATE_READY);
            }
        }
        else
        {
            qWarning("Unable to determine pipeline state. Attempting restart.");
            close_pipeline();
        }

    }
    else
    {
        set_device = true;
        p_pipeline = gst_parse_launch(pipeline_string.c_str(), &err);
    }

    if (!p_pipeline)
    {
        qErrnoWarning("Unable to start pipeline!");
        if (err)
        {
            qErrnoWarning("Reason: %s", err->message);
        }
        return;
    }
    else
    {
        qInfo("Started pipeline: %s", pipeline_string.c_str());
    }

    auto bus = gst_pipeline_get_bus (GST_PIPELINE(p_pipeline));
    m_gst_bus_id = gst_bus_add_watch(bus, bus_callback, this);
    gst_object_unref(bus);

    auto has_property = [](GstElement* element, const char* name)
    {
        return g_object_class_find_property(G_OBJECT_GET_CLASS(element), name) != nullptr;
    };

    if (set_device)
    {
        p_source = gst_bin_get_by_name(GST_BIN(p_pipeline), "tcam0");

        if (!p_source)
        {
            // TODO throw error to user
            qInfo("NO source for you");
            return;
        }

        if (has_property(p_source, "serial"))
        {
            g_object_set(p_source, "serial", serial.c_str(), nullptr);
        }
    }
    // if (has_property(p_source, "use-dutils"))
    // {
    //     g_object_set(p_source, "use-dutils", m_config.use_dutils, nullptr);
    // }

    // must not be freed
    GstElementFactory* factory = gst_element_get_factory(p_source);
    GType element_type = gst_element_factory_get_element_type(factory);

    gst_element_set_state(p_source, GST_STATE_READY);

    // we want the device caps
    // there are two scenarios
    // tcambin -> get subelement
    // source element -> use that
    GstElement* tcamsrc = nullptr;
    if (element_type == g_type_from_name("GstTcamBin"))
    {
        tcamsrc = gst_bin_get_by_name(GST_BIN(p_source), "tcambin-source");
    }
    else
    {
        tcamsrc = p_source;
        gst_object_ref(tcamsrc);
    }

    GstCaps* src_caps = nullptr;
    if (has_property(p_source, "available-caps"))
    {
        const char* available_caps = nullptr;
        g_object_get(p_source, "available-caps", &available_caps, NULL);
        src_caps = gst_caps_from_string(available_caps);
        m_selected_device.set_caps(src_caps);
    }
    else
    {
        GstPad* src_pad = gst_element_get_static_pad(tcamsrc, "src");

        src_caps = gst_pad_query_caps(src_pad, nullptr);

        gst_object_unref(src_pad);

        m_selected_device.set_caps(src_caps);

        gst_object_unref(tcamsrc);
    }

    GstCaps* caps;
    if (handling == FormatHandling::Dialog)
    {
        //on_actionOpenFormat_triggered();
        p_selected_caps = open_format_dialog();

        caps = p_selected_caps;
        qInfo("Manually selected caps are: %s", gst_caps_to_string(caps));

    }
    else if (handling == FormatHandling::Static)
    {
        caps = p_selected_caps;
        qInfo("Manually selected caps are: %s", gst_caps_to_string(caps));
    }
    else
    {
        auto c = Caps(src_caps);
        caps = c.get_default_caps();

        qInfo("Auto selected caps are: %s", gst_caps_to_string(caps));
    }

    if (src_caps)
    {
        gst_caps_unref(src_caps);
    }

    if (has_property(p_source, "device-caps"))
    {
        qInfo("settings caps to : %s", gst_caps_to_string(caps));
        g_object_set(p_source, "device-caps", gst_caps_to_string(caps), nullptr);
    }
    else
    {
        auto capsfilter = gst_bin_get_by_name(GST_BIN(p_pipeline), "device-caps");

        if (!capsfilter )
        {
            qWarning("Source does not have property 'device-caps'.");
            qWarning("Alternative of capsfilter named 'device-caps' does not exist.");
        }
        else
        {
            // TODO check if element really is a capsfilter
            g_object_set(capsfilter, "caps", caps, nullptr);
        }
    }

    p_displaysink = gst_bin_get_by_name(GST_BIN(p_pipeline), "sink");

    if (has_property(p_displaysink, "video-sink"))
    {
        GstElement* disp;
        g_object_get(p_displaysink, "video-sink", &disp, nullptr);
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(disp), this->ui->widget->winId());
    }

    m_fps_signal_id = g_signal_connect(p_displaysink,
                                       "fps-measurements",
                                       G_CALLBACK(&FPSCounter::fps_callback), &m_fps_counter);

    gst_element_set_state(p_pipeline, GST_STATE_PLAYING);

    connect(&m_fps_counter, &FPSCounter::new_fps_measurement, this, &MainWindow::fps_tick);
    m_fps_counter.start();

    enable_device_gui_elements(true);

    // do this last
    // Class queries elements automatically
    // at this point all properties have to be available
    m_tcam_collection = TcamCollection(GST_BIN(p_pipeline));
}

void MainWindow::close_pipeline()
{
    m_fps_counter.stop();

    enable_device_gui_elements(false);

    if (p_pipeline)
    {
        gst_element_set_state(p_pipeline, GST_STATE_NULL);
        gst_object_unref(p_pipeline);
        p_pipeline = nullptr;

        if (p_about)
        {
            p_about->set_tcambin(nullptr);
        }
        p_source = nullptr;
    }

    reset_fps_tick();

    // repaint the display widget
    // do this to not have the last received image
    // displayed indefinitely
    ui->widget->repaint();
}


void MainWindow::load_settings()
{
    m_config.load();
}


void MainWindow::save_settings()
{
    m_config.save();
}

void MainWindow::on_actionPlay_triggered()
{
    open_pipeline(FormatHandling::Auto);
}

void MainWindow::on_actionStop_triggered()
{
    close_pipeline();
}

void MainWindow::open_property_dialog()
{
    if (p_property_dialog)
    {
        return;
    }

    if (!p_pipeline)
    {
        return;
    }

    p_property_dialog = new PropertyDialog(m_tcam_collection);
    p_property_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(p_property_dialog, &QDialog::finished, this, &MainWindow::free_property_dialog);
    connect(p_property_dialog, &PropertyDialog::device_lost, this, &MainWindow::device_lost);

    p_property_dialog->show();
}


void MainWindow::free_property_dialog()
{
    // this only exists to ensure that only one dialog instance can exist
    // deletion is taken care of by WA_DeleteOnClose
    p_property_dialog = nullptr;
}


void MainWindow::open_about_triggered()
{
    p_about = new AboutDialog(this);

    if (p_source)
    {
        p_about->set_tcambin(p_source);
    }

    p_about->show();
}


void MainWindow::open_options_triggered()
{
    OptionsSettings settings;

    auto dia = OptionsDialog(m_config, settings, this);

    int res = dia.exec();

    if (res == QDialog::Accepted)
    {
        m_config = dia.get_config();
    }
}


void MainWindow::enable_device_gui_elements(bool toggle)
{
    p_action_property_dialog->setEnabled(toggle);
    p_action_format_dialog->setEnabled(toggle);

    if (p_about)
    {
        p_about->set_tcambin(p_source);
    }
}


void MainWindow::reset_fps_tick()
{
    p_fps_label->setText("");
}


void MainWindow::fps_tick(double new_fps)
{
    p_fps_label->setText("FPS: " + QString::number(new_fps));
}


void MainWindow::device_lost_cb(const Device& dev)
{
    if (dev == m_selected_device)
    {
        device_lost(dev.serial().c_str());
    }
}


void MainWindow::device_lost(const QString& message)
{
    //qError("Lost device. Stopping...");
    if (!message.isEmpty())
    {
        qWarning("%s", message.toStdString().c_str());
    }

    if (message != m_selected_device.serial().c_str())
    {
        return;
    }

    close_pipeline();

    free_property_dialog();

    auto error_dialog = new QMessageBox(this);
    error_dialog->setCheckBox(nullptr);

    error_dialog->setIcon(QMessageBox::Critical);
    error_dialog->setWindowTitle("Device lost");
    QString s = "Device has been lost. Please reconnect or re-open it.\n\n";
    s += message;
    error_dialog->setText(s);
    //error_dialog->showMessage(s);

    error_dialog->exec();
    //error_dialog->show();
}


void MainWindow::set_settings_string(const QString str)
{
    p_settings_label->setText(str);
}


void MainWindow::on_widget_customContextMenuRequested(const QPoint& pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("Open Property Dialog", this);
    connect(&action1, SIGNAL(triggered()), this, SLOT(open_property_dialog()));
    contextMenu.addAction(&action1);

    contextMenu.exec(mapToGlobal(pos));
}

void MainWindow::on_actionOpen_triggered()
{
    open_property_dialog();
}


GstCaps* MainWindow::open_format_dialog()
{

    auto format_dialog = QDialog();

    auto layout = new QVBoxLayout();

    format_dialog.setLayout(layout);

    //qInfo("Device caps for format dialog: %s", gst_caps_to_string(m_selected_device.caps()));
    auto fmt_widget = new CapsWidget(m_selected_device.caps());

    layout->addWidget(fmt_widget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, &format_dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &format_dialog, &QDialog::reject);

    layout->addWidget(buttonBox);

    if (format_dialog.exec() == QDialog::Accepted)
    {
        GstCaps* new_caps = fmt_widget->get_caps();

        if (p_selected_caps)
        {
            gst_caps_unref(p_selected_caps);
        }

        //p_selected_caps = new_caps;
        return new_caps;
    }
    return nullptr;
}


void MainWindow::open_format_triggered()
{
    auto caps = open_format_dialog();

    if (caps)
    {
        p_selected_caps = caps;
        open_pipeline(FormatHandling::Static);
    }

}
