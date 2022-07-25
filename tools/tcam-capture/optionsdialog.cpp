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

#include "optionsdialog.h"

#include "config.h"
#include "filename_generator.h"
#include "ui_optionsdialog.h"
#include "videosaver.h"

#include <QStandardItemModel>
#include <gst/gst.h>
#include <qfiledialog.h>
#include <qobjectdefs.h>
#include <fstream>


OptionsDialog::OptionsDialog(TcamCaptureConfig& config,
                             const OptionsSettings& /*settings*/,
                             QWidget* parent)
    : QDialog(parent), app_config(config), ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::Tool);

    setup_general();

    setup_image();

    setup_video();
}


void OptionsDialog::setup_general()
{


    // helper, checks if element really exists
    // disables entry otherwise
    auto verify_entry = [this](const QString& name, int index) -> bool
    {
        auto factory = gst_element_factory_find(name.toStdString().c_str());

        if (factory)
        {
            gst_object_unref(factory);
            this->enable_menu_entry(this->ui->combo_convert_options, index, true);
            return true;
        }
        else
        {
            this->enable_menu_entry(this->ui->combo_convert_options, index, false);
            return false;
        }
    };

    int active_index = (int)app_config.conversion_element;

    // these should be in the same order as the enum class
    // that makes get/set easier
    ui->combo_convert_options->addItem("auto");
    ui->combo_convert_options->addItem("tcamconvert");

    // do not test tcamconvert
    // it is part of tiscamera and always available

    ui->combo_convert_options->addItem("tcamdutils");

    if (!verify_entry("tcamdutils", 2) && (int)app_config.conversion_element == 2)
    {
        // reset active index to `auto`
        active_index = 0;
    }

    ui->combo_convert_options->addItem("tcamdutils-cuda");

    if (!verify_entry("tcamdutils-cuda", 3) && (int)app_config.conversion_element == 3)
    {
        active_index = 0;
    }

    ui->combo_convert_options->setCurrentIndex(active_index);
}

void OptionsDialog::setup_image()
{


    auto image_type_entries = get_image_save_type_names();

    int active_image_type = (int)app_config.save_image_type;
    for (const auto& entry : image_type_entries) { ui->saveImageAsComboBox->addItem(entry); }
    ui->saveImageAsComboBox->setCurrentIndex(active_image_type);

    // save image stuff

    connect(ui->openImageSaveFileDialogButton,
            SIGNAL(clicked()),
            this,
            SLOT(open_image_file_dialog()));

    ui->saveImageLocationEditLine->setText(app_config.save_image_location);

    ui->saveImageFilenameEditLine->setText(app_config.save_image_filename_structure);
    // update once manually so that everything displays correctly
    update_image_filename_preview(app_config.save_image_filename_structure);

    connect(ui->saveImageFilenameEditLine,
            SIGNAL(textChanged(const QString&)),
            this,
            SLOT(update_image_filename_preview(const QString&)));

    ui->saveImageInfoLabel->setText(tcam::tools::capture::FileNameGenerator::get_help_text());

    connect(ui->saveImageAsComboBox,
            SIGNAL(currentIndexChanged(const QString&)),
            this,
            SLOT(update_image_filename_extension_preview(const QString&)));
}


void OptionsDialog::setup_video()
{
    auto codec_entries = get_video_codec_names();

    int active_codec = (int)app_config.save_video_type;
    for (const auto& codec : codec_entries)
    {
        ui->saveVideoAsComboBox->addItem(codec);
    }
    ui->saveVideoAsComboBox->setCurrentIndex(active_codec);

    ui->saveVideoFilenameEditLine->setText(app_config.save_video_filename_structure);
    update_video_filename_preview(app_config.save_video_filename_structure);

    connect(ui->saveVideoFilenameEditLine,
            SIGNAL(textChanged(const QString&)),
            this,
            SLOT(update_video_filename_preview(const QString&)));
    connect(ui->openVideoSaveFileDialogButton, SIGNAL(clicked()),
            this, SLOT(open_video_file_dialog()));
    ui->saveVideoLocationEditLine->setText(app_config.save_video_location);

    ui->saveVideoInfoLabel->setText(tcam::tools::capture::FileNameGenerator::get_help_text());

    connect(ui->saveVideoAsComboBox,
            SIGNAL(currentIndexChanged(const QString&)),
            this,
            SLOT(update_video_filename_extension_preview(const QString&)));

}


