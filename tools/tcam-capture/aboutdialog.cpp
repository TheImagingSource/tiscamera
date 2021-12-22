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
#include <QFormLayout>
#include <QApplication>
#include <QClipboard>


namespace {

struct meta_entry
{
    QString name;
    QLabel* value;
};

class MetaWidget : public QWidget
{
public:

    MetaWidget(QWidget*parent=nullptr)
        : QWidget(parent)
    {
        setLayout(new QFormLayout());
    };

    ~MetaWidget()
    {}

    void update(GstStructure& struc)
    {

        auto meta_cb = [] (GQuark field_id,
                           const GValue* value,
                           gpointer user_data) -> gboolean
        {

            auto fill_label = [] (QLabel* label, const GValue* gvalue)
            {
                if (G_VALUE_TYPE(gvalue) == G_TYPE_BOOLEAN)
                {
                    gboolean val = g_value_get_boolean(gvalue);
                    if (val)
                    {
                        label->setText("true");
                    }
                    else
                    {
                        label->setText("false");
                    }
                }
                else if (G_VALUE_TYPE(gvalue) == G_TYPE_DOUBLE)
                {
                    double val = g_value_get_double(gvalue);
                    label->setText(QString::number(val));

                }
                else if (G_VALUE_TYPE(gvalue) == G_TYPE_UINT64)
                {
                    guint64 val = g_value_get_uint64(gvalue);
                    label->setText(QString::number(val));
                }
                else
                {
                    qWarning("value type not implemented for TcamMeta\n");
                }
            };

            MetaWidget* self  = (MetaWidget*)user_data;

            QString name = g_quark_to_string(field_id);

            auto iter = std::find_if(self->m_entries.begin(), self->m_entries.end(),
                                     [&name] (const meta_entry& entry)
                                     {
                                         if (entry.name == name)
                                         {
                                             return true;
                                         }
                                         return false;
                                     });

            if (iter == self->m_entries.end())
            {
                meta_entry e = {name, new QLabel()};
                e.value->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
                self->m_entries.push_back(e);

                dynamic_cast<QFormLayout*>(self->layout())->addRow(name, e.value);
                fill_label(e.value, value);
            }
            else
            {
                fill_label(iter->value, value);
            }

            return TRUE;
        };

        gst_structure_foreach(&struc, meta_cb, this);

    }

private:

    std::vector<meta_entry> m_entries;
};

} // namespace

AboutDialog::AboutDialog(const QString& pipeline_str, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    m_pipeline_str(pipeline_str)
{
    ui->setupUi(this);

    ui->tab_versions->layout()->setAlignment(Qt::AlignTop);

    fill_stream();
    fill_versions();
    fill_state();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}


void AboutDialog::fill_stream()
{
    auto layout = dynamic_cast<QFormLayout*>(ui->tab_stream->layout());

    auto tmp = m_pipeline_str;

    tmp.replace("!", "<br>!");

    auto pipe_label = new QLabel(tmp);

    layout->addRow("Pipeline: ", pipe_label);

    if (!p_label_device_caps)
    {
        p_label_device_caps = new QLabel();
    }

    layout->addRow("Device caps: ", p_label_device_caps);

    p_meta = new MetaWidget();

    layout->addRow("GstMeta:", p_meta);

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


void AboutDialog::set_device_caps(const QString& dev_caps)
{
    p_label_device_caps->setText(dev_caps);
}


void AboutDialog::set_tcambin(GstElement* bin)
{
    p_tcambin = bin;

    update_state();
}


void AboutDialog::update_meta(GstStructure* meta)
{
    MetaWidget* m = (MetaWidget*)p_meta;

    m->update(*meta);

    // has to free here
    gst_structure_free(meta);
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
