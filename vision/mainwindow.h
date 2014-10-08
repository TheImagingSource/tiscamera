#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include "capturedeviceselectiondialog.h"
#include "propertywidget.h"

#include <tis.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow (QWidget *parent = 0);
    ~MainWindow();

    tis_imaging::Grabber* getGrabber ();

    unsigned char* data;
    size_t length;

signals:

    void newImage_received (std::shared_ptr<MemoryBuffer>);

private slots:

    void my_captureDevice_selected (tis_imaging::CaptureDevice);

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

private:

    Ui::MainWindow *ui;

    QImage* current_frame;
    QGraphicsScene* scene;
    QImage* img;

    QPixmap pixmap;

    QPainter painter;
    bool playing;

    tis_imaging::Grabber* grabber;
    std::shared_ptr<tis_imaging::ImageSink> sink;
    std::vector<tis_imaging::VideoFormatDescription> available_formats;

    tis_imaging::CaptureDevice open_device;

    VideoFormat active_format;

    CaptureDeviceSelectionDialog* selection_dialog;

    void reset_gui();

    void internal_callback (std::shared_ptr<MemoryBuffer>);

    static void callback (std::shared_ptr<MemoryBuffer>, void*);

    bool getActiveVideoFormat ();

    void start_stream ();

    void stop_stream ();
};

#endif // MAINWINDOW_H
