#ifndef showcamera_H
#define showcamera_H

#include <QtGui/QMainWindow>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QtGui>

#include <gst/gst.h>
#include <glib.h>

#include "CameraList.h"
#include "cpropertiesdialog.h"
 
typedef struct 
{	
  QLabel *TestLabel;
  int ImageBrightness;
  int cbCount;
  bool SaveImage;
} _AppData;

////////////////////////////////////////////////////
class MainLoopThread : public QThread
{
  public  :
    GMainLoop* loop;
  private:
    void run()
    {
      g_main_loop_run(loop);
      qDebug()<<"End thread";
    }
};


class showcamera : public QMainWindow
{
  Q_OBJECT
  public:
      showcamera();
      virtual ~showcamera();
      
  private slots:
      void startcamera();
      void stopcamera();
      void OnSnapButton();
      void OncboCameraChanced(int Index);
      void OncboCamerasChanced(int Index);
      void OnPropertiesButton();
      void OnTimer();

  private:
      QWidget *window;
      
      QGridLayout *mainLayout;
      QWidget *videowindow;
      QWidget *videowindow2;
      QPushButton *SnapButton;
      QPushButton *StartButton;
      QPushButton *StopButton;
      QComboBox *cboCameras;
      QComboBox *cboCamera;
      QVBoxLayout *Cameralayout;
      QHBoxLayout *Videolayout;
      QLabel *TestLabel;
      QPushButton *PropertiesButton;
      QProgressBar *BrightnessBar;
      QTimer _Timer;
      QLabel *FrameCounts;
      
      MainLoopThread _MainLoopThread;
      
      CCameraList _CameraList;
      CCamera *_cCamera;

      
      _AppData AppData;
     

      void FindCameras();
      void GetDeviceProperties(const char *UniqueName);
      void CreateUI();
      CPropertiesDialog *_cPropertiesDialog;
      
};

#endif // showcamera_H
