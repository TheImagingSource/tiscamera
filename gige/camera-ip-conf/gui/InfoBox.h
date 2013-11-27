/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

#ifndef INFOBOX_H
#define INFOBOX_H

#include <QDialog>

namespace Ui {
class InfoBox;
}

class InfoBox : public QDialog
{
    Q_OBJECT

public:
    explicit InfoBox(QWidget *parent = 0);
    ~InfoBox();

    void showMessage (QString& message);

public slots:
    void hideBox();

private:
    Ui::InfoBox *ui;
};

#endif // INFOBOX_H
