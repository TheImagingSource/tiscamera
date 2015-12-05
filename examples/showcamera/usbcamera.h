#ifndef CUSBCamera_H
#define CUSBCamera_H
#include "camerabase.h"
#include <vector>

class CUSBCamera : public CCamera
{
  
  public:
    CUSBCamera(const char* path,const char* manufacturer, const char* name, const char* serial, const char* ProductID, bool Color);
    
    virtual bool getColorFormat( int No, char* Format );
    
    virtual bool start();
    virtual bool stop();
    
    int GetDroppedFrameCount();
    int GetGoodFrameCount();
    
    virtual GMainLoop* GetPipelineLoop();


    virtual void SelectFormat( int Index);
    int GetWidth();
    int GetHeight();
    int GetDenominator();
    int GetNumerator();
    char _DevicePath[255];
    char _ProductID[255];
    virtual bool SetProperty( int  Property, int  value);
    virtual bool GetPropertyRange( int  Property, int &Min , int &Max);
    virtual bool GetProperty( int  Property, int &value );

   
  protected:

    typedef struct
    {
	const char* device_name;
	int width;
	int height;
	int denominator;
	int numerator;
	char fourcc[5];
	uint pixelformat;
    } Format_t;


    std::vector<Format_t *> _Formats;
	bool _HasFocus;
	bool _Color;
    
    int _SelectedCamera;
    virtual void query_formats();

    bool CreatePipeline();
    bool UnlinkPipeline();
    bool InstantiateGSTElements();
    
    int tis_xioctl (int fd, int request, void *arg);
    bool SetAuto( const gchar* Elementname, const gchar* Property, bool OnOff );
    bool SetGstProperty( const gchar* Elementname, const gchar* Property, int value );
   
};

#endif