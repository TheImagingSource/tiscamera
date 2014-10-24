#ifndef CAPTUREDEVICESELECTIONDIALOG_H
#define CAPTUREDEVICESELECTIONDIALOG_H

#include <QDialog>

#include <tis.h>

#include <thread>

namespace Ui {
class CaptureDeviceSelectionDialog;
}

class CaptureDeviceSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureDeviceSelectionDialog (QWidget *parent = 0);
    ~CaptureDeviceSelectionDialog ();

    tis_imaging::CaptureDevice getSelection ();

    void update_list ();

signals:
    void device_selected (tis_imaging::CaptureDevice);

private slots:
    void on_buttonBox_accepted ();

    void on_buttonBox_rejected ();

private:
    Ui::CaptureDeviceSelectionDialog *ui;

    std::vector<tis_imaging::CaptureDevice> devices;
    std::shared_ptr<tis_imaging::DeviceIndex> device_watch_dog;
    bool run_thread;
    std::thread work_thread;
};

#endif // CAPTUREDEVICESELECTIONDIALOG_H
