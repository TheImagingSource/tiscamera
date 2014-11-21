#ifndef CAPTUREDEVICESELECTIONDIALOG_H
#define CAPTUREDEVICESELECTIONDIALOG_H

#include <QDialog>

#include <tcam.h>

#include <thread>

using namespace tcam;

namespace Ui {
class DeviceInfoSelectionDialog;
}

class DeviceInfoSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceInfoSelectionDialog (QWidget *parent = 0);
    ~DeviceInfoSelectionDialog ();

    tcam::DeviceInfo getSelection ();

    void update_list ();

signals:
    void device_selected (tcam::DeviceInfo);

private slots:
    void on_buttonBox_accepted ();

    void on_buttonBox_rejected ();

private:
    Ui::DeviceInfoSelectionDialog *ui;

    std::vector<tcam::DeviceInfo> devices;
    std::shared_ptr<tcam::DeviceIndex> device_watch_dog;
    bool run_thread;
    std::thread work_thread;
};

#endif // CAPTUREDEVICESELECTIONDIALOG_H
