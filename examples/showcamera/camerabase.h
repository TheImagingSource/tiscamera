#ifndef camerabase_H
#define camerabase_H
#include <glib.h>
#include <gst/gst.h>
#include <glib.h>
#include "property.h"
#include <vector>
/*
 * Base class for all cameras. In implements standard properties such as name, serial and gstreamer callback
 * start live and stoplive are virtual and must be implemented in the classes, that inherit from this
 * The inherit classes can be USB, GigE based on Aravis and the DFK AFU050-L53
 */
typedef gboolean (*BUFFER_CALLBACK_T) (GstElement *image_sink, GstBuffer *buffer, GstPad *pad, void *appdata);

static gboolean bus_call (GstBus *bus,  GstMessage *msg,  gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;
 // g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (msg));
  
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}



class CCamera
{
  public:
    char _manufacturer[255];
    char _name[255];
    char _serial[255];
    char _uniquename[255];
  
    CCamera(const char* manufacturer, const char* name, const char* serial);
    ~CCamera();
    virtual bool getColorFormat( int No, char* Format ) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual void SelectFormat( int Index) = 0;
    virtual int GetWidth() = 0;;
    virtual int GetHeight() = 0;;
    virtual int GetDenominator() = 0;;
    virtual int GetNumerator() = 0;
    virtual GMainLoop* GetPipelineLoop() = 0;
    
    void SetWindowHandle( long unsigned int Handle){WindowHandle = Handle;}
    void SetCallback( BUFFER_CALLBACK_T cbfunc, void* Data){_cbfunc = cbfunc;_CallbackData = Data;}
    
    virtual bool SetProperty( int  Property, int  value) =0;
    virtual bool GetPropertyRange( int  Property, int &Min , int &Max) =0;
    virtual bool GetProperty( int  Property, int &value ) =0;
    virtual int GetDroppedFrameCount() { return -1; };
    virtual int GetGoodFrameCount()  { return -1; };
    std::vector<CProperty*> _cProperties;
    void AddProperty( const char* name, int id, int minimum, int maximum, int def, int value, CProperty::VALUE_TYPE type =CProperty::INTEGER);
    
    
  protected:
    /* container holding all elements needed for gstreamer pipeline */
    typedef struct _Video
    {
	GMainLoop* loop;
	GstBus* bus;
	guint bus_watch_id;
	GstElement* pipeline;       /* the actual pipeline */
    } Gstreamer_Pipeline;
    
    Gstreamer_Pipeline _Pipeline;
    
    GstPad *tee_q1_pad;
    GstPad *tee_q2_pad;
    GstPad *q1_pad; 
    GstPad *q2_pad;

    
    bool InstantiateGSTElement( const gchar *FactoryName, const gchar *Name, bool *success );
    virtual bool InstantiateGSTElements() = 0; // Each camera class is responsible on its own for the elements to be used
    GstElement* GetElement(const gchar *Name);

    long unsigned int WindowHandle;
    BUFFER_CALLBACK_T _cbfunc;
    void *_CallbackData;

  private:
    bool SetAuto(const gchar* Elementname, const gchar* Property, bool OnOff );
  
};


#endif
