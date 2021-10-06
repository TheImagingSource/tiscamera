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

OptionsDialog::OptionsDialog(TcamCaptureConfig& config, const OptionsSettings& settings, QWidget* parent)
    : QDialog(parent), ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    ui->check_dutils->setEnabled(settings.enable_dutils);
    ui->check_dutils->setChecked(config.use_dutils);

    // QObject::connect(ui->buttonBox->a acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    // QObject::connect(ui->rejectButton, SIGNAL(clicked()), this, SLOT(reject()));

}


OptionsDialog::~OptionsDialog()
{}


TcamCaptureConfig OptionsDialog::get_config () const
{
    TcamCaptureConfig conf = {};

    if (ui->check_dutils->isEnabled())
    {
        conf.use_dutils = ui->check_dutils->isChecked();
    }
    return conf;

}
