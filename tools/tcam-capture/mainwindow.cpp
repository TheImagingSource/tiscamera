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

#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QMenu>
#include <QMessageBox>
// #include <QVideoWidget>
#include "aboutdialog.h"
#include "caps.h"
#include "definitions.h"
#include "device.h"
#include "devicedialog.h"
#include "filename_generator.h"
#include "optionsdialog.h"
#include "propertydialog.h"

#include <glib-object.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <memory>
#include <qimage.h>
#include "../../libs/tcam-property//src/gst/meta/gstmetatcamstatistics.h"
#include "videosaver.h"


namespace
{

bool has_property (GstElement* element, const char* name)
{
    return g_object_class_find_property(G_OBJECT_GET_CLASS(element), name) != nullptr;
};

} // namespace


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    p_property_dialog = nullptr;
    setWindowTitle("tcam-capture");
    setWindowIcon(QIcon(":/images/logo.png"));
    m_index = std::make_shared<Indexer>();
    connect(m_index.get(), &Indexer::device_lost, this, &MainWindow::device_lost_cb);

    p_pipeline = nullptr;

    load_settings();

    // toolbar stuff

    // prevent context menus that allow removal of the main toolbar
    setContextMenuPolicy(Qt::NoContextMenu);

    // ownership of p_action* lies with the toolbar
    // we retain pointer to them to enable/disable them later
    // of no concern otherwise

    p_toolbar = new QToolBar("Main");

    auto action_quit = new QAction(QIcon::fromTheme("application-exit"), "Quit");
    action_quit->setShortcut(QKeySequence("Ctrl+Shift+Q"));

    connect(action_quit, &QAction::triggered, this, &MainWindow::on_actionQuit_triggered);
    p_toolbar->addAction(action_quit);

    auto action_open = new QAction(QIcon(":/images/camera.png"), "Open Device");
    action_open->setShortcut(QKeySequence("Ctrl+O"));
    connect(action_open, &QAction::triggered, this, &MainWindow::on_actionOpen_Device_triggered);
    p_toolbar->addAction(action_open);

    p_action_property_dialog = new QAction("Properties");
    p_action_property_dialog->setStatusTip("Open Property Dialog");
    p_action_property_dialog->setShortcut(QKeySequence("Ctrl+P"));
    connect(
        p_action_property_dialog, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);

    p_toolbar->addAction(p_action_property_dialog);

    p_action_format_dialog = new QAction("Format");
    p_action_format_dialog->setToolTip("Open Format Dialog");
    p_action_format_dialog->setShortcut(QKeySequence("Ctrl+U"));
    connect(p_action_format_dialog, &QAction::triggered, this, &MainWindow::open_format_triggered);

    p_toolbar->addAction(p_action_format_dialog);


    auto action_about = new QAction("Info");
    action_about->setStatusTip("Open Info Dialog");
    action_about->setShortcut(QKeySequence("Ctrl+I"));
    connect(action_about, &QAction::triggered, this, &MainWindow::open_about_triggered);
    p_toolbar->addAction(action_about);

    auto action_options = new QAction(QIcon::fromTheme("preferences-other"), "Options");
    connect(action_options, &QAction::triggered, this, &MainWindow::open_options_triggered);
    p_toolbar->addAction(action_options);

    p_action_save_image = new QAction(QIcon(":/images/snap.png"), "Save Image");
    connect(p_action_save_image, &QAction::triggered, this, &MainWindow::save_image_triggered);
    p_toolbar->addAction(p_action_save_image);

    p_action_save_video = new QAction(QIcon(":/images/start_capture.png"), "Record Video");
    connect(p_action_save_video, &QAction::triggered, this, &MainWindow::save_video_triggered);
    p_toolbar->addAction(p_action_save_video);

    p_toolbar->setMovable(false);
    this->addToolBar(p_toolbar);

    // status bar stuff

    p_fps_label = new QLabel();

    this->statusBar()->addPermanentWidget(p_fps_label);

    enable_device_gui_elements(false);

    // open device dialog to make it more obvious what to do next
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent* event)
{
    close_pipeline();
    save_settings();
    event->accept();
}

