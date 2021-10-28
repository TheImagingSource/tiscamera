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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QProcess>
#include <QLabel>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->tab_versions->layout()->setAlignment(Qt::AlignTop);

    fill_versions();
    fill_state();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}


void AboutDialog::fill_versions()
{
    QProcess process_tis_version;
    process_tis_version.start("tcam-ctrl", {"--version"});
    process_tis_version.waitForFinished(-1); // will wait forever until finished

    QString stdout_tis = process_tis_version.readAllStandardOutput();
    QString stderr = process_tis_version.readAllStandardError();

    QProcess process_tis_packages;
    process_tis_packages.start("tcam-ctrl", {"--packages"});
    process_tis_packages.waitForFinished(-1); // will wait forever until finished

    QString stdout_packages = process_tis_packages.readAllStandardOutput();

    // we work with a rich text label
    // this means a 'normal' string
    // does not suffice. Instead use html tags.
    stdout_tis.replace("\n", "<br>");
    stdout_packages.replace("\n", "<br>");
    stdout_packages.replace("\t", "&nbsp;");

    QString s = "<h3>Tiscamera:</h3>"
        + stdout_tis
        + "<h3>Packages:</h3>"
        + stdout_packages;

    ui->label_versions->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    ui->label_versions->setText(s);
}


void AboutDialog::fill_state()
{
    auto apply_button = ui->buttonBox_2->button(QDialogButtonBox::Apply);
    auto reset_button = ui->buttonBox_2->button(QDialogButtonBox::Reset);

    connect(apply_button, &QPushButton::clicked, this, &AboutDialog::write_state);
    connect(reset_button, &QPushButton::clicked, this, &AboutDialog::update_state);
}


void AboutDialog::set_tcambin(GstElement* bin)
{
    p_tcambin = bin;

    update_state();
}


void AboutDialog::update_state()
{
    if (!p_tcambin)
    {
        ui->state_field->setPlainText("");
        ui->state_field->setEnabled(false);
        return;
    }

    GValue state = G_VALUE_INIT;
    g_object_get_property(G_OBJECT(p_tcambin), "tcam-properties-json", &state);

    ui->state_field->setPlainText(g_value_get_string(&state));

    ui->state_field->setEnabled(true);
}


void AboutDialog::write_state()
{
    auto str = ui->state_field->toPlainText();

    g_object_set(p_tcambin, "tcam-properties-json", str.toStdString().c_str(), nullptr);

}
