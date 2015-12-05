#include "config.h"
#include "dfkafu050.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <libudev.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/interfaces/xoverlay.h>


#define CLEAR(x) memset(&(x), 0, sizeof(x)) 



#ifdef USE_AFU050

#define PROP_AFU050_FOCUS 0x199e001
#define PROP_AFU050_EXPOSURE_AUTO 0x199e002
#define PROP_AFU050_GAIN_AUTO 0x199e003
#define PROP_AFU050_WB_AUTO 0x199e004


int new_frame_cb(afu050_handle_t handle, 
		 unsigned char *jpegbuf,
		 size_t jpegsize,
		 void *data) 
{
	GstElement *appsrc = GST_ELEMENT(data);
	GstBuffer *buf;

	//printf ("new frame: %d!\n", jpegsize); 

	buf = gst_buffer_new();
	GST_BUFFER_DATA(buf) = jpegbuf;
	GST_BUFFER_SIZE(buf) = jpegsize;
	GST_BUFFER_MALLOCDATA(buf) = jpegbuf;

	if (gst_app_src_push_buffer(GST_APP_SRC(appsrc), buf) != GST_FLOW_OK)
	{
	  gst_buffer_unref (buf);
	}
	return 0;
}

#endif

CDFKAFU050::CDFKAFU050(const char* path,const char* manufacturer, const char* name, const char* serial):
CCamera( manufacturer,name,serial)
{
    query_formats();
}

// Query the avaialble color formats / pixel formats.
bool CDFKAFU050::getColorFormat( int Index, char* Format )
{
    if( Index >= 0 && Index < _Formats.size() )
    {
      sprintf( Format,"(%dx%d) @ %d/%d",_Formats[Index].width,_Formats[Index].height,_Formats[Index].denominator,_Formats[Index].numerator );
      return true;
    }

  return false;
}


// Query avaialble video formats and frame rates. Save them in an internal list.
void CDFKAFU050::query_formats()
{
 
    _Formats.clear();
#ifdef USE_AFU050
    
    _Formats.push_back( cFormat(2955,1944,15,1, AFU050_FMT_2592X1944 ));	
    //_Formats.push_back( cFormat(2955,1944,15,2, AFU050_FMT_2592X1944 ));	

    _Formats.push_back( cFormat(1920,1080,30,1, AFU050_FMT_1920X1080 ));	
    //_Formats.push_back( cFormat(1920,1080,15,1, AFU050_FMT_1920X1080 ));	
    //_Formats.push_back( cFormat(1920,1080,15,2, AFU050_FMT_1920X1080 ));	
    
    _Formats.push_back( cFormat(1280,960,60,1, AFU050_FMT_1280X960 ));	
    //_Formats.push_back( cFormat(1280,960,30,1, AFU050_FMT_1280X960 ));	
    //_Formats.push_back( cFormat(1280,960,15,1, AFU050_FMT_1280X960 ));	
    //_Formats.push_back( cFormat(1280,960,15,2, AFU050_FMT_1280X960 ));	
    _SelectedCamera = 0;
    
    AddProperty("Focus", PROP_AFU050_FOCUS, 0, 1, 1, 1, CProperty::BUTTON );
    AddProperty("Exposure auto", PROP_AFU050_EXPOSURE_AUTO, 0, 1, 1, 1, CProperty::BOOLEAN );
    AddProperty("Gain auto", PROP_AFU050_GAIN_AUTO, 0, 1, 1, 1, CProperty::BOOLEAN );
    AddProperty("WhiteBalance auto", PROP_AFU050_WB_AUTO, 0, 1, 1, 1, CProperty::BOOLEAN );
#endif
}



void CDFKAFU050::SelectFormat( int Index)
{
  if( Index >= 0 && Index < _Formats.size() )
    _SelectedCamera = Index;
  else
    _SelectedCamera = -1;

}

int CDFKAFU050::GetWidth()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera].width;

  return 0;
}

int CDFKAFU050::GetHeight()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera].height;

  return 0;
}

int CDFKAFU050::GetDenominator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera].denominator;

  return 0;
}

int CDFKAFU050::GetNumerator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera].numerator;

  return 0;
}

/////////////////////////////////////////////////////////////////////
bool CDFKAFU050::start()
{

  CreatePipeline(); 


  return true;
}


