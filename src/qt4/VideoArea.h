#ifndef TCAM_QT4_VIDEOAREA_H
#define TCAM_QT4_VIDEOAREA_H

#include <QWidget>
#include <QTreeWidget>

#include <tcam.h>

namespace Ui {
class VideoArea;
}


class VideoArea : public QWidget
{
    Q_OBJECT

public:
    explicit VideoArea (QWidget* parent = 0, tcam::DeviceInfo* info = 0);
    ~VideoArea ();

    QPixmap m;
    bool new_image;

public slots:

    void start();

    void stop();

    bool setVideoFormat(const tcam::VideoFormat&);

    tcam::VideoFormat getVideoFormat () const;

    std::vector<tcam::VideoFormatDescription> getAvailableVideoFormats () const;

    QTreeWidget* get_property_tree (QWidget* p);

protected:

    void paintEvent (QPaintEvent* e);
    void resizeEvent (QResizeEvent* event);

private:

    static void callback (tcam::MemoryBuffer* buffer, void* user_data);
    void internal_callback (tcam::MemoryBuffer*);

    Ui::VideoArea* ui;

    std::shared_ptr<tcam::CaptureDevice> device;
    std::shared_ptr<tcam::ImageSink> sink;

    bool received_new_image;
    bool playing;
};


#endif // TCAM_QT4_VIDEOAREA_H
