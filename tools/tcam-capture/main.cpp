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

#include "../src/version.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <gst/gst.h>

int main(int argc, char* argv[])
{
    // always init gstreamer
    gst_init(&argc, &argv);

    // make logging useful
    qSetMessagePattern("%{time} - %{file}:%{line}: %{type} %{message}");

    QApplication a(argc, argv);

    a.setOrganizationName("the_imaging_source");
    a.setOrganizationDomain("theimagingsource.com");
    a.setApplicationName("tcam-capture");

    a.setApplicationVersion(get_version());

    QCommandLineParser parser;
    parser.setApplicationDescription("The Imaging Source Live Stream Application");
    parser.addHelpOption();
    parser.addVersionOption();

    // QCommandLineOption reset_option("reset", "Reset application settings and clear cache");
    // parser.addOption(reset_option);

    QCommandLineOption serial_option("serial", "Open device on startup", "serial");
    parser.addOption(serial_option);

    // Process the actual command line arguments given by the user
    parser.process(a);

    QString serial = parser.value(serial_option);

    MainWindow w;

    w.show();

    if (serial.isEmpty())
    {
        QTimer::singleShot(200, &w, SLOT(on_actionOpen_Device_triggered()));
    }
    else
    {
        // this will block the GUI
        // should the camera need a long time to open
        if (!w.open_device(serial))
        {
            return 1;
        }
    }
    return a.exec();
}
