#ifndef __AFU050_H__
#define __AFU050_H__

typedef struct afu050_handle *afu050_handle_t;

typedef enum
{
	AFU050_FMT_2592X1944 = 1,
	AFU050_FMT_1920X1080 = 2,
	AFU050_FMT_1280X960 = 3 
}afu050_video_format_t;

	
typedef int (*afu050_new_frame_callback_t) (afu050_handle_t handle, 
					   unsigned char *jpegbuf,
					   size_t bufsize,
					   void *user_data);

/*
  afu050_open: Aqcuire handle to TIS_AFU050 camera

  returns: handle on success, NULL otherwise
 */
extern "C" afu050_handle_t afu050_open(void);

/*
  afu050_close: Release allocated resources of a camera
  
  handle: handle to camera, as returned by afu050_open
*/
extern "C" void afu050_close(afu050_handle_t handle);

/*
  afu050_set_video_format

  handle: handle to camera
  format: video format to be set

  returns: 0 on success, error value on failure
*/
extern "C" int afu050_set_video_format(afu050_handle_t handle, afu050_video_format_t format);

/*
  afu050_capture_start: Start capturing image data

  handle: handle to camera
  cb: callback to be called when a new frame is captured
  user_data: a data pointer which is provided to the callback function

  returns: 0 on success, error value on failure
*/
extern "C" int afu050_capture_start (afu050_handle_t handle, 
			 afu050_new_frame_callback_t cb, 
			 void *user_data);
/* 
   afu050_capture_stop: Stop capturing image data

   handle: handle to camera

   returns: 0 on success, error value on failure
*/   
extern "C" int afu050_capture_stop(afu050_handle_t handle);

extern "C" int afu050_set_exposure_auto(afu050_handle_t handle, int auto_value);
extern "C" int afu050_get_exposure_auto(afu050_handle_t handle, int *auto_value);
extern "C" int afu050_set_exposure_time(afu050_handle_t handle, unsigned int exposure_us);
extern "C" int afu050_get_exposure_time(afu050_handle_t handle, int *exposure_us);

extern "C" int afu050_set_gain_auto(afu050_handle_t handle, int auto_value);
extern "C" int afu050_get_gain_auto(afu050_handle_t handle, int *auto_value);
extern "C" int afu050_set_gain(afu050_handle_t handle, unsigned short gain);
extern "C" int afu050_get_gain(afu050_handle_t handle, unsigned short *gain);

extern "C" int afu050_set_white_balance_auto(afu050_handle_t handle, int auto_value);
extern "C" int afu050_get_white_balance_auto(afu050_handle_t handle, int *auto_value);
extern "C" int afu050_set_white_balance_components(afu050_handle_t handle, 
				       unsigned short red, 
				       unsigned short green, 
				       unsigned short blue);
extern "C" int afu050_get_white_balance_components(afu050_handle_t handle, 
				       unsigned short *red, 
				       unsigned short *green, 
				       unsigned short *blue);


extern "C" int afu050_auto_focus (afu050_handle_t handle);
extern "C" int afu050_set_focus_zone (afu050_handle_t handle, int zoneX, int zoneY);


#endif//__AFU050_H__
