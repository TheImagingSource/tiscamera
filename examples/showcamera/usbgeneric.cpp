#include "usbgeneric.h"
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

#define FORMAT_MONO "video/x-raw-gray" 
#define FORMAT_COLOR "video/x-raw-bayer"
#define FORMAT_MJPEG "video/image/jpeg"
#define PATTERN "grgb"


CUSBGenericCamera::CUSBGenericCamera(const char* path,const char* manufacturer, const char* name, const char* serial, const char* ProductID):
CUSBCamera(path, manufacturer,name,serial,ProductID,true)
{
  query_formats();
}
// Query avaialble video formats and frame rates. Save them in an internal list.
void CUSBGenericCamera::query_formats()
{
    struct stat st; 
    int fd = -1;     
    
    v4l2_fmtdesc formats;
    v4l2_frmsizeenum Res;
    v4l2_frmivalenum rates;
      
    _Formats.clear();
    g_print("Checking Generic camera  %s\n", _DevicePath);
    
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
	  char fourcc[5] = {0};
	  strncpy(fourcc, (char *)&formats.pixelformat, 4);
	  
	  g_print("\tFormat %d :  %s  %d %s\n", formats.index, formats.description, formats.type,  fourcc );
  
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
	  
	  formats.index++;
	}
	
	// Enumerate available properties. Just for testing here.
	struct v4l2_queryctrl qctrl;
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &qctrl)) 
	{
	  g_print("Property id 0x%x name %s min: %d, max: %d, default %d, type %d\n",qctrl.id, qctrl.name, qctrl.minimum, qctrl.maximum,qctrl.default_value, qctrl.type);
	  
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
	  qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	 
	::close(fd);
	  
	g_print("End\n");
      }
   } 

}




bool CUSBGenericCamera::CreatePipeline()
{
  GstCaps* caps; 
  
  g_print("Pipeline build start\n");
  
  //_Pipeline.loop 	= g_main_loop_new (NULL, FALSE);
  _Pipeline.pipeline 	= gst_pipeline_new ("camera");

  InstantiateGSTElements();
  
  char media_type[255];
  
  if( strcmp(_Formats[_SelectedCamera]->fourcc, "MJPG") == 0 )
  {
      strcpy( media_type, "video/mpeg, mpegversion=2");
  }
  else
  {
      sprintf( media_type,"video/x-raw, format=%s", _Formats[_SelectedCamera]->fourcc);
  }

   
  g_object_set( GetElement( "source"), "device", _DevicePath, NULL);
  caps = gst_caps_new_simple (media_type,
			      //"format", _Formats[_SelectedCamera]->fourcc,
                              "width", G_TYPE_INT, GetWidth(),
                              "height", G_TYPE_INT, GetHeight(),
                              "framerate", GST_TYPE_FRACTION, (unsigned int ) GetDenominator(), GetNumerator(),
                              NULL); 
  
  g_object_set( GetElement("filter"), "caps", caps, NULL);
  
   // Create the output pads of the tee video tee. Must be done, before the
  // videotee is linked.
   
  if( q1_pad == NULL )
  {
    g_critical ("Unable to get q1 pad");
    return false;
  }
  
  if( q2_pad == NULL )
  {
    g_critical ("Unable to get q2 pad");
    return false;
  }
  
  
  
   // Pipeline to the tee. Does the camera control and color image.
  if( !gst_element_link_many(GetElement("source"),
		    GetElement("filter"),
		    GetElement("decoder"),
		    GetElement("cogcolorspace"),			     
		    GetElement("tee"),
		    NULL))
  {
    g_critical ("Unable to link source pipeline");
    return false;
  }

 
  // Link the branch to the display of live video in the videowindow.
  if( ! gst_element_link_many(GetElement("queue1"), 
			      GetElement("videoscale1"), 
			      GetElement("output"), NULL))
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

  // Link the fakesing pipeline to the second video tee pad.
  ret = gst_pad_link (tee_q2_pad, q2_pad);
  if(ret != GST_PAD_LINK_OK)
  {
    g_critical ("Unable to link tee with queue2");
    return false;
  }
  
  _Pipeline.bus = gst_pipeline_get_bus (GST_PIPELINE (_Pipeline.pipeline));
  _Pipeline.bus_watch_id = gst_bus_add_watch (_Pipeline.bus, bus_call, _Pipeline.loop);
  
  gst_object_unref (_Pipeline.bus); 

  gst_element_set_state (_Pipeline.pipeline, GST_STATE_READY);  
  
  gst_x_overlay_set_xwindow_id(GST_X_OVERLAY ( GetElement("output") ),WindowHandle);

 
  g_print("Pipeline built.\n");
  return true;
}

