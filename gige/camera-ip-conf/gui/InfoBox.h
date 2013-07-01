///
/// @file InfoBox.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

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
