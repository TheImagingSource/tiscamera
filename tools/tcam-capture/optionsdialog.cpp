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

#include "ui_optionsdialog.h"

#include <QStandardItemModel>
#include <gst/gst.h>

OptionsDialog::OptionsDialog(TcamCaptureConfig& config,
                             const OptionsSettings& /*settings*/,
                             QWidget* parent)
    : QDialog(parent), app_config(config), ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::Tool);

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

    int active_index = (int)config.conversion_element;

    // these should be in the same order as the enum class
    // that makes get/set easier
    ui->combo_convert_options->addItem("auto");
    ui->combo_convert_options->addItem("tcamconvert");

    // do not test tcamconvert
    // it is part of tiscamera and always available

    ui->combo_convert_options->addItem("tcamdutils");

    if (!verify_entry("tcamdutils", 2) && (int)config.conversion_element == 2)
    {
        // reset active index to `auto`
        active_index = 0;
    }

    ui->combo_convert_options->addItem("tcamdutils-cuda");

    if (!verify_entry("tcamdutils-cuda", 3) && (int)config.conversion_element == 3)
    {
        active_index = 0;
    }

    ui->combo_convert_options->setCurrentIndex(active_index);
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


TcamCaptureConfig OptionsDialog::get_config()
{
    app_config.conversion_element = (ConversionElement)ui->combo_convert_options->currentIndex();

    return app_config;
}
