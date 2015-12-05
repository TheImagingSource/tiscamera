/*
 * Copyright 2015 bvtest <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <dirent.h> 
#include <gst/gst.h>
#include <glib.h>
#include <gst/interfaces/xoverlay.h>


#include "cgigecamera.h"

#ifdef USE_ARAVIS_GIGE
  #include <arv.h>
#endif

CGigECamera::CGigECamera(const char* foundname):
CCamera( "",foundname,"")
{
    strcpy(_DevicePath, ""); 
    strcpy(_ProductID, "");
    query_formats();
    
}

void CGigECamera::query_formats()
{
}

// Query the avaialble color formats / pixel formats.
bool CGigECamera::getColorFormat( int Index, char* Format )
{
    if( Index >= 0 && Index < _Formats.size() )
    {
      sprintf( Format,"%s (%dx%d) @ %d/%d",_Formats[Index]->fourcc, _Formats[Index]->width,_Formats[Index]->height,_Formats[Index]->denominator,_Formats[Index]->numerator );
      return true;
    }

  return false;
}

void CGigECamera::SelectFormat( int Index)
{
  if( Index >= 0 && Index < _Formats.size() )
    _SelectedCamera = Index;
  else
    _SelectedCamera = -1;

}

int CGigECamera::GetWidth()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->width;

  return 0;
}

int CGigECamera::GetHeight()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->height;

  return 0;
}

int CGigECamera::GetDenominator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->denominator;

  return 0;
}

int CGigECamera::GetNumerator()
{
  if( _SelectedCamera > -1 )
    return _Formats[_SelectedCamera]->numerator;

  return 0;
}

/////////////////////////////////////////////////////////////////////
bool CGigECamera::start()
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


bool CGigECamera::stop()
{
  UnlinkPipeline();
  return true;
}

GMainLoop* CGigECamera::GetPipelineLoop()
{
  return _Pipeline.loop;
}



bool CGigECamera::CreatePipeline()
{
  return false;
}

bool CGigECamera::UnlinkPipeline()
{
  return false;
}

bool CGigECamera::InstantiateGSTElements()
{
  return false;
}











bool CGigECamera::GetPropertyRange( int  Property, int &Min , int &Max)
{
  return false;
}

bool CGigECamera::SetProperty( int  Property, int  value)
{
    bool result = true;

    return result;
}

bool CGigECamera::GetProperty( int  Property, int &value )
{
    bool result = true;

    return result;
}

