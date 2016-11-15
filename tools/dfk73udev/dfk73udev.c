#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>

#include "dfk73.h"

#define MAXBUS 64
#define MAXDEV 255

int udev_get_devaddr(const char* devfile, int *busnum, int *devnum)
{
   struct udev *udev;
   struct udev_enumerate *enumerate;
   struct udev_list_entry *devices, *dev_list_entry;
   struct udev_device *dev;
   struct udev_device *pdev;
   int ret = -1;

   udev = udev_new ();
   if (!udev)
      return 0;

   enumerate = udev_enumerate_new (udev);
   udev_enumerate_add_match_subsystem (enumerate, "video4linux");
   udev_enumerate_scan_devices (enumerate);
   devices = udev_enumerate_get_list_entry (enumerate);
   udev_list_entry_foreach (dev_list_entry, devices){
      const char *path;

      path = udev_list_entry_get_name (dev_list_entry);
      dev = udev_device_new_from_syspath (udev, path);

      if (!strcmp (udev_device_get_devnode (dev), devfile)){
	 pdev = udev_device_get_parent_with_subsystem_devtype (dev, "usb", "usb_device");
	 if (pdev){
	    const char *busnum_s;
	    const char *devnum_s;
	    ret = 0;

	    busnum_s = udev_device_get_sysattr_value(pdev, "busnum");
	    if (busnum_s){
	       *busnum = atoi(busnum_s);
	    } else {
	       ret = -1;
	    }
	    devnum_s = udev_device_get_sysattr_value(pdev, "devnum");
	    if (devnum_s){
	       *devnum = atoi(devnum_s);
	    } else {
	       ret = -1;
	    }
	 }
      }
      udev_device_unref(dev);
   }
   /* Free the enumerator object */
   udev_enumerate_unref(enumerate);

   udev_unref(udev);

   return ret;
}

int main(int argc, char **argv)
{
   int busnum, devnum;
   int ret;
   int shmfd;
   unsigned char *devmap;
   struct stat shms;

   if(argc < 2)
   {
      fprintf(stderr, "Usage: %s <device>\n",
	      argv[0]);
      return -1;
   }

   ret = udev_get_devaddr(argv[1], &busnum, &devnum);

   FILE *f;
   f = fopen("/tmp/dfk73udev.txt", "a");
   fprintf(f, "arg: %s, Busnum: %d, Devnum: %d Uid: %d ret: %d\n", argv[1], busnum, devnum, getuid(), ret);

   shmfd = shm_open("/dfk73udev", O_RDWR | O_CREAT, 0666);
   fstat(shmfd, &shms);
   if(shms.st_size == 0)
   {
      ftruncate(shmfd, MAXBUS * MAXDEV);
      devmap = mmap(NULL, MAXBUS * MAXDEV, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
      memset(devmap, 0x0, MAXBUS * MAXDEV);
   } else {
      devmap = mmap(NULL, MAXBUS * MAXDEV, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
   }
   if (ret == 0)
   {
      if((busnum >= MAXBUS) || (devnum >= MAXDEV))
      {
	 fprintf(f, "Invalid busnum / devnum\n");
      } else {
	 if(devmap[busnum * MAXDEV + devnum])
	 {
	    fprintf(f, "Device already configured\n");
	 } else {
	    fprintf(f, "Configuring device\n");
	    dfk73_prepare(busnum, devnum);
	    devmap[busnum * MAXDEV + devnum] = 1;
	 }
      }
   }

   close(shmfd);

   dfk73_v4l2_prepare(argv[1]);

   fclose(f);
   fprintf(f, "success in doing stuff\n");

   return 0;
}
