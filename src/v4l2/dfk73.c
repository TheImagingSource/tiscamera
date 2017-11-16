#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/uvcvideo.h>
#include <linux/videodev2.h>
#include <stropts.h>

#include <libusb-1.0/libusb.h>

#include "dfk73.h"

#define ID_VENDOR 0x199e
#define ID_PRODUCT 0x8221

struct dfk73_devinfo
{
    libusb_device_handle* dev;
    int driver_detached;
};


static struct libusb_device* dfk73_find_device (int index)
{
    ssize_t num_devices;
    libusb_device** devlist;
    libusb_device* dev = NULL;
    int i;
    int ci = 0;

    num_devices = libusb_get_device_list(NULL, &devlist);

    for (i = 0; i < num_devices; i++)
    {
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(devlist[i], &desc) < 0)
        {
            continue;
        }
        printf("Bus: %d Device: %d VID: %04x PID: %04x\n",
               libusb_get_bus_number(devlist[i]),
               libusb_get_device_address(devlist[i]),
               desc.idVendor, desc.idProduct);

        if ((desc.idVendor == ID_VENDOR ) &&
           (desc.idProduct == ID_PRODUCT))
        {
            if (ci == index)
            {
                dev = devlist[i];
                libusb_ref_device(dev);
                break;
            }
            ci++;
        }
    }

    libusb_free_device_list(devlist, 1);

    return dev;
}


static struct libusb_device* dfk73_get_device (int bus, int devnum)
{
    ssize_t num_devices;
    libusb_device** devlist;
    libusb_device* dev = NULL;
    int i;

    num_devices = libusb_get_device_list(NULL, &devlist);

    for (i = 0; i < num_devices; i++)
    {
        if ((libusb_get_bus_number(devlist[i]) == bus) &&
           (libusb_get_device_address(devlist[i]) == devnum))
        {
            dev = devlist[i];
            libusb_ref_device(dev);
            break;
        }
    }

    libusb_free_device_list(devlist, 1);

    return dev;
}


static struct dfk73_devinfo* dfk73_open (struct libusb_device* udev)
{
    struct dfk73_devinfo* devinfo = NULL;
    libusb_device_handle* dev;
    char driver_name[64];
    int driver_detached = 0;
    int itf = 0;
    int r;

    driver_name[0] = 0;

    if (libusb_open(udev, &dev) < 0)
    {
        fprintf(stderr, "Unable to open device.\n");
        return NULL;
    }

    for (itf = 0; itf < 1; itf++)
    {
        if (libusb_claim_interface (dev, itf) < 0)
        {
            // This error is OK when a video driver is loaded
            /* fprintf(stderr, "Unable to claim interface %d, detaching
             * uvcvideo\n", */
            /* itf); */
            r = libusb_detach_kernel_driver( dev, itf);
            if (libusb_claim_interface (dev, itf) < 0)
            {
                libusb_close(dev);
                fprintf(stderr, "Unable to claim interface %d, detaching uvcvideo\n",
                        itf);
                return NULL;
            }
            driver_detached = 1;
        }
    }

    devinfo = calloc(1, sizeof(struct dfk73_devinfo));
    devinfo->driver_detached = driver_detached;
    devinfo->dev = dev;

    return devinfo;
}


static void dfk73_close (struct dfk73_devinfo* devinfo)
{
    int itf = 0;
    int r;

    if (devinfo->driver_detached)
    {
        r = libusb_release_interface(devinfo->dev, itf);
        for(itf = 0; itf < 1; itf++)
        {
            r = libusb_attach_kernel_driver (devinfo->dev, itf);
        }
    }
    free(devinfo);
}


static
int dfk73_xu_register_read (struct dfk73_devinfo* devinfo,
                            uint16_t addr)
{
    int ret;
    unsigned int SET_CUR = 0x1;
    unsigned int GET_CUR = 0x81;

    libusb_device_handle *devh = devinfo->dev;

    unsigned char data[5];
    data[0] = 0;
    data[1] = addr & 0xff;
    data[2] = (addr >> 8) & 0xff;
    data[3] = 0;
    data[4] = 0;

    ret = libusb_control_transfer(devh,
                                  LIBUSB_ENDPOINT_OUT |
                                  LIBUSB_REQUEST_TYPE_CLASS |
                                  LIBUSB_RECIPIENT_INTERFACE,
                                  SET_CUR,
                                  2 << 8,
                                  0x4 << 8,
                                  data, 5, 10000);

    if (ret != 5)
    {
        fprintf(stderr, "Error setting xu control: %d\n", ret);
        return -1;
    }

    memset(data, 0x0, sizeof(data));

    ret = libusb_control_transfer(devh,
                                  LIBUSB_ENDPOINT_IN |
                                  LIBUSB_REQUEST_TYPE_CLASS |
                                  LIBUSB_RECIPIENT_INTERFACE,
                                  GET_CUR,
                                  2 << 8,
                                  0x4 << 8,
                                  data, sizeof(data), 10000);

    if (ret != 5)
    {
        printf ("Error getting xu control: %d\n", ret);
        return -1;
    }

    return data[1];
}


