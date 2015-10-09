#include "config.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <glib.h>

#include <dirent.h> 
#include <sys/ioctl.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/mman.h> 
#include <sys/time.h>
#include <fcntl.h> 
#include <unistd.h>
#include <linux/videodev2.h>
#include <libudev.h>


#include "CameraList.h"
#include "usbgeneric.h"

#include "afu050.h"
#include "dfkafu050.h"

#ifdef USE_ARAVIS_GIGE
  #include <arv.h>
  #include "cgigecamera.h"
#endif


CCameraList::~CCameraList()
{
  for( std::vector<CCamera*>::iterator it = _Cameras.begin(); it != _Cameras.end(); it++ )
  {
    delete *it;
  }
  _Cameras.clear();
}

int CCameraList::enumerate_cameras()
{
  _Cameras.clear();

  int cameracount = enumerate_cameras_V4L2();
  cameracount += enumerate_cameras_afu050();
  cameracount += enumerate_cameras_V4L2_generic();
  cameracount += enumerate_cameras_aravis();
  
  return cameracount;
}

int CCameraList::enumerate_cameras_V4L2()
{
    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices;
    struct udev_list_entry* dev_list_entry;
    struct udev_device* dev;
	bool Color = false;

    int camera_cnt = 0;

    udev = udev_new();
    
        /* Create a list of the devices in the 'video4linux' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char* path;
        char needed_path[100];

	
        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/

        /* we need to copy the devnode (/dev/videoX) before the path
           is changed to the path of the usb device behind it (/sys/class/....) */
        strcpy(needed_path, udev_device_get_devnode(dev));

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        if (!dev)
        {
            return 0;
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passe
d into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the directory which represents
           the USB device. Note that USB strings are Unicode, UCS2
           encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */

        if (strcmp(udev_device_get_sysattr_value(dev, "idVendor"), "199e") == 0)
        {
			char product[255];
            g_print("%s %s %s %s\n",needed_path,
			      udev_device_get_sysattr_value(dev, "manufacturer"),
			      udev_device_get_sysattr_value(dev, "product"),
			      udev_device_get_sysattr_value(dev, "serial"));

	    strcpy( product, udev_device_get_sysattr_value(dev, "product"));
	    
	    // Here we do a simple decision using the camera name: "DMK" always mono, all other is color.
		
		
	    if( product[1] !='M' )
	    {
			Color = true;
	    }
	    else
	    {				 
			Color = false;
	    }
		CUSBCamera *cam = new CUSBCamera(needed_path,
				udev_device_get_sysattr_value(dev, "manufacturer"),
				product,
				udev_device_get_sysattr_value(dev, "serial"),
				udev_device_get_sysattr_value(dev, "idProduct"),
				Color		);
		_Cameras.push_back(cam);

	    
            camera_cnt++;
        }
        udev_device_unref(dev);
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return camera_cnt;
}



int CCameraList::enumerate_cameras_V4L2_generic()
{
    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices;
    struct udev_list_entry* dev_list_entry;
    struct udev_device* dev;

    int camera_cnt = 0;

    udev = udev_new();
    
        /* Create a list of the devices in the 'video4linux' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char* path;
        char needed_path[100];

	
        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/

        /* we need to copy the devnode (/dev/videoX) before the path
           is changed to the path of the usb device behind it (/sys/class/....) */
        strcpy(needed_path, udev_device_get_devnode(dev));

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        if (!dev)
        {
            return 0;
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passe
d into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the directory which represents
           the USB device. Note that USB strings are Unicode, UCS2
           encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */

        if (strcmp(udev_device_get_sysattr_value(dev, "idVendor"), "199e") != 0)
        {
	    char product[255];
            g_print("%s %s %s %s\n",needed_path,
			      udev_device_get_sysattr_value(dev, "manufacturer"),
			      udev_device_get_sysattr_value(dev, "product"),
			      udev_device_get_sysattr_value(dev, "serial"));

	    strcpy( product, udev_device_get_sysattr_value(dev, "product"));
	    
	    CUSBGenericCamera *cam = new CUSBGenericCamera(needed_path,
				      udev_device_get_sysattr_value(dev, "manufacturer"),
				      product,
				      udev_device_get_sysattr_value(dev, "serial"),
				      udev_device_get_sysattr_value(dev, "idProduct")
					    );
	    _Cameras.push_back(cam);
	    
	    
	     camera_cnt++;
        }
        udev_device_unref(dev);
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return camera_cnt;
}




int CCameraList::enumerate_cameras_afu050()
{
  afu050_handle_t handle;
  int camera_cnt = 0; 
#ifdef USE_AFU050
  handle = afu050_open();
  if (handle)
  {
    CDFKAFU050 *cam = new CDFKAFU050("","The Imaging Source Europe GmbH","DFK AFU050","00000000");
    _Cameras.push_back(cam);
    afu050_close(handle);	    
    camera_cnt++;
    
  }
#endif  
  return camera_cnt;
}

/////////////////////////////////////////////////////////////////////////
// Enumerate GigE cameras

int CCameraList::enumerate_cameras_aravis()
{
  int camera_cnt = 0; 
  
#ifdef USE_ARAVIS_GIGE
  unsigned int n_devices;
  unsigned int i;
  printf("Enumerating GigE cameras\n");

  arv_update_device_list ();
  n_devices = arv_get_n_devices ();
  for (i = 0; i < n_devices; i++) 
  {
    printf("GigE : %s\n", arv_get_device_id(i));
    
    CGigECamera *cam = new CGigECamera( arv_get_device_id(i) );
    _Cameras.push_back(cam);
     camera_cnt++;
  }
#endif
  return camera_cnt;
}



const char* CCameraList::getUniqueCameraName( int i )
{
    if( _Cameras[i]->_uniquename == NULL )
    {
      printf("%d, %s\n",i, _Cameras[i]->_name);
    }
    return _Cameras[i]->_uniquename;
}


const char* CCameraList::getCameraPath( int i )
{
    return "";
}

// Select a camera 
CCamera* CCameraList::SelectCamera( const char* UniqueName)
{
  for( int i = 0; i < _Cameras.size(); i++ )
  {
    if( strcmp( UniqueName, _Cameras[i]->_uniquename) == 0 )
    {
      return _Cameras[i];
      break;
    }
  }
  return NULL;
}

const char* CCameraList::getPathforUniqueName( const char* UniqueName )
{
  int found = -1;
  
  for( int i = 0; i < _Cameras.size(); i++ )
  {
    if( strcmp( UniqueName, _Cameras[i]->_uniquename) == 0 )
    {
      found = i;
      break;
    }
  }
  if( found >= 0 )
  {
    return "";//_Cameras[found]._path;
  }
  
  return "";
}
