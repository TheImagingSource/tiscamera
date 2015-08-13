#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QDialog>

#include <tcam.h>

using namespace tcam;

namespace Ui {
class PropertyDialog;
}

class PropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(QWidget *parent = 0);
    explicit PropertyDialog(QWidget *parent = 0, CaptureDevice* = 0);
    ~PropertyDialog();

private:

    void populate_dialog ();

    Ui::PropertyDialog *ui;

    CaptureDevice* device;

    std::vector<Property*> properties;
};

#endif // PROPERTYDIALOG_H
