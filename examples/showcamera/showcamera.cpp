#include "showcamera.h"
#include "config.h"
#include <gst/gst.h>
#include <glib.h>
#include <gst/interfaces/xoverlay.h>
//#include <io.h>
#include <stdio.h>
#include <stdlib.h> 
#include <dirent.h> 
#include <sys/ioctl.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/mman.h> 
#include <sys/time.h>
#include <fcntl.h> 
#include <unistd.h>
#include <linux/videodev2.h>


char CAMERA[256];
#define FORMAT_MONO "video/x-raw-gray" 
#define FORMAT_COLOR "video/x-raw-bayer"
#define PATTERN "grgb"

///////////////////////////////////////////////////////////////////////////////
/* This callback will be registered to the image sink
 * after user requests a photo */
static gboolean buffer_probe_callback(
		GstElement *image_sink,
		GstBuffer *buffer, GstPad *pad, void* _data)
{
  char Text[200];
  gint width, height, bpp;
  const GstStructure *str;
  GstCaps* pad_caps;
  QImage::Format Format;
  _AppData *appdata = (_AppData*)_data;
    
  pad_caps = gst_pad_get_negotiated_caps( pad );
  
  if( pad_caps == NULL )
  {
    g_print("pad_caps = 0" );
    return TRUE;
  }

  str = gst_caps_get_structure (pad_caps, 0);
  gst_structure_get_int (str, "bpp", &bpp);

  
  if( appdata->SaveImage == TRUE ) // Save an image on demand
  {
      
      str = gst_caps_get_structure (pad_caps, 0);
      //g_print("caps: %s\n", gst_caps_to_string(pad_caps)); 
      
      if (!gst_structure_get_int (str, "width", &width) ||
	  !gst_structure_get_int (str, "height", &height) ||
	  !gst_structure_get_int (str, "bpp", &bpp)) 
      {
	g_print ("No width/height/bpp available\n");
	gst_caps_unref(pad_caps);
	return TRUE;
      }
      gst_caps_unref(pad_caps);
    
      switch( bpp )
      {
	case 8:
	  Format = QImage::Format_Indexed8; // Not correct, since we must create the index!
	  break;

	case 16:
	  Format = QImage::Format_RGB555;
	  break;

	case 24:
	  Format = QImage::Format_RGB888;
	  break;

	case 32:
	  Format = QImage::Format_RGB32;
	  break;
	  
	default:
	  g_print ("Unknown pixel format.\n");
	  return TRUE;

      }
      

      if( bpp == 8 )
      {
	sprintf(Text,"Mono 8 not working.");
      }
      else
      {
	appdata->cbCount++;
	QImage *img = new QImage( (unsigned char *)GST_BUFFER_DATA(buffer),width , height, Format);
	sprintf(Text,"img_%d.jpg",appdata->cbCount);
	img->save(Text);
	delete img;
      }
      appdata->TestLabel->setText(Text);
      
      appdata->SaveImage = FALSE;
  }
  
  
  // Do some image processing. Calc average brightness. Only, if 
  // image in memory has 32 bit. color channels must be 
  // BGRA (-> attention, here are kind of magic numbers.

  
  double bright = 0.0;

  if ( bpp == 32 )
  {
      guint8 *pos = buffer->data;
      guint8 *end = buffer->data + buffer->size;
      
      while( pos < end )
      {
	bright += (double)*pos; //B
	pos++;
	bright += (double)*pos; //G
	pos++;
	bright += (double)*pos; //R
	pos++;
	pos++;			//A
      }
  
    bright /= (double)buffer->size;
  
  }
  
  if ( bpp == 8 )
  {
      guint8 *pos = buffer->data;
      guint8 *end = buffer->data + buffer->size;
      
      while( pos < end )
      {
	bright += (double)*pos; //B
	pos++;
      }
  
    bright /= (double)buffer->size;
  
  }
  appdata->ImageBrightness = (int)bright;

  
  return TRUE;
}