bool CDFKAFU050::stop()
{
#ifdef USE_AFU050
  
  if( _handle != NULL )
  {
    afu050_capture_stop(_handle);
    gst_element_set_state (_Pipeline.pipeline, GST_STATE_PAUSED); 
    UnlinkPipeline();
    afu050_close(_handle);
    _handle = NULL;
  }
#endif  
  return true;
}

GMainLoop* CDFKAFU050::GetPipelineLoop()
{
  return _Pipeline.loop;
}

bool CDFKAFU050::CreatePipeline()
{
#ifdef USE_AFU050
 
  GstCaps* caps; 
  GstCaps* videoscalecaps; 
  GstElement *appsrc;
  g_print("Pipeline build start\n");
  
  _handle = afu050_open();
  if(!_handle )
  {
    printf("Failed to open camera.\n");
    return false;
  }

  afu050_set_video_format(_handle, _Formats[_SelectedCamera].internalFormat);

  _Pipeline.pipeline 	= gst_pipeline_new ("camera");

  
  if( !InstantiateGSTElements() )
  {
    return false;
  }
   
  appsrc = GetElement( "appsrc");
  g_object_set( appsrc, "block", FALSE, NULL);
    
  caps = gst_caps_from_string("image/jpeg");
  gst_app_src_set_caps(GST_APP_SRC( appsrc ), caps);

  // Create the output pads of the tee video tee. Must be done, before the
  // videotee is linked.

  // Size of the output window
    videoscalecaps = gst_caps_new_simple ("video/x-raw-rgb",
                              "width", G_TYPE_INT, 640, //GetWidth(),
                              "height", G_TYPE_INT, 480, //GetHeight(),
                                NULL); 

    g_object_set( GetElement("filter"), "caps", videoscalecaps, NULL);
    g_object_set( GetElement("ximagesink"), "sync", FALSE, NULL);
  
  
   // Pipeline to the tee. Does the camera control and color image.
  if( !gst_element_link_many(appsrc,
		    GetElement("jpegdec"),
		    GetElement("cogcolorspace"),
		    GetElement("tee"),
			     NULL) )
  {
    g_critical ("Unable to link source pipeline");
    return false;
  }

    // Link the branch to the display of live video in the videowindow.
  if( ! gst_element_link_many(GetElement("queue1"), 
			      GetElement("videoscale1"),
			      GetElement("filter"),
			      GetElement("ximagesink"),
			      NULL))
  {
    g_critical ("Unable to link first branch.");
    return false;
  }

  // Link the branch to the fakesink for image processing.
  if( ! gst_element_link_many(GetElement("queue2"), GetElement("image_sink"), NULL))
  {
    g_critical ("Unable to link second branch.");
    return  false;
  }
  
  

  // Link the display pipeline to the first video tee pad.
  GstPadLinkReturn ret = gst_pad_link (tee_q1_pad, q1_pad);
  if(ret != GST_PAD_LINK_OK)
  {
    g_critical ("Unable to link tee with queue1");
    return false;
  }

  // Link the fakesink pipeline to the second video tee pad.
  ret = gst_pad_link (tee_q2_pad, q2_pad);
  if(ret != GST_PAD_LINK_OK)
  {
    g_critical ("Unable to link tee with queue2");
    return false;
  }
  
  
  
  _Pipeline.bus = gst_pipeline_get_bus (GST_PIPELINE (_Pipeline.pipeline));
  _Pipeline.bus_watch_id = gst_bus_add_watch (_Pipeline.bus, bus_call, _Pipeline.loop);
  
  gst_object_unref (_Pipeline.bus); 

  //gst_element_set_state (_Pipeline.pipeline, GST_STATE_READY);  
  
  gst_x_overlay_set_xwindow_id(GST_X_OVERLAY ( GetElement("ximagesink") ),WindowHandle);

  

  afu050_set_exposure_auto (_handle, 1);
  afu050_set_gain_auto (_handle, 1);
  afu050_set_white_balance_auto (_handle, 1);
  
  g_object_set(G_OBJECT( GetElement("image_sink" )),"signal-handoffs", TRUE, NULL);
  g_signal_connect(G_OBJECT( GetElement( "image_sink")), "handoff", G_CALLBACK(_cbfunc), _CallbackData);	

  
  GstStateChangeReturn sret = gst_element_set_state (_Pipeline.pipeline, GST_STATE_PLAYING);
  //gst_element_get_state(_Pipeline.pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
  if (sret == GST_STATE_CHANGE_FAILURE) 
  {
    g_printerr ("Playing set_state failed.\n");
    return false;
  }
  else
  {
    g_print ("Playing\n");
  }


  if (afu050_capture_start(_handle, new_frame_cb, appsrc) < 0)
  {
	  fprintf (stderr, "failed to start capture\n");
	  return false;
  }
  
  g_print("Pipeline built.\n");
#endif
 
  return true;
}

