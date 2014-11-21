#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include <QLabel>

#include "capturedeviceselectiondialog.h"
#include "propertywidget.h"

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

    tcam::CaptureDevice* getCaptureDevice ();

signals:

    void newImage_received (MemoryBuffer*);

private slots:

    void my_captureDevice_selected (tcam::DeviceInfo);

    // create selection dialog
    void on_actionOpen_Camera_triggered ();

    // quit whole application
    void on_actionQuit_triggered ();

    void on_actionPlay_Pause_triggered ();

    void my_newImage_received (std::shared_ptr<MemoryBuffer>);

    void property_changed (PropertyWidget*);

    void on_format_box_currentIndexChanged (int index);

    void on_actionClose_Camera_triggered ();

    void on_size_box_currentIndexChanged(int index);

    void update_properties ();

private:

    Ui::MainWindow *ui;

    QImage* current_frame;
    QGraphicsScene* scene;
    QImage* img;

    QPixmap pixmap;

    QPainter painter;
    bool playing;

    tcam::CaptureDevice* grabber;
    std::shared_ptr<tcam::ImageSink> sink;
    std::vector<tcam::VideoFormatDescription> available_formats;

    tcam::DeviceInfo open_device;

    VideoFormat active_format;

    DeviceInfoSelectionDialog* selection_dialog;

    void reset_gui ();

    void internal_callback (MemoryBuffer*);

    static void callback (MemoryBuffer*, void*);

    bool getActiveVideoFormat ();

    void start_stream ();

    void stop_stream ();

    QLabel* status_label;
};

#endif // MAINWINDOW_H
