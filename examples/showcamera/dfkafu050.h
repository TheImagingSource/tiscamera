#ifndef CDFKAFU050_H
#define CDFKAFU050_H
#include "camerabase.h"
#include <vector>
#include <string>

#include "afu050.h"

class CDFKAFU050 : public CCamera
{
  
  public:
    CDFKAFU050(const char* path,const char* manufacturer, const char* name, const char* serial);
    
    virtual bool getColorFormat( int No, char* Format );
    
    virtual bool start();
    virtual bool stop();
    
    virtual GMainLoop* GetPipelineLoop();

    virtual bool SetProperty( int  Property, int  value);
    virtual bool GetPropertyRange( int  Property, int &Min , int &Max);
    virtual bool GetProperty( int  Property, int &value );
    //virtual int GetDroppedFrameCount(){return 0;} ;
    //virtual int GetGoodFrameCount(){return 0;} ;


    virtual void SelectFormat( int Index);
    int GetWidth();
    int GetHeight();
    int GetDenominator();
    int GetNumerator();
    char _DevicePath[255];
    
  private:
    afu050_handle_t _handle;
    
   class cFormat
    {
	public:
	  cFormat( int w, int h, int d, int n, afu050_video_format_t f)
	  {
	    width = w;
	    height = h;
	    denominator = d;
	    numerator = n;
	    internalFormat = f;
	  }

	  int width;
	  int height;
	  int denominator;
	  int numerator;
	  afu050_video_format_t internalFormat;
    }; 

        std::vector<cFormat> _Formats;
    
    
    int _SelectedCamera;
    void query_formats();

    bool CreatePipeline();
    bool UnlinkPipeline();
    virtual bool InstantiateGSTElements();   
};

#endif