/////////////////////////////////////////////////////////////////////////////////
// Unlinks all elements
//
bool CDFKAFU050::UnlinkPipeline()
{
#ifdef USE_AFU050

  gst_element_set_state (_Pipeline.pipeline, GST_STATE_PAUSED); 
  gst_element_set_state (_Pipeline.pipeline, GST_STATE_NULL);

  g_main_loop_quit (_Pipeline.loop);

  
     // Pipeline to the tee. Does the camera control and color image.
  gst_element_unlink_many(GetElement("appsrc"),
		    GetElement("jpegdec"),
		    GetElement("cogcolorspace"),
		    GetElement("tee"),
		    NULL);
  
  gst_element_unlink_many(GetElement("queue1"), GetElement("videoscale1"), GetElement("filter"),
			      GetElement("ximagesink"), NULL);

  gst_element_unlink_many(GetElement("queue2"), GetElement("image_sink"), NULL);

  
  gst_pad_unlink(tee_q1_pad, q1_pad);
  gst_pad_unlink(tee_q2_pad, q2_pad);

  
  gst_object_unref(GetElement("appsrc"));
  gst_object_unref(GetElement("filter"));
  gst_object_unref(GetElement("jpegdec"));
  gst_object_unref(GetElement("tee"));
  gst_object_unref(GetElement("ximagesink"));
  gst_object_unref(GetElement("queue1"));
  gst_object_unref(GetElement("queue2"));
  gst_object_unref(GetElement("videoscale1"));
  gst_object_unref(GetElement("image_sink"));
  
   // clean up and delete the pipeline
  gst_object_unref (GST_OBJECT (_Pipeline.pipeline));
  g_source_remove (_Pipeline.bus_watch_id);
#endif
  return true;
  
}

bool CDFKAFU050::InstantiateGSTElements()
{
 #ifdef USE_AFU050

  bool success = true;
  
  InstantiateGSTElement("appsrc",		"appsrc", &success);
  InstantiateGSTElement("capsfilter",          "filter", &success);
  InstantiateGSTElement("jpegdec",          "jpegdec", &success);
  InstantiateGSTElement("tee","tee", &success);
  InstantiateGSTElement("ximagesink",          "ximagesink", &success);
  InstantiateGSTElement("queue",		 "queue1", &success);
  InstantiateGSTElement("queue",		 "queue2", &success);
  InstantiateGSTElement("videoscale","videoscale1", &success);
  InstantiateGSTElement("cogcolorspace","cogcolorspace", &success);
  InstantiateGSTElement("fakesink", "image_sink", &success);
  
  tee_q1_pad = gst_element_get_request_pad(GetElement( "tee"), "src%d");
  tee_q2_pad = gst_element_get_request_pad(GetElement( "tee"), "src%d");

  q1_pad = gst_element_get_static_pad (GetElement( "queue1"), "sink"); 
  q2_pad = gst_element_get_static_pad (GetElement( "queue2"), "sink");
  
  #ifdef RaspberryPI
  InstantiateGSTElement("videorate", "videorate", &success);
#endif

  if( !success) 
    return false;

  // Set the callback for our fakesing "image_sink"
#endif  
  return true;
}




bool CDFKAFU050::SetProperty( int  Property, int  value)
{
#ifdef USE_AFU050
  if( _handle != NULL )
  {
    switch(Property)
    {
      case PROP_AFU050_FOCUS:
	afu050_auto_focus(_handle);
	break;

      case PROP_AFU050_EXPOSURE_AUTO:
	afu050_set_exposure_auto(_handle,value);
	break;

      case PROP_AFU050_GAIN_AUTO:
	afu050_set_gain_auto(_handle,value);
	break;

      case PROP_AFU050_WB_AUTO:
	afu050_set_white_balance_auto(_handle,value);
	break;
	
      default:
	break;
    }
  }
#endif  
  return false;
}

bool CDFKAFU050::GetProperty( int  Property, int &value )
{
    return false;
}

bool CDFKAFU050::GetPropertyRange( int  Property, int &Min , int &Max)
{
  return false;
}

