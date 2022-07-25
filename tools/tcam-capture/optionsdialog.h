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

#pragma once

#include <QDialog>

#include "config.h"

namespace Ui
{
class OptionsDialog;
}
class QComboBox;

enum class OptionsConversionElement
{
    Auto,
    TcamConvert,
    TcamDutils,
    TcamDutilsCuda,
};

enum class MediaType
{
    Image,
    Video,
};


struct OptionsSettings
{
    OptionsConversionElement conversion_element = OptionsConversionElement::Auto;
    bool enable_restart_device = true;
};


class OptionsDialog : public QDialog
{

    Q_OBJECT

public:

    explicit OptionsDialog(TcamCaptureConfig& config, const OptionsSettings& settings, QWidget* parent = nullptr);
    ~OptionsDialog();

    TcamCaptureConfig get_config ();

public slots:

    void open_image_file_dialog ();
    void open_video_file_dialog ();
    void open_file_dialog (MediaType);

    void update_image_filename_preview (const QString&);
    void update_image_filename_extension_preview (const QString&);
    void update_video_filename_preview (const QString&);
    void update_video_filename_extension_preview (const QString&);

private:
    void enable_menu_entry (QComboBox* comboBox,
                            int index,
                            bool enabled);

    void setup_general();
    void setup_image();
    void setup_video();

    TcamCaptureConfig app_config;

    Ui::OptionsDialog* ui;
};