void MainWindow::on_actionQuit_triggered()
{
    // this and closeEvent are twins
    // this function is a callback for keybindings/buttons
    // closeEvent as an event for things like alt-f4
    close_pipeline();
    save_settings();
    QApplication::quit();
}


bool MainWindow::open_device(const QString& serial)
{
    auto device_list = m_index->get_device_list();

    bool found_it = false;
    for (const auto& dev : device_list)
    {
        if (dev.serial_long() == serial.toStdString() || dev.serial() == serial.toStdString())
        {
            found_it = true;
            m_selected_device = dev;
            break;
        }
    }

    if (!found_it)
    {
        return false;
    }

    qInfo("device selected: %s\n", m_selected_device.str().c_str());

    open_pipeline(FormatHandling::Auto);

    return true;
}


gboolean MainWindow::bus_callback(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_INFO:
        {
            char* str;
            GError* err = nullptr;
            gst_message_parse_info(message, &err, &str);

            //qInfo("INFO: %s", str);

            QString s = str;

            // infos concerning the caps that are actually set
            // set infos for users
            if (s.startsWith("Working with src caps:"))
            {
                s = s.remove(QRegExp("\\(\\w*\\)"));
                s = s.section(":", 1);

                auto mw = ((MainWindow*)user_data);
                if (mw->p_about)
                {
                    mw->p_about->set_device_caps(s);
                }
                mw->m_device_caps = s;
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
                int start = s.indexOf("(") + 1;
                QString serial = s.mid(start, s.indexOf(")") - start);
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
        case GST_MESSAGE_ELEMENT:
        {
            const GstStructure* s = gst_message_get_structure(message);

            GstMessage* forward_msg = nullptr;

            gst_structure_get(s, "message", GST_TYPE_MESSAGE, &forward_msg, nullptr);

            //qInfo("%s",gst_structure_to_string(s));

            if (gst_structure_has_name(s, "GstBinForwarded"))
            {
                if (GST_MESSAGE_TYPE(forward_msg) == GST_MESSAGE_EOS)
                {
                    MainWindow* self = ((MainWindow*)user_data);

                    if (self->video_saver_ && GST_MESSAGE_SRC(forward_msg) == self->video_saver_->gst_pointer())
                    {
                        self->video_saver_->destroy_pipeline();
                        self->video_saver_ = nullptr;

                        self->statusBar()->showMessage("Saved video. ", 5000);
                        return TRUE;
                    }
                }
            }
            break;
        }
        case GST_MESSAGE_QOS:
        {
            // const GstStructure* s = gst_message_get_structure(message);
            // GstMessage* forward_msg = NULL;

            // gst_structure_get(s, "message", GST_TYPE_MESSAGE, &forward_msg, NULL);
            // qInfo("=================== %s", GST_OBJECT_NAME(GST_MESSAGE_SRC(forward_msg)));
            // gst_message_unref(forward_msg);

            break;
        }
        case GST_MESSAGE_LATENCY:
        case GST_MESSAGE_STREAM_START:
        case GST_MESSAGE_STREAM_STATUS:
        case GST_MESSAGE_STATE_CHANGED:
        case GST_MESSAGE_ASYNC_DONE:
        case GST_MESSAGE_NEW_CLOCK:
        { // ignore
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


GstPadProbeReturn MainWindow::pad_probe_callback (GstPad* /*pad*/,
                                                  GstPadProbeInfo* /*info*/,
                                                  gpointer user_data)
{
    MainWindow* self = (MainWindow*)user_data;

    self->ui->widget->layout()->removeWidget(self->p_trigger_info_label);
    self->p_trigger_info_label->deleteLater();

    return GST_PAD_PROBE_REMOVE;
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
            p_selected_caps = nullptr;
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


void MainWindow::open_pipeline(FormatHandling handling)
{
    std::string pipeline_string = m_config.pipeline.toStdString();
    GError* err = nullptr;

    bool set_device = false;

    if (p_pipeline)
    {
        GstState state;
        GstState pending;
        // wait 0.1 sec
        GstStateChangeReturn change_ret =
            gst_element_get_state(p_pipeline, &state, &pending, 100000000);

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
        g_object_set(G_OBJECT(p_pipeline), "message-forward", TRUE, nullptr);

        auto bus = gst_pipeline_get_bus(GST_PIPELINE(p_pipeline));
        [[maybe_unused]] auto gst_bus_id = gst_bus_add_watch(bus, bus_callback, this);
        gst_object_unref(bus);
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
            std::string serial = m_selected_device.serial_long();
            g_object_set(p_source, "serial", serial.c_str(), nullptr);
        }

        if (has_property(p_source, "conversion-element"))
        {
            qDebug("Setting 'conversion-element' property to '%s'",
                   conversion_element_to_string(m_config.conversion_element));
            g_object_set(p_source, "conversion-element", m_config.conversion_element, nullptr);
        }
    }

    // must not be freed
    GstElementFactory* factory = gst_element_get_factory(p_source);
    GType element_type = gst_element_factory_get_element_type(factory);

    auto src_change_ret = gst_element_set_state(p_source, GST_STATE_READY);

    if (src_change_ret == GST_STATE_CHANGE_ASYNC)
    {
        GstState state;
        GstState pending;
        // wait 0.1 sec
        GstStateChangeReturn change_ret =
            gst_element_get_state(p_source, &state, &pending, 100000000);

        if (change_ret == GST_STATE_CHANGE_SUCCESS)
        {
            if (state == GST_STATE_PAUSED || state == GST_STATE_PLAYING)
            {
                gst_element_set_state(p_source, GST_STATE_READY);
            }
        }
        else
        {
            qWarning("Unable to start pipeline. Stopping.");
            close_pipeline();
            // #TODO: what now?
            return;
        }
    }
    else if (src_change_ret == GST_STATE_CHANGE_FAILURE)
    {
        QString err_str = "Unable to open device. Please check gstreamer log 'tcam*:5' for more information.";
        qWarning("%s",err_str.toStdString().c_str());
        close_pipeline();

        QMessageBox::critical(this, "Unable to open device", err_str);
        return;
    }
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

        if (available_caps)
        {
            src_caps = gst_caps_from_string(available_caps);
            m_selected_device.set_caps(src_caps);
        }
    }
    else
    {
        GstPad* src_pad = gst_element_get_static_pad(tcamsrc, "src");

        src_caps = gst_pad_query_caps(src_pad, nullptr);

        gst_object_unref(src_pad);

        m_selected_device.set_caps(src_caps);

        gst_object_unref(tcamsrc);
    }

    GstCaps* caps = nullptr;
    if (handling == FormatHandling::Dialog)
    {
        p_selected_caps = open_format_dialog();

        if (!p_selected_caps)
        {
            close_pipeline();
            return;
        }

        caps = gst_caps_copy(p_selected_caps);
    }
    else if (handling == FormatHandling::Static)
    {
        caps = gst_caps_copy(p_selected_caps);
    }
    else
    {
        caps = Caps::get_default_caps(src_caps);
        p_selected_caps = gst_caps_copy(caps);
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

        if (!capsfilter)
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

    if (caps)
    {
        gst_caps_unref(caps);
        caps = nullptr;
    }

    p_displaysink = gst_bin_get_by_name(GST_BIN(p_pipeline), "sink");

    if (!p_displaysink)
    {
        qErrnoWarning("Unable to find sink element. Potentially unable to stream...");
    }
    else
    {
        if (strcmp(g_type_name(gst_element_factory_get_element_type(gst_element_get_factory(p_displaysink))),
                   "GstFPSDisplaySink") == 0)
        {
            GstElement* disp = nullptr;
            g_object_get(p_displaysink, "video-sink", &disp, nullptr);
            if (disp)
            {
                if (has_property(disp, "force-aspect-ratio"))
                {
                    g_object_set(disp, "force-aspect-ratio", true, nullptr);
                }

                if (has_property(disp, "widget"))
                {
                    g_object_set(disp, "widget", ui->widget, nullptr);
                }
                else if (has_property(disp, "video-sink"))
                {
                    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(disp), this->ui->widget->winId());
                }
                g_object_unref(disp);

                m_fps_signal_id = g_signal_connect(
                    p_displaysink, "fps-measurements", G_CALLBACK(&FPSCounter::fps_callback), &m_fps_counter);
            }
            else
            {
                SPDLOG_ERROR("Unable to retrieve video-sink from fpsdisplaysink. This will cause certain functionalities to not work.");
            }
        }

    }
    gst_element_set_state(p_pipeline, GST_STATE_PLAYING);

    connect(&m_fps_counter, &FPSCounter::new_fps_measurement, this, &MainWindow::fps_tick);
    m_fps_counter.start();

    enable_device_gui_elements(true);

    // do this last
    // Class queries elements automatically
    // at this point all properties have to be available
    m_tcam_collection = TcamCollection(GST_BIN(p_pipeline));

    // if the property dialog is open
    // refresh its content, so that format
    // dependent properties are correctly displayed
    if (p_property_dialog)
    {
        p_property_dialog->refresh();
    }

    if (m_tcam_collection.is_trigger_mode_active())
    {
        this->ui->widget->setAutoFillBackground(true);
        this->ui->widget->setStyleSheet("background-color:black;color:white;");

        QLayout* l = new QVBoxLayout();
        this->ui->widget->setLayout(l);
        p_trigger_info_label = new QLabel("No images received. TriggerMode is 'On'.");
        p_trigger_info_label->setStyleSheet("background-color:black;color:white;");
        l->addWidget(p_trigger_info_label);

        if (p_displaysink)
        {
            // we add a probe that informs us about incoming data
            // this way we know a buffer went through
            // and can remove any warnings about trigger mode
            // being active
            // this prevents any weird overlays of video display and info text
            GstPad* display_pad = gst_element_get_static_pad(p_displaysink, "sink");

            gst_pad_add_probe(display_pad,
                              GST_PAD_PROBE_TYPE_BUFFER,
                              pad_probe_callback,
                              this,
                              NULL);

            gst_object_unref(display_pad);
        }

    }
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

            p_about->set_device_caps("");
        }

        m_device_caps.clear();

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
        p_property_dialog->activateWindow();
        return;
    }

    if (!p_pipeline)
    {
        return;
    }

    p_property_dialog = new PropertyDialog(m_tcam_collection, this);
    p_property_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(p_property_dialog, &QDialog::finished, this, &MainWindow::free_property_dialog);
    connect(p_property_dialog, &PropertyDialog::device_lost, this, &MainWindow::device_lost);

    QString window_title = "tcam-capture - Properties - ";
    window_title += m_selected_device.model().c_str();
    window_title += " - ";
    window_title += m_selected_device.serial_long().c_str();
    p_property_dialog->setWindowTitle(window_title);
    p_property_dialog->setWindowIcon(QIcon(":/images/logo.png"));

    p_property_dialog->show();
}


void MainWindow::free_property_dialog (bool force_close)
{
    if (p_property_dialog && force_close)
    {
        p_property_dialog->close();
    }

    // this only exists to ensure that only one dialog instance can exist
    // deletion is taken care of by WA_DeleteOnClose
    p_property_dialog = nullptr;
}


void MainWindow::open_about_triggered()
{
    if (p_about)
    {
        p_about->update_state();
        // if closed -> display again
        p_about->show();
        // if not in focus -> put in foreground
        p_about->activateWindow();
        return;
    }

    p_about = new AboutDialog(m_config.pipeline, this);

    p_about->setWindowIcon(QIcon(":/images/logo.png"));

    if (p_source)
    {
        p_about->set_tcambin(p_source);
    }

    if (!m_device_caps.isEmpty())
    {
        p_about->set_device_caps(m_device_caps);
    }

    connect(this, &MainWindow::new_meta, p_about, &AboutDialog::update_meta);

    p_about->show();
}


void MainWindow::open_options_triggered()
{
    OptionsSettings settings;

    auto dia = OptionsDialog(m_config, settings, this);

    dia.setWindowIcon(QIcon(":/images/logo.png"));

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

    p_action_save_image->setEnabled(toggle);
    p_action_save_video->setEnabled(toggle);

    if (p_about)
    {
        p_about->set_tcambin(p_source);
    }

    QString window_title = "tcam-capture";
    if (toggle)
    {
        window_title += " - ";
        window_title += m_selected_device.model().c_str();
        window_title += " - ";
        window_title += m_selected_device.serial_long().c_str();
    }

    setWindowTitle(window_title);
}


void MainWindow::reset_fps_tick()
{
    p_fps_label->setText("");
}


void MainWindow::fps_tick(double new_fps)
{
    if (!p_displaysink)
    {
        return;
    }

    p_fps_label->setText("FPS: " + QString::number(new_fps));

    GstElement* sink = nullptr;

    if (has_property(p_displaysink, "video-sink"))
    {
        g_object_get(p_displaysink, "video-sink", &sink, nullptr);
    }
    else
    {
        sink = p_displaysink;
    }

    if (new_fps < 0)
    {
        if (has_property(sink, "stats"))
        {
            GstStructure* struc = nullptr;
            g_object_get(sink, "stats", &struc, nullptr);
            double rate;
            gst_structure_get_double(struc, "average-rate", &rate);
            p_fps_label->setText("FPS: " + QString::number(rate));

            //gst_object_unref(struc);
            gst_structure_free(struc);
            qInfo("stats %f", rate);
        }
    }


    GstSample* sample = nullptr;

    g_object_get(sink, "last-sample", &sample, nullptr);

    if (sample)
    {

        GstBuffer* buffer = gst_sample_get_buffer(sample);

        // if you need the associated caps
        // GstCaps* c = gst_sample_get_caps(sample);

        auto meta_api = g_type_from_name("TcamStatisticsMetaApi");

        if (meta_api == 0)
        {
            // this means TcamStatistics is not a registered type
            // tegrasrc or similar might cause this
            gst_sample_unref(sample);
            return;
        }

        GstMeta* meta = gst_buffer_get_meta(buffer, meta_api);

        if (meta)
        {
            GstStructure* struc = ((TcamStatisticsMeta*)meta)->structure;

            // will be freed by receiver
            emit new_meta(gst_structure_copy(struc));
        }
        else
        {
            static bool sample_warning_issued;
            if (!sample_warning_issued)
            {
                qWarning("No meta data available. This warning will only be issued once.");
                sample_warning_issued = true;
            }
        }

        gst_sample_unref(sample);
    }
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
    if (!message.isEmpty())
    {
        qWarning("%s", message.toStdString().c_str());
    }

    if (message != m_selected_device.serial().c_str())
    {
        return;
    }

    close_pipeline();

    free_property_dialog(true);

    m_selected_device = {};

    auto error_dialog = new QMessageBox(this);
    error_dialog->setCheckBox(nullptr);

    error_dialog->setIcon(QMessageBox::Critical);
    error_dialog->setWindowTitle("Device lost");
    QString s = "Device has been lost. Please reconnect or re-open it.\n\n";
    s += message;
    error_dialog->setText(s);

    error_dialog->exec();
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

    format_dialog.setWindowFlags(format_dialog.windowFlags() | Qt::Tool);

    auto layout = new QVBoxLayout();

    format_dialog.setLayout(layout);

    auto factory = gst_element_get_factory(p_source);

    // dependending on the pipeline we want to use a different element
    // tcambin will change GstQueries we send
    // always prefer tcamsrc when dealing with device caps
    GstElement* caps_element = p_source;
    bool must_free = false;
    if (strcmp("GstTcamBin", g_type_name(gst_element_factory_get_element_type (factory))) == 0)
    {
        caps_element = gst_bin_get_by_name(GST_BIN(p_source), "tcambin-source");
        must_free = true;
    }

    auto fmt_widget = new CapsWidget(Caps(m_selected_device.caps(), *caps_element));

    layout->addWidget(fmt_widget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, &format_dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &format_dialog, &QDialog::reject);

    layout->addWidget(buttonBox);

    QString window_title = "Caps - ";
    window_title += m_selected_device.model().c_str();
    window_title += " - ";
    window_title += m_selected_device.serial_long().c_str();
    format_dialog.setWindowTitle(window_title);
    format_dialog.setWindowIcon(QIcon(":/images/logo.png"));

    format_dialog.setMinimumSize(320, 240);
    format_dialog.setMaximumSize(640, 480);

    if (!m_device_caps.isEmpty())
    {
        GstCaps* c = gst_caps_from_string(m_device_caps.toStdString().c_str());
        fmt_widget->set_caps(c, *caps_element);
        gst_caps_unref(c);
    }

    if (must_free)
    {
        gst_object_unref(caps_element);
    }

    if (format_dialog.exec() == QDialog::Accepted)
    {
        GstCaps* new_caps = fmt_widget->get_caps();
        return new_caps;
    }
    return nullptr;
}


void MainWindow::open_format_triggered()
{
    auto caps = open_format_dialog();
    if (caps)
    {
        if (p_selected_caps)
        {
            gst_caps_unref(p_selected_caps);
            p_selected_caps = nullptr;
        }

        p_selected_caps = caps;
        open_pipeline(FormatHandling::Static);
    }
}


void MainWindow::save_image_triggered()
{

    GstElement* sink = p_displaysink;

    if (has_property(p_displaysink, "video-sink"))
    {
        sink = nullptr;
        g_object_get(p_displaysink, "video-sink", &sink, nullptr);
    }

    GstSample* sample = nullptr;
    /* Retrieve the buffer */

    g_object_get(sink, "last-sample", &sample, nullptr);

    if (sample)
    {

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            GstVideoInfo* video_info = gst_video_info_new();
            if (!gst_video_info_from_caps(video_info, gst_sample_get_caps(sample)))
            {
                // Could not parse video info (should not happen)
                g_warning("Failed to parse video info");
            }

            QImage image = QImage(info.data, video_info->width, video_info->height, QImage::Format::Format_ARGB32);

            // BMP - Windows Bitmap Read / write
            // GIF - Graphic Interchange Format(optional) Read
            // JPG - Joint Photographic Experts Group Read / write
            // JPEG - Joint Photographic Experts Group Read / write
            // PNG - Portable Network Graphics Read / write
            // PBM - Portable Bitmap Read
            // PGM - Portable Graymap Read PPM Portable Pixmap Read / write
            // XBM - X11 Bitmap Read / write
            // XPM

            const QString path = m_config.save_image_location;
            auto image_type = m_config.save_image_type;
            const QString extension = image_save_type_to_string(image_type).toLower();


            QString caps_str;
            if (p_selected_caps)
            {
                caps_str = tcam::tools::capture::caps_to_file_str(*p_selected_caps);
            }
            else
            {
                qInfo("No caps to interpret");
                caps_str = "";
            }

            auto fng = tcam::tools::capture::FileNameGenerator(
                m_selected_device.serial_long().c_str(), caps_str);

            fng.set_base_pattern(m_config.save_image_filename_structure);
            fng.set_file_extension(extension);

            auto name = fng.generate();

            if (image.save(path + "/" + name, extension.toStdString().c_str()))
            {
                // show message for 5 seconds
                statusBar()->showMessage("Saved image: " + path + "/" + name, 5000);
            }
            else
            {
                statusBar()->showMessage("ERROR! No image saved.", 5000);
            }

            gst_buffer_unmap(buffer, &info);
            gst_video_info_free(video_info);
        }

        gst_sample_unref(sample);
    }

    if (has_property(p_displaysink, "video-sink"))
    {
        g_object_unref(sink);
    }
}


void MainWindow::save_video_triggered()
{
    if (!video_saver_)
    {
        auto codec = m_config.save_video_type;

        auto caps_str = tcam::tools::capture::caps_to_file_str(*p_selected_caps);
        //
        auto fng = tcam::tools::capture::FileNameGenerator(m_selected_device.serial_long().c_str(), caps_str);
        fng.set_base_pattern(m_config.save_video_filename_structure);
        fng.set_file_extension(video_codec_file_extension(codec));

        QString name = m_config.save_video_location + "/" + fng.generate();
        video_saver_ = std::make_unique<tcam::tools::capture::VideoSaver>(GST_PIPELINE(p_pipeline), name, codec);

        video_saver_->start_saving();

        // Adjust GUI

        p_action_save_video->setText("Stop Recording");
        p_action_save_video->setIcon(QIcon(":/images/stop.png"));

        statusBar()->showMessage("Saving video: " + name);
    }
    else
    {
        qInfo("Stop saving");

        video_saver_->stop_saving();
        // do not delete saver
        // still need to wait on EOS
        //video_saver_ = nullptr;

        // Reset GUI

        p_action_save_video->setText("Start Recording");
        p_action_save_video->setIcon(QIcon(":/images/start_capture.png"));
    }
}
