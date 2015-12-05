#include "usbcamera.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <dirent.h> 
#include <gst/gst.h>
#include <glib.h>
#include <sys/ioctl.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/mman.h> 
#include <sys/time.h>
#include <fcntl.h> 
#include <unistd.h>
#include <linux/videodev2.h>
#include <libudev.h>

#include <gst/interfaces/xoverlay.h>


#define CLEAR(x) memset(&(x), 0, sizeof(x)) 


#define WBRed 0x001
#define WBGreen 0x002
#define WBBlue 0x003
#define WBAuto 0x004
#define ExposureAuto 0x005
#define FocusOnePush 0x006



// If you are on a weak computer uncomment the following define. 
// Then the videorate module is inserted and the frame rate is set to 2 fps
// for the rest of the pipeline...
//#define RaspberryPI

CUSBCamera::CUSBCamera(const char* path,const char* manufacturer, const char* name, const char* serial, const char* ProductID, bool Color):
CCamera( manufacturer,name,serial)
{
	_Color = Color;
	
    strcpy(_DevicePath, path); // e.g. "/dev/video0"
    
    if( ProductID != NULL )
      strcpy(_ProductID, ProductID);
    else
      strcpy(_ProductID, "");
	
	query_formats();

    
}

// Query the avaialble color formats / pixel formats.
bool CUSBCamera::getColorFormat( int Index, char* Format )
{
    if( Index >= 0 && Index < _Formats.size() )
    {
      sprintf( Format,"%s (%dx%d) @ %d/%d",_Formats[Index]->fourcc, _Formats[Index]->width,_Formats[Index]->height,_Formats[Index]->denominator,_Formats[Index]->numerator );
      return true;
    }

  return false;
}


// Query avaialble video formats and frame rates. Save them in an internal list.
void CUSBCamera::query_formats()
{
    struct stat st; 
    int fd = -1;     
    _HasFocus = false; // Assume no focus unit
    
    v4l2_fmtdesc formats;
    v4l2_frmsizeenum Res;
    v4l2_frmivalenum rates;
      
    _Formats.clear();
    g_print("Checking %s\n", _DevicePath);
    
    if (-1 == stat(_DevicePath, &st)) 
    {
        g_print("Cannot identify '%s': %d, %s\n", _DevicePath, errno, strerror(errno));
    } 
    else
    {
      
      fd = open(_DevicePath, O_RDWR /* required */ | O_NONBLOCK, 0);

      if (-1 == fd) 
      {
	  g_print( "Cannot open '%s': %d, %s\n", _DevicePath, errno, strerror(errno));
      }
      else
      {
	
	CLEAR(formats);

	formats.index = 0;
	formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	
	while( ioctl(fd, VIDIOC_ENUM_FMT, &formats) == 0 )
	{
	  if( strstr((char*)formats.description, "Y800") != NULL  ) // Get Y800 format only. Will be debayered to color.
	  {
	    g_print("\tFormat %d :  %s  %d\n", formats.index, formats.description, formats.type);
    
	    Res.pixel_format = formats.pixelformat;
	    Res.index = 0;
	    while( ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &Res) == 0 )
	    {
	      g_print("\t\t%dx%d\n", Res.discrete.width, Res.discrete.height);
	      rates.pixel_format = Res.pixel_format;
	      rates.width = Res.discrete.width;
	      rates.height = Res.discrete.height;
	      rates.index = 0;
	      while( ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &rates) == 0 )
	      {
		g_print("\t\t\t%d/%d\n", rates.discrete.denominator, rates.discrete.numerator);
		Format_t *cam = new Format_t;
		cam->width = rates.width;
		cam->height = rates.height;
		cam->denominator = rates.discrete.denominator;
		cam->numerator = rates.discrete.numerator;
		cam->pixelformat = formats.pixelformat;
		strncpy(cam->fourcc, (char *)&formats.pixelformat, 4);
		cam->fourcc[4] ='\0';

		_Formats.push_back(cam);
		rates.index++;
	      }
	      Res.index++;
	    }
	  }
	  
	  formats.index++;
	}
	
	// Enumerate available properties. Just for testing here.
	struct v4l2_queryctrl qctrl;
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) 
	{
	  g_print("Property id 0x%x name %s min: %d, max: %d, default %d, type %d\n",qctrl.id, qctrl.name, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.type);
	  
	  if(qctrl.id != 0x980921 &&
	     qctrl.id != 0x980922 &&
	     qctrl.id != 0x980923 &&
	     qctrl.id != 0x980926 && // softwaretrigger
	     qctrl.id != 0x980924 &&  //Trigger
	     qctrl.id != 0x980930)	// Global Reset shutter
	  {
	    
	    switch(qctrl.type)
	    {
     	      case 2:
		AddProperty((const char* )qctrl.name, qctrl.id, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value, CProperty::BOOLEAN );
		break;

    	      case 4:
		AddProperty((const char* )qctrl.name, qctrl.id, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value, CProperty::BUTTON );
		break;

		
	      case 1:
	      default:
		AddProperty((const char* )qctrl.name, qctrl.id, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value );
		break;
	    }
	  }
	  
	  // Trigger and Software Trigger
	  if( qctrl.id ==(__u32)0x980924 )
	  {
	    g_print("Hallo\n");
	     AddProperty((const char* )qctrl.name, qctrl.id, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value, CProperty::BOOLEAN  );
	     AddProperty("Software Trigger",  0x980926, 0, 1,0, 0, CProperty::BUTTON  );
	  }

	  // Global Reset Shutter 
	  if( qctrl.id == 0x980930 ) 
	  {
	     AddProperty((const char* )qctrl.name, qctrl.id, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value, CProperty::BOOLEAN  );
	  }

	  // Check for focus unit
	  if( qctrl.id == 0x9a090a ) 
	  {
		_HasFocus = true;
		AddProperty("Focus One Push", FocusOnePush, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.default_value, CProperty::BUTTON );

	  }
	  qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	AddProperty("Expsoure Auto", ExposureAuto, 0, 1, 1, 1, CProperty::BOOLEAN );

	// Add the gstreamer WB Properties

	AddProperty("White Balance Auto", WBAuto, 0, 1, 1, 1, CProperty::BOOLEAN );
	AddProperty("Red", WBRed, 0, 255, 64, 64 );
	AddProperty("Green", WBGreen, 0, 255, 64, 64 );
	AddProperty("Blue", WBBlue, 0, 255, 64, 64 );
	
	 
	::close(fd);
	  
	g_print("End\n");
      }
    } 

}



