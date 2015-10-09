#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "camerabase.h"

CCamera::CCamera(const char* manufacturer, const char* name,const char* serial)
{
  if( manufacturer != NULL )
    strcpy(_manufacturer, manufacturer);
  else
    strcpy(_manufacturer, "Unknown Manufacturer");
  
  if( name != NULL )
    strcpy(_name, name);
  else
    strcpy(_name,"Unknown Model");

  if( serial != NULL )
    strcpy(_serial, serial);
  else
    strcpy(_serial, "No serial");
  

  strcpy(_uniquename,name);
  strcat(_uniquename," ");
  strcat(_uniquename,_serial);
  
  WindowHandle = 0;
  _Pipeline.loop = g_main_loop_new (NULL, FALSE);;
  _Pipeline.pipeline = NULL;
 
}

CCamera::~CCamera()
{
   g_main_loop_unref (_Pipeline.loop);  
     // Clean properties
  for( std::vector<CProperty*>::iterator it = _cProperties.begin(); it != _cProperties.end(); it++ )
  {
    delete *it;
  }
  _cProperties.clear();

}

// Gstreamer parts

///////////////////////////////////////////////////////////////////////
// Create the elements and add them to the elements list
//
bool CCamera::InstantiateGSTElement( const gchar *FactoryName, const gchar *Name, bool *success )
{
  if( !*success) return false;
  
  GstElement *newElement =  gst_element_factory_make(FactoryName, Name);
  if( !newElement )
  {
      g_printerr ("The pipeline element \"%s\", \"%s\" could not be created.\n",FactoryName, Name);
      *success = false;
      return false;
  }
  else
  {
    gst_bin_add(GST_BIN (_Pipeline.pipeline), newElement );
  }

  return true;
}


///////////////////////////////////////////////////////////////////////
// Search an element by Name- only for convinience.
//
GstElement* CCamera::GetElement(const gchar *Name)
{
  if( _Pipeline.pipeline != NULL )
  {
    //g_print("Get module \"%s\".\n", Name);
    if( gst_bin_get_by_name(GST_BIN (_Pipeline.pipeline),Name )== NULL )
    {
		g_print("Module \"%s\" is null.\n", Name);
    }
    return gst_bin_get_by_name(GST_BIN (_Pipeline.pipeline),Name );
  }
  else
  {
    //g_print("Get module %s no pipeline available.\n", Name);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////
// Add a new property to the internal property list.
//
void CCamera::AddProperty( const char* name, int id, int minimum, int maximum, int def, int value,CProperty::VALUE_TYPE type )
{
  _cProperties.push_back( new CProperty( name, id, minimum, maximum, def, value, type));
}





