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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>

#include <gst/gst.h>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(const QString& pipeline_str, QWidget *parent = nullptr);
    ~AboutDialog();

    void set_tcambin(GstElement* bin);
    void set_device_caps(const QString&);

public slots:

    void update_meta(GstStructure* meta);
    void update_state();

private slots:

    void write_state();

    void open_state();
    void save_state();

private:

    void fill_stream();
    void fill_versions();
    void fill_state();

    Ui::AboutDialog *ui;

    GstElement* p_tcambin = nullptr;
    QString m_pipeline_str;
    QLabel* p_label_device_caps = nullptr;
    QWidget* p_meta = nullptr;
};

#endif // ABOUTDIALOG_H