void CUSBCamera::SelectFormat( int Index)
{
  if( Index >= 0 && Index < _Formats.size() )
    _SelectedCamera = Index;
  else
    _SelectedCamera = -1;

}

int CUSBCamera::GetWidth()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->width;

  return 0;
}

int CUSBCamera::GetHeight()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->height;

  return 0;
}

int CUSBCamera::GetDenominator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->denominator;

  return 0;
}

int CUSBCamera::GetNumerator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->numerator;

  return 0;
}

/////////////////////////////////////////////////////////////////////
bool CUSBCamera::start()
{

  CreatePipeline(); 

  g_object_set(G_OBJECT( GetElement("image_sink" )),"signal-handoffs", TRUE, NULL);
  g_signal_connect(G_OBJECT( GetElement( "image_sink")), "handoff", G_CALLBACK(_cbfunc), _CallbackData);	

  
  GstStateChangeReturn sret = gst_element_set_state (_Pipeline.pipeline, GST_STATE_PLAYING);
  if (sret == GST_STATE_CHANGE_FAILURE) 
  {
    g_printerr ("Playing set_state failed.\n");
    return false;
  }
  else
  {
    g_print ("Playing\n");
  }
  
  
  return true;
}


bool CUSBCamera::stop()
{
  UnlinkPipeline();
  return true;
}

GMainLoop* CUSBCamera::GetPipelineLoop()
{
  return _Pipeline.loop;
}


