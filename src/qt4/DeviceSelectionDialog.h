#ifndef DEVICESELECTIONDIALOG_H
#define DEVICESELECTIONDIALOG_H

#include <QDialog>

#include <tcam.h>

#include <thread>

using namespace tcam;

namespace Ui {
class DeviceSelectionDialog;
}

class DeviceSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSelectionDialog (QWidget* parent = 0);
    ~DeviceSelectionDialog ();

    tcam::DeviceInfo getSelection ();

    void update_list ();

signals:
    void device_selected (tcam::DeviceInfo);

private slots:
    void on_buttonBox_accepted ();

    void on_buttonBox_rejected ();

private:
    Ui::DeviceSelectionDialog* ui;

    std::vector<tcam::DeviceInfo> devices;
    std::shared_ptr<tcam::DeviceIndex> device_watch_dog;
    bool run_thread;
    std::thread work_thread;
};

#endif // DEVICESELECTIONDIALOG_H