/////////////////////////////////////////////////////////////////////////////////
// Unlinks all elements
//
bool CUSBGenericCamera::UnlinkPipeline()
{

  gst_element_set_state (_Pipeline.pipeline, GST_STATE_PAUSED); 
  gst_element_set_state (_Pipeline.pipeline, GST_STATE_NULL);

  g_main_loop_quit (_Pipeline.loop);
  
     // Pipeline to the tee. Does the camera control and color image.
  gst_element_unlink_many(GetElement("source"),
		    GetElement("filter"),
		    GetElement("decoder"),
		    GetElement("cogcolorspace"),			  
		    GetElement("tee"),
		    NULL);
  
  gst_element_unlink_many(GetElement("queue1"), 
			  GetElement("videoscale1"), 
			  GetElement("output"), NULL);


  gst_element_unlink_many(GetElement("queue2"), GetElement("image_sink"), NULL);

  gst_pad_unlink(tee_q1_pad, q1_pad);
  gst_pad_unlink(tee_q2_pad, q2_pad);

  // Clean up all used gst elements. This seems a stupid way... should better be a list.
  gst_object_unref(GetElement("source"));
  gst_object_unref(GetElement("filter"));

  gst_object_unref(GetElement("queue"));
  gst_object_unref(GetElement("decoder"));
  gst_object_unref(GetElement("cogcolorspace"));
  gst_object_unref(GetElement("tee"));
  
  gst_object_unref(GetElement("queue1"));
  gst_object_unref(GetElement("videoscale1"));
  gst_object_unref(GetElement("output"));
  
  gst_object_unref(GetElement("queue2"));
  gst_object_unref(GetElement("image_sink"));
  
  // clean up and delete the pipeline
  gst_object_unref (GST_OBJECT (_Pipeline.pipeline));
  g_source_remove (_Pipeline.bus_watch_id);

  return true;
  
}

//////////////////////////////////////////////////////////////////////////
// Instantiate (create new) all needed GST pipeline elements.
// return true, if all elements were created successfully, else false
//
bool CUSBGenericCamera::InstantiateGSTElements()
{
  bool success = true;
  
  InstantiateGSTElement("v4l2src",		"source", &success);
  InstantiateGSTElement("capsfilter",          "filter", &success);

  if( strcmp(_Formats[_SelectedCamera]->fourcc, "MJPG") == 0 )
  {
    InstantiateGSTElement("jpegdec",          "decoder", &success);
  }
  else
  {
    InstantiateGSTElement("ffmpegcolorspace",    "decoder", &success);
  }

  
  InstantiateGSTElement("cogcolorspace","cogcolorspace", &success);

  InstantiateGSTElement("ximagesink",          "output", &success);
  InstantiateGSTElement("videoscale","videoscale1", &success);
  //InstantiateGSTElement("capsfilter",          "Targetfilter", &success);
  InstantiateGSTElement("tee","tee", &success);
  InstantiateGSTElement("queue", "queue1", &success);
  InstantiateGSTElement("queue", "queue2", &success);
  InstantiateGSTElement("fakesink", "image_sink", &success);

  tee_q1_pad = gst_element_get_request_pad(GetElement( "tee"), "src%d");
  tee_q2_pad = gst_element_get_request_pad(GetElement( "tee"), "src%d");
  
  q1_pad = gst_element_get_static_pad (GetElement( "queue1"), "sink"); 
  q2_pad = gst_element_get_static_pad (GetElement( "queue2"), "sink");
  
  if( !success) 
    return false;

  // Set the callback for our fakesing "image_sink"
  
  return true;
}

