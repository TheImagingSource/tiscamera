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

#ifndef CGIGECAMERA_H
#define CGIGECAMERA_H
#include "camerabase.h"

class CGigECamera : public CCamera
{
    public:
    CGigECamera(const char* foundname);
    
    virtual bool getColorFormat( int No, char* Format );
    
    virtual bool start();
    virtual bool stop();
    
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
    
    
    int _SelectedCamera;

    virtual void query_formats();
    virtual bool CreatePipeline();
    virtual bool UnlinkPipeline();
    virtual bool InstantiateGSTElements();



};

#endif // CGIGECAMERA_H
