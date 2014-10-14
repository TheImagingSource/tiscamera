#ifndef CAPTUREDISPLAY_H
#define CAPTUREDISPLAY_H

#include <QWidget>

namespace Ui {
class CaptureDisplay;
}

class CaptureDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureDisplay (QWidget* parent = 0);
    ~CaptureDisplay ();

    QPixmap m;
    bool new_image;

protected:

    void paintEvent (QPaintEvent* e);
    void resizeEvent (QResizeEvent* event);

private:
    Ui::CaptureDisplay* ui;
};

#endif // CAPTUREDISPLAY_H