//////////////////////////////////////////////////

showcamera::showcamera()
{
    CreateUI(); 
    FindCameras();
}

//////////////////////////////////////////////////
//
showcamera::~showcamera()
{
    delete _cPropertiesDialog;
    g_print("End of Program\n");
}


//////////////////////////////////////////////////////////////////////////
// Create the user interface in a QGridLayout.
//
void showcamera::CreateUI()
{
    window = new QWidget(this);
    //window->setFixedSize(640, 480);
    setCentralWidget( window );
    window->show();
    
    
    mainLayout = new QGridLayout;
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_SIMULATOR)
     mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
 #endif    
    window->setLayout(mainLayout);

    
    videowindow =  new QWidget();
    videowindow->setMinimumSize (640, 480);
    
    
    /******************************************************* 
     * The following code makes the videowindow transparent. This 
     * seemed to be necessary on XFCE, but not on Gnome.
     */
    videowindow->setAutoFillBackground(true);

    videowindow->setWindowOpacity(0);
    videowindow->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    videowindow->setAttribute(Qt::WA_NoSystemBackground);
    videowindow->setAttribute(Qt::WA_TranslucentBackground);
    videowindow->setAttribute(Qt::WA_PaintOnScreen);

    videowindow->setAttribute(Qt::WA_TransparentForMouseEvents);
     
    mainLayout->addWidget(videowindow, 0, 0,Qt::AlignLeft|Qt::AlignTop);
    videowindow->show();

    // ***********************************************************
    // Create the layout for the camera setup controls
    Cameralayout = new QVBoxLayout();
    TestLabel = new QLabel();
    TestLabel->setText("Ready");
    
    AppData.TestLabel = TestLabel;
    AppData.cbCount = 0;
    AppData.SaveImage = FALSE;
    
    Cameralayout->addWidget(TestLabel,Qt::AlignLeft|Qt::AlignTop);
    
    BrightnessBar = new QProgressBar();
    Cameralayout->addWidget(BrightnessBar,Qt::AlignLeft|Qt::AlignTop);
    BrightnessBar->setRange(0,255);
    BrightnessBar->setValue(0);
    BrightnessBar->setFormat("Brightness : %v");
    
    AppData.ImageBrightness = 0;
    
    FrameCounts = new QLabel();
    FrameCounts->setText("0/0");
    Cameralayout->addWidget(FrameCounts);
    
    // Combobox for select connected cameras
    cboCameras = new QComboBox();
    Cameralayout->addWidget(cboCameras);
    QObject::connect(cboCameras, SIGNAL(currentIndexChanged(int)), this, SLOT(OncboCamerasChanced(int)));
    
    // Combobox for select video format of above selected camera
    cboCamera = new QComboBox();
    Cameralayout->addWidget(cboCamera);
    QObject::connect(cboCamera, SIGNAL(currentIndexChanged(int)), this, SLOT(OncboCameraChanced(int)));
    
    PropertiesButton = new QPushButton("Properties");
    Cameralayout->addWidget(PropertiesButton);
    QObject::connect(PropertiesButton, SIGNAL(released()), this, SLOT( OnPropertiesButton()));
    
    
    
    /*
    sldExposureAutoReference = new QSlider(Qt::Horizontal );
    Cameralayout->addWidget(sldExposureAutoReference);
    QObject::connect(sldExposure, SIGNAL(sliderMoved(int)), this, SLOT(OnMovedsldExposureAutoExposureReference(int)));
    */
    
    mainLayout->addLayout(Cameralayout, 0,1,Qt::AlignLeft|Qt::AlignTop);
    // End of camera setup controls
    
    // ***********************************************************
    // Video controls layout
    Videolayout = new QHBoxLayout();
    
    SnapButton = new QPushButton("Snap Image" );
    connect(SnapButton, SIGNAL(released()), this, SLOT(OnSnapButton()));
    Videolayout->addWidget(SnapButton);

    StartButton = new QPushButton("Start" );
    connect(StartButton, SIGNAL(released()), this, SLOT(startcamera()));
    Videolayout->addWidget(StartButton);
   
    StopButton = new QPushButton("Stop" );
    connect(StopButton, SIGNAL(released()), this, SLOT(stopcamera()));
    Videolayout->addWidget(StopButton);
  
    mainLayout->addLayout(Videolayout, 1,0);
    // End Video controls layout
    
  
    // Create the properties dialog window
    _cPropertiesDialog = new CPropertiesDialog(this);
    
     connect(&_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    _Timer.start(200);
    
}


