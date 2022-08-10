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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "capswidget.h"
#include "config.h"
#include "definitions.h"
#include "fpscounter.h"
#include "indexer.h"
#include "tcamcollection.h"
#include "videosaver.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <gst/gst.h>
#include <memory>

class PropertyDialog;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class AboutDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent* event) override;

    bool open_device(const QString& serial);

signals:

    void new_meta(GstStructure*);

public slots:

    void device_lost(const QString& message = "");
    void device_lost_cb(const Device& dev);

    void save_image_triggered();
    void save_video_triggered();

private slots:

    void on_actionOpen_Device_triggered();

    void on_actionQuit_triggered();

    void on_actionPlay_triggered();

    void on_actionStop_triggered();

    void on_widget_customContextMenuRequested(const QPoint& pos);

    void open_property_dialog();
    void free_property_dialog(bool force_close = false);

    void on_actionOpen_triggered();

    void open_format_triggered();

    void open_about_triggered();

    void open_options_triggered();

    void enable_device_gui_elements(bool toggle);

    void fps_tick(double);


private:
    static void init_device_dialog(MainWindow* instance)
    {
        instance->on_actionOpen_Device_triggered();
    }

    Ui::MainWindow* ui = nullptr;

    AboutDialog* p_about = nullptr;

    TcamCaptureConfig m_config;

    QToolBar* p_toolbar = nullptr;
    QAction* p_action_save_video = nullptr;
    QAction* p_action_save_image = nullptr;

    QAction* p_action_property_dialog = nullptr;
    QAction* p_action_format_dialog = nullptr;

    std::shared_ptr<Indexer> m_index;
    Device m_selected_device;
    PropertyDialog* p_property_dialog = nullptr;
    GstElement* p_pipeline = nullptr;

    TcamCollection m_tcam_collection;
    GstElement* p_source = nullptr;
    GstElement* p_displaysink = nullptr;
    gulong m_fps_signal_id = 0;

    QLabel* p_fps_label = nullptr;
    QLabel* p_trigger_info_label = nullptr;

    FPSCounter m_fps_counter;

    std::unique_ptr<tcam::tools::capture::VideoSaver> video_saver_ = nullptr;

    static gboolean bus_callback(GstBus* /*bus*/, GstMessage* message, gpointer user_data);
    static GstPadProbeReturn pad_probe_callback(GstPad* pad,
                                                GstPadProbeInfo* info,
                                                gpointer user_data);

    void reset_fps_tick();
    GstCaps* open_format_dialog();

    void open_pipeline(FormatHandling);
    void close_pipeline();

    void load_settings();
    void save_settings();

    QTimer* p_fps_timer = nullptr;

    GstCaps* p_selected_caps = nullptr;
    QString m_device_caps;
};
#endif // MAINWINDOW_H