bool CUSBCamera::SetProperty( int  Property, int  value)
{
    struct v4l2_control control = {0}; 
    control.id = Property; 
    control.value = value; 
    bool result = true;
    
    // Handle the properties of GStreamer first.
    switch( Property )
    {
      case WBAuto:
	return SetAuto("white", "auto", value == 1);
	break;

      case ExposureAuto:
	return SetAuto("autoexposure", "auto-exposure", value == 1);
	break;
	
	  case FocusOnePush:
		return SetAuto("autofocus", "auto", value == 1);
		break;
	
      case WBRed:
	return SetGstProperty("white","red",value);
	break;
	
      case WBGreen:
	return SetGstProperty("white","green",value);
	break;
	
      case WBBlue:
	return SetGstProperty("white","blue",value);
	break;
    }	
	
    // Handle properties provided by the V4L2 driver 
    g_print("Set Property %x to %d\n", Property, value );
    gint fd;
    GstElement *source = GetElement("source");
    if( source != NULL )
    {
      g_object_get(source, "device-fd", &fd, NULL);
    }
    else
    {
      fd = open(_DevicePath, O_RDWR /* required */ | O_NONBLOCK, 0);
    }

    if (tis_xioctl(fd, VIDIOC_S_CTRL, &control) == -1) 
    { 
      result = false;
    }

    if( source == NULL )
    {
      ::close(fd);
    }

    return result;
}

bool CUSBCamera::GetProperty( int  Property, int &value )
{
    struct v4l2_control control = {0}; 
    control.id = Property; 
    control.value = value; 
      
    bool result = true;
    
    gint fd;
    GstElement *source = GetElement("source");
    if( source != NULL )
    {
      g_object_get(source, "device-fd", &fd, NULL);
    }
    else
    {
      fd = open(_DevicePath, O_RDWR /* required */ | O_NONBLOCK, 0);
    }

    if (tis_xioctl(fd, VIDIOC_G_CTRL, &control) == -1) 
    { 
      result = false;
    }

    if( source == NULL )
    {
      ::close(fd);
    }

    return result;
}


int CUSBCamera::tis_xioctl (int fd, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

bool CUSBCamera::GetPropertyRange( int  Property, int &Min , int &Max)
{
  struct v4l2_queryctrl qctrl;
  gint fd = open(_DevicePath, O_RDWR /* required */ | O_NONBLOCK, 0);

  qctrl.id = (unsigned int) Property;
  if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) 
  {
    Min = qctrl.minimum;
    Max =  qctrl.maximum;
    g_print("Property id 0x%x name %s min: %d, max: %d\n",qctrl.id, qctrl.name, qctrl.minimum, qctrl.maximum);
    ::close(fd);
    return true;
  }
  ::close(fd);
  return false;
}

bool CUSBCamera::SetAuto( const gchar* Elementname, const gchar* Property, bool OnOff )
{
  GstElement *Auto = GetElement( Elementname) ;
  
  if( Auto != NULL)
  {
    g_object_set(G_OBJECT( Auto), Property, OnOff, NULL);
    return true;
  }
  
  return false;
}

bool CUSBCamera::SetGstProperty( const gchar* Elementname, const gchar* Property, int value )
{
  GstElement *Prop = GetElement( Elementname) ;
  
  if( Prop != NULL)
  {
    g_object_set(G_OBJECT( Prop), Property, value, NULL);
    return true;
  }
  
  return false;
}

#define FORMAT_MONO "video/x-raw-gray" 
#define FORMAT_COLOR "video/x-raw-bayer"
#define PATTERN "grgb"

/////////////////////////////////////////////////////////////////////////////////////
/*
  Create the pipeline, depending on _Color to insert tiscolor, tiswhitebalance and bayer2rbg
  and depending on _HasFocus whether the tis_autofocus must be inserted.
 */ 