//////////////////////////////////////////////////////////////////////////
// Build a pipline for color cameras and a scaled display.
/*
                                                                              queue - scale-output
                                                                           /
 source-filter-color-autoexposure-whitebalance-bayer-queue-colorspace-tee
                                                                            \
                                                                               quere - image-filter-image_sink                                                                            
 

 */
 
///////////////////////////////////////////////////////////////////////#
//
void showcamera::startcamera()
{
  if( _cCamera != NULL )
  {
    _MainLoopThread.loop = _cCamera->GetPipelineLoop();
    _MainLoopThread.start();

    
    _cCamera->SetWindowHandle(videowindow->winId());
    _cCamera->SetCallback(buffer_probe_callback,(void*)&AppData);
    _cCamera->start();
    

    
    videowindow->hide();
    videowindow->show();
    
  }
}

///////////////////////////////////////////////////////////////////////#
//
void showcamera::stopcamera()
{
  if( _cCamera != NULL )
  {
    _cCamera->stop();
  }
  return;
}


void showcamera::FindCameras()
{
  int CamCount =   _CameraList.enumerate_cameras();
  if( CamCount > 0 )
  {
    for( int i = 0; i< CamCount; i++)
    {
      cboCameras->addItem(_CameraList.getUniqueCameraName(i));
    }
  }
}




void showcamera::GetDeviceProperties(const char *UniqueName)
{
  int ExposureMin = 0;
  int ExposureMax = 0;
  int ExposureValue = 0;
  cboCamera->clear();
  
  _cCamera = _CameraList.SelectCamera( UniqueName );
  
  if( _cCamera  != NULL )
  {
    //_cCamera.get_formats(UniqueName);
    //strcpy(CAMERA,_cCamera->_name);
  
    int i= 0;
    char format[256];
    while( _cCamera->getColorFormat(i,format) )
    {
      cboCamera->addItem(format);
      i++;
    }
    _cCamera->SelectFormat(0);
    
   _cPropertiesDialog->SetCamera(_cCamera);
  }
}

void showcamera::OnSnapButton()
{
  AppData.SaveImage = TRUE;
}

void showcamera::OncboCameraChanced(int Index)
{
  g_print("Select Format %d\n", Index );
  if( _cCamera != NULL )
  {
    _cCamera->SelectFormat(Index);
  }
} 

// New camera was selected
void showcamera::OncboCamerasChanced(int Index)
{
  g_print("Camera Change %s\n", cboCameras->currentText().toStdString().c_str());

  GetDeviceProperties(cboCameras->currentText().toStdString().c_str());
} 


void showcamera::OnPropertiesButton()
{
  if( _cCamera != NULL )
  {
    _cPropertiesDialog->SetCamera(_cCamera);
    _cPropertiesDialog->show();
  }
}


void showcamera::OnTimer()
{
  BrightnessBar->setValue(AppData.ImageBrightness);
#ifdef USE_FRAMECOUNTER  
  if( _cCamera != NULL )
  {
    char Text[100];
    sprintf(Text,"Dropp: %d, Rec: %d", _cCamera->GetDroppedFrameCount(), _cCamera->GetGoodFrameCount());
    FrameCounts->setText(Text);
  }
#endif
}

#include "showcamera.moc"
