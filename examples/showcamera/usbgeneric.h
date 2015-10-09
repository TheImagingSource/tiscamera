#ifndef CUSBGENERIC_H
#define CUSBGENERIC_H
#include "usbcamera.h"
#include <vector>

class CUSBGenericCamera : public CUSBCamera
{
  
  public:
    CUSBGenericCamera(const char* path,const char* manufacturer, const char* name, const char* serial, const char* ProductID);
    
  protected:
    virtual bool CreatePipeline();
    virtual bool InstantiateGSTElements();
    virtual bool  UnlinkPipeline();
    virtual void query_formats();
};

#endif