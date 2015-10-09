#ifndef CCameraList_H
#define CCameraList_H

#include "camerabase.h"
#include <vector>


class CCameraList
{
  private:
    std::vector<CCamera*> _Cameras;
    int enumerate_cameras_V4L2();
    int enumerate_cameras_afu050();
    int enumerate_cameras_V4L2_generic();
    int enumerate_cameras_aravis();
    
  public:
    ~CCameraList();
    int enumerate_cameras();
    const char* getUniqueCameraName( int i );
    const char* getCameraPath( int i );
    const char* getPathforUniqueName( const char* UniqueName );
    CCamera*  SelectCamera( const char* UniqueName);
    
};
#endif