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
#include "gst/gst.h"
#include "indexer.h"
#include "propertydialog.h"
#include "tcamcollection.h"
#include "fpscounter.h"
#include "config.h"
#include "definitions.h"

#include <QMainWindow>
#include <QSettings>
#include <QToolBar>
#include <QTimer>
#include <QCloseEvent>

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

    void closeEvent(QCloseEvent *event);

    void set_settings_string(const QString str);

public slots:

    void device_lost(const QString& message="");
    void device_lost_cb(const Device& dev);

private slots:

    void on_actionOpen_Device_triggered();

    void on_actionQuit_triggered();

    void on_actionPlay_triggered();

    void on_actionStop_triggered();

    void on_widget_customContextMenuRequested(const QPoint& pos);

    void open_property_dialog();
    void free_property_dialog();

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

    Ui::MainWindow* ui;

    AboutDialog* p_about = nullptr;

    TcamCaptureConfig m_config;

    QToolBar* p_toolbar = nullptr;

    QAction* p_action_property_dialog = nullptr;
    QAction* p_action_format_dialog = nullptr;

    std::shared_ptr<Indexer> m_index;
    Device m_selected_device;
    PropertyDialog* p_property_dialog;
    GstElement* p_pipeline;

    TcamCollection m_tcam_collection;
    GstElement* p_source = nullptr;
    GstElement* p_displaysink = nullptr;
    gulong m_fps_signal_id;

    QLabel* p_fps_label = nullptr;
    QLabel* p_settings_label = nullptr;

    FPSCounter m_fps_counter;

    void reset_fps_tick();
    GstCaps* open_format_dialog();

    void open_pipeline(FormatHandling);
    void close_pipeline();

    void load_settings();
    void save_settings();

    QTimer* p_fps_timer = nullptr;

    int m_gst_bus_id = -1;
    bool m_use_dutils = true;

    GstCaps* p_selected_caps = nullptr;
};
#endif // MAINWINDOW_H