static void dfk73_init (void)
{
    libusb_init(NULL);
}


void dfk73_prepare_all(void)
{
    int idx = 0;
    libusb_device* usbdev;

    dfk73_init();

    for (usbdev = dfk73_find_device(idx++);
         usbdev;
         usbdev = dfk73_find_device(idx++))
    {
        struct dfk73_devinfo* dev;
        dev = dfk73_open(usbdev);
        // Make a dummy write to allow XU to be configured by uvcvideo driver
        dfk73_xu_register_read(dev, 0x3F0C);
        dfk73_close(dev);
        libusb_unref_device(usbdev);
    }
}


int dfk73_prepare (int bus, int devnum)
{
    libusb_device* usbdev;
    struct dfk73_devinfo* dev;
    int ret = 1;

    dfk73_init();
    usbdev = dfk73_get_device(bus, devnum);
    dev = dfk73_open(usbdev);
    // Make a dummy write to allow XU to be configured by uvcvideo driver
    if (dfk73_xu_register_read(dev, 0x3F0C) != -1)
    {
        ret = 0;
    }
    dfk73_close(dev);
    libusb_unref_device(usbdev);

    return ret;
}


/* int dfk73_v4l2_prepare (char* devfile) */
/* { */
/*     int ret; */

/*     // Map the EM27595Register control */
/*     unsigned char uvcctl_data[] = { */
/*         0x20, 0x09, 0x98, 0x00, 0x45, 0x4d, 0x32, 0x37, 0x35, 0x39, 0x35, */
/*         0x52, 0x65, 0x67, 0x69, 0x73, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00, */
/*         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, */
/*         0x00, 0x00, 0x00, 0x52, 0xf2, 0xb8, 0xaa, 0xd1, 0x8e, 0x72, 0x49, */
/*         0x8c, 0xed, 0x96, 0xb1, 0x7f, 0x04, 0x40, 0x8b, 0x02, 0x20, 0x00, */
/*         0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, */
/*         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, */
/*         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, */
/*         0x00, 0x00, 0x00, 0x00 */
/*     }; */

/*     int fd; */

/*     fd = open(devfile, O_RDWR); */
/*     ret = ioctl(fd, UVCIOC_CTRL_MAP, uvcctl_data); */
/*     if (ret == -1) */
/*     { */
/*         perror("Failed to map control"); */
/*     } */
/*     close(fd); */

/*     return ret; */
/* } */


int dfk73_v4l2_prepare (char* devfile)
{
    int ret;

    // Map the EM27595Register control
    unsigned char uvcctl_data[] = {
        0x20, 0x09, 0x98, 0x00, 0x45, 0x4d, 0x32, 0x37, 0x35, 0x39, 0x35,
        0x52, 0x65, 0x67, 0x69, 0x73, 0x74, 0x65, 0x72, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x52, 0xf2, 0xb8, 0xaa, 0xd1, 0x8e, 0x72, 0x49,
        0x8c, 0xed, 0x96, 0xb1, 0x7f, 0x04, 0x40, 0x8b, 0x02, 0x20, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    int fd;

    fd = open(devfile, O_RDWR);
    ret = ioctl(fd, UVCIOC_CTRL_MAP, uvcctl_data);
    if (ret == -1)
    {
        perror("Failed to map control");
    }
    close(fd);

    return ret;
}


static int dfk73_v4l2_set_register (int fd, uint16_t addr, uint8_t value)
{
    struct v4l2_control ctrl;
    int ret;

    ctrl.id = 0x00980920;
    ctrl.value = addr << 8;

    ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);

    if (ret != -1)
    {
        ctrl.value = value << 8 | 2;

        ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
    }

    return ret;
}


int dfk73_v4l2_set_framerate_index (int fd, int index)
{
    uint8_t value;
    if((index < 0) || (index > 3))
    {
        return -1;
    }

    value = ((index+1) << 4) | 1;

    return dfk73_v4l2_set_register(fd, 0x3035, value);
}