OptionsDialog::~OptionsDialog() {}


void OptionsDialog::enable_menu_entry(QComboBox* comboBox, int index, bool enabled)
{
    auto* model = qobject_cast<QStandardItemModel*>(comboBox->model());

    if (!model)
    {
        return;
    }

    auto* item = model->item(index);

    if (!item)
    {
        return;
    }
    item->setEnabled(enabled);
}


void OptionsDialog::open_video_file_dialog()
{
    open_file_dialog(MediaType::Video);
}


void OptionsDialog::open_image_file_dialog()
{
    open_file_dialog(MediaType::Image);
}


void OptionsDialog::open_file_dialog(MediaType type)
{
    QString old_dir;

    if (type == MediaType::Image)
    {
        old_dir = app_config.save_image_location;
    }
    else
    {
        old_dir = app_config.save_video_location;
    }

    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Save Directory"),
                                                    old_dir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);


    if (type == MediaType::Image)
    {
        app_config.save_image_location = dir;
        ui->saveImageLocationEditLine->setText(dir);
    }
    else
    {
        app_config.save_video_location = dir;
        ui->saveVideoLocationEditLine->setText(dir);
    }
}


void OptionsDialog::update_image_filename_preview(const QString& new_text)
{
    qInfo("Current text: %s", new_text.toStdString().c_str());

    auto fng = tcam::tools::capture::FileNameGenerator("", "");

    fng.set_file_extension(image_save_type_to_string(
                               (ImageSaveType)ui->saveImageAsComboBox->currentIndex())
                               .toLower());

    ui->saveImageFilenamePreviewEditLine->setText(fng.preview(new_text));

    // auto fileExists = [](const char* fileName)
    // {
    //     std::ifstream test(fileName);
    //     return (test) ? true : false;
    // };

    // TODO: verify filename legality

    // if (!fileExists())
    // {

    // }
}


void OptionsDialog::update_image_filename_extension_preview(const QString& new_extension)
{

    auto fng = tcam::tools::capture::FileNameGenerator("", "");

    fng.set_file_extension(new_extension);

    ui->saveImageFilenamePreviewEditLine->setText(
        fng.preview(ui->saveImageFilenameEditLine->text()));
}


void OptionsDialog::update_video_filename_preview(const QString& new_text)
{
    auto fng = tcam::tools::capture::FileNameGenerator("", "");

    fng.set_file_extension(video_codec_file_extension(
                               (VideoCodec)ui->saveVideoAsComboBox->currentIndex())
                               .toLower());

    ui->saveVideoFilenamePreviewEditLine->setText(fng.preview(new_text));

}

void OptionsDialog::update_video_filename_extension_preview(const QString& new_extension)
{
    auto fng = tcam::tools::capture::FileNameGenerator("", "");

    fng.set_file_extension(video_codec_file_extension(new_extension));

    ui->saveImageFilenamePreviewEditLine->setText(fng.preview(ui->saveVideoFilenameEditLine->text()));
}


TcamCaptureConfig OptionsDialog::get_config()
{
    app_config.conversion_element = (ConversionElement)ui->combo_convert_options->currentIndex();

    // save image settings
    app_config.save_image_type = (ImageSaveType)ui->saveImageAsComboBox->currentIndex();
    app_config.save_video_location = ui->saveImageLocationEditLine->text();
    app_config.save_image_filename_structure = ui->saveImageFilenameEditLine->text();

    // save video settings

    app_config.save_video_type = (VideoCodec)ui->saveVideoAsComboBox->currentIndex();
    app_config.save_video_location = ui->saveVideoLocationEditLine->text();
    app_config.save_video_filename_structure = ui->saveImageFilenameEditLine->text();

    return app_config;
}
