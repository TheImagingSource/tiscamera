#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include <QLabel>

#include <tcam_qt4.h>

#include <tcam.h>

using namespace tcam;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow (QWidget *parent = 0);
    ~MainWindow();

private slots:

    void my_captureDevice_selected (tcam::DeviceInfo);

    // create selection dialog
    void on_actionOpen_Camera_triggered ();

    // quit whole application
    void on_actionQuit_triggered ();

    void on_actionPlay_Pause_triggered ();

    void on_format_box_currentIndexChanged (int index);

    void on_actionClose_Camera_triggered ();

    void on_size_box_currentIndexChanged(int index);

    void update_properties ();

    void on_actionContact_triggered ();

private:

    Ui::MainWindow *ui;

    bool playing;

    std::vector<tcam::VideoFormatDescription> available_formats;

    tcam::DeviceInfo open_device;

    VideoFormat active_format;

    DeviceSelectionDialog* selection_dialog;

    VideoArea* area;

    void reset_gui ();

    bool getActiveVideoFormat ();

    void start_stream ();

    void stop_stream ();

    QLabel* status_label;
};

#endif // MAINWINDOW_H