bool CUSBCamera::CreatePipeline()
{
	GstCaps* caps; 
	GstElement *LastElement = NULL; // This is a last element in chain, where to link following elements to.
	
	g_print("Pipeline build start\n");
	
	//_Pipeline.loop 	= g_main_loop_new (NULL, FALSE);
	_Pipeline.pipeline 	= gst_pipeline_new ("camera");

	InstantiateGSTElements();
	
	g_object_set( GetElement( "source"), "device", _DevicePath, NULL);
		caps = gst_caps_new_simple (FORMAT_MONO,
									"format", G_TYPE_STRING, PATTERN, 
									"width", G_TYPE_INT, GetWidth(),
									"height", G_TYPE_INT, GetHeight(),
									"framerate", GST_TYPE_FRACTION, (unsigned int ) GetDenominator(), GetNumerator(),
									NULL); 
	
	g_object_set( GetElement("filter"), "caps", caps, NULL);


	// Link the source pipeline until autoexposure.
	if( !gst_element_link_many(GetElement("source"), GetElement("filter"),	GetElement("bufferfilter"),
	#ifdef RaspberryPI
				GetElement("videorate"),	     
	#endif			     
				GetElement("autoexposure"), NULL ))
	{
		g_critical ("Unable to link source pipeline to autoexposure");
		return false;
	}

	LastElement = GetElement("autoexposure");
	
	// Add the colorizing pipepline elemnts
	if( _Color )
	{
		if( !gst_element_link_many(LastElement,GetElement("tiscolor"),GetElement("queuewb"), GetElement("white"),GetElement("queuebayer"),GetElement("bayer"),NULL ))
		{
			g_critical ("Unable to link color pipeline to autoexposure");
			return false;
		}
		LastElement = GetElement("bayer");
	}
  
    // Chec for Focus
	if( _HasFocus )
	{
		if( !gst_element_link_many(LastElement,GetElement("autofocus"),NULL ))
		{
			g_critical ("Unable to insert auto focus");
			return false;
		}
		LastElement = GetElement("autofocus");
}

	// Add queue,  ffmpegcolorspace and the tee filter
	 if( !gst_element_link_many( LastElement,GetElement("queue"),GetElement("colorspace"),GetElement("tee"),NULL))
	 {
		g_critical ("Unable to link queue to tee.");
		return false;
	 }
	 
 
	// Link the branch to the display of live video in the videowindow.
	if( ! gst_element_link_many(GetElement("queue1"), GetElement("videoscale1"), GetElement("output"), NULL))
	{
		g_critical ("Unable to link first branch.");
		return false;
	}

	
	// Link the branch to the fakesink for image processing.
	if( ! gst_element_link_many(GetElement("queue2"), GetElement("image_filter"),GetElement("sinkcaps"), GetElement("image_sink"), NULL))
	{
		g_critical ("Unable to link second branch.");
		return  false;
	}
	
		
	if( !gst_element_link( GetElement("tee"),GetElement("queue1")))
	{
		g_critical ("Unable to link tee with queue1");
		return false;
	}

	
	if(!gst_element_link( GetElement("tee"),GetElement("queue2")))
	{
		g_critical ("Unable to link tee with queue2");
		return false;
	}
	
	caps = gst_caps_new_simple ("video/x-raw-rgb",
								"bpp", G_TYPE_INT, 32u,
								"depth", G_TYPE_INT, 24u,									
								NULL); 

	g_object_set( GetElement("sinkcaps"), "caps", caps, NULL);

	
	_Pipeline.bus = gst_pipeline_get_bus (GST_PIPELINE (_Pipeline.pipeline));
	_Pipeline.bus_watch_id = gst_bus_add_watch (_Pipeline.bus, bus_call, _Pipeline.loop);
	
	gst_object_unref (_Pipeline.bus); 

	gst_element_set_state (_Pipeline.pipeline, GST_STATE_READY);  
	
	gst_x_overlay_set_xwindow_id(GST_X_OVERLAY ( GetElement("output") ),WindowHandle);

#ifdef RaspberryPI
	g_object_set(GetElement("videorate"), "drop-only", 1, NULL);
	g_object_set(GetElement("videorate"), "max-rate", 2, NULL);
#endif			     
	
	g_print("Pipeline built.\n");
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Instantiate (create new) all needed GST pipeline elements.
// return true, if all elements were created successfully, else false
//
bool CUSBCamera::InstantiateGSTElements()
{
	bool success = true;
	
	InstantiateGSTElement("v4l2src",		"source", &success);
	InstantiateGSTElement("capsfilter",          "filter", &success);
	InstantiateGSTElement("tisvideobufferfilter",          "bufferfilter", &success);
	InstantiateGSTElement("tis_auto_exposure",   "autoexposure", &success);
	InstantiateGSTElement("queue",		 "queue", &success);
	InstantiateGSTElement("ffmpegcolorspace",    "colorspace", &success);
	InstantiateGSTElement("ximagesink",          "output", &success);

	
	if( _Color )
	{
		InstantiateGSTElement("bayer2rgb",           "bayer", &success);
		InstantiateGSTElement("tiswhitebalance",     "white", &success);
		InstantiateGSTElement("tiscolorize",         "tiscolor", &success);
		InstantiateGSTElement("queue", "queuebayer", &success);
		InstantiateGSTElement("queue", "queuewb", &success);
	}
	
	
	InstantiateGSTElement("videoscale","videoscale1", &success);
	InstantiateGSTElement("capsfilter",          "Targetfilter", &success);
	InstantiateGSTElement("tee","tee", &success);
	InstantiateGSTElement("queue", "queue1", &success);
	InstantiateGSTElement("queue", "queue2", &success);


	InstantiateGSTElement("ffmpegcolorspace", "image_filter", &success);
	InstantiateGSTElement("capsfilter", "sinkcaps", &success);
	InstantiateGSTElement("fakesink", "image_sink", &success);
	
	if( _HasFocus )
	{
		InstantiateGSTElement("tis_autofocus", "autofocus", &success);
	}

#ifdef RaspberryPI
	InstantiateGSTElement("videorate", "videorate", &success);
#endif

  if( !success) 
    return false;
  return true;
}


/////////////////////////////////////////////////////////////////////////////////
// Unlinks all elements
//
bool CUSBCamera::UnlinkPipeline()
{
	GstElement *LastElement = NULL;
	
	gst_element_set_state (_Pipeline.pipeline, GST_STATE_PAUSED); 
	gst_element_set_state (_Pipeline.pipeline, GST_STATE_NULL);

	g_main_loop_quit (_Pipeline.loop);

	gst_element_unlink_many(GetElement("tee"),GetElement("queue1"),GetElement("image_filter"),GetElement("sinkcaps"),GetElement("image_sink"),NULL);
	
	gst_element_unlink_many(GetElement("tee"), GetElement("queue1"), GetElement("videoscale1"), GetElement("output"), NULL);
	
	gst_element_unlink_many(GetElement("queue"),GetElement("colorspace"), GetElement("tee"),NULL);
	
	if( _HasFocus )
	{
		if( _Color )
			gst_element_unlink_many(GetElement("bayer"), GetElement("autofocus"), GetElement("queue"), NULL);
		else
			gst_element_unlink_many(GetElement("autoexposure"), GetElement("autofocus"), GetElement("queue"), NULL);
	}

	if( _Color )
	{
		gst_element_unlink_many(GetElement("tiscolor"),GetElement("queuewb"),GetElement("white"),GetElement("queue"),GetElement("bayer"),NULL);
		gst_object_unref(GetElement("queuewb"));
		gst_object_unref(GetElement("queuebayer"));
	}

	gst_element_unlink_many(GetElement("source"),GetElement("filter"),GetElement("bufferfilter"),GetElement("autoexposure"),NULL);

	// Clean up all used gst elements. This seems a stupid way... should better be a list.
	gst_object_unref(GetElement("source"));
	gst_object_unref(GetElement("filter"));
	gst_object_unref(GetElement("autoexposure"));
	if( _Color )
	{
		gst_object_unref(GetElement("tiscolor"));
		gst_object_unref(GetElement("white"));
		gst_object_unref(GetElement("bayer"));
	}

	if( _HasFocus )
	{
		gst_object_unref(GetElement("autofocus"));
	}

	gst_object_unref(GetElement("queue"));
	gst_object_unref(GetElement("colorspace"));
	gst_object_unref(GetElement("tee"));

	gst_object_unref(GetElement("queue1"));
	gst_object_unref(GetElement("videoscale1"));
	gst_object_unref(GetElement("output"));

	gst_object_unref(GetElement("queue2"));
	gst_object_unref(GetElement("image_filter"));
	gst_object_unref(GetElement("image_sink"));

	// clean up and delete the pipeline
	gst_object_unref (GST_OBJECT (_Pipeline.pipeline));
	g_source_remove (_Pipeline.bus_watch_id);
	//g_main_loop_unref (_Pipeline.loop);  

	return true;
  
}


int CUSBCamera::GetDroppedFrameCount()
{
	return 0;
    int result = 0;
    GstElement *filter = GetElement("bufferfilter");
    if( filter != NULL )
    {
      g_object_get(filter, "dropcount", &result, NULL);
    }
    return result;
}



int CUSBCamera::GetGoodFrameCount()
{
	return 0;
    int result = 0;
    GstElement *filter = GetElement("bufferfilter");
    if( filter != NULL )
    {
      g_object_get(filter, "framecount", &result, NULL);
    }
    return result;
}
