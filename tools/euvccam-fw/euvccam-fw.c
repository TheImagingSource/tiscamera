/*
 * Copyright 2013 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <glob.h>
#include <usb.h>
#include <byteswap.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>

struct caminfo
{
	unsigned short idVendor;
	unsigned short idProduct;
	int mode_flag;
	int trigger;
	char *firmware_file;
};

typedef enum{
	CAMERA_INTERFACE_MODE_PROPRIETARY,
	CAMERA_INTERFACE_MODE_UVC,
} euvc_mode_t;

#define DEV_DXK21xUC_OBC	0x199e, 0x8201
#define DEV_DXK21xUC		0x199e, 0x8202
#define DEV_DXK21xUC_UVC	0x199e, 0x8302
#define DEV_DFK61xUC            0x199e, 0x8203
#define DEV_DMK41xUC            0x199e, 0x8204
#define DEV_DFK51xUC            0x199e, 0x8205
#define DEV_DFK41xUC            0x199e, 0x8206
#define DEV_DFK71xUC            0x199e, 0x8207
#define DEV_DFK71xUC_UVC        0x199e, 0x8307
#define DEV_DFK42xUC            0x199e, 0x8208
#define DEV_DFK42xUC_UVC        0x199e, 0x8308
#define DEV_EMPIA               0xeb1a, 0x2760

#define EEPROM_SIZE 16384
#define DATA_SIZE   512
#define FIRMWARE_END (EEPROM_SIZE - DATA_SIZE)
#define FLAGS_LOCATION (FIRMWARE_END-1)
#define PID_LOCATION_LO (0x5a)
#define PID_LOCATION_HI (0x5b)

#define min(a,b)			((a<b)?(a):(b))


static char *read_string( usb_dev_handle *dev, int index );
static struct usb_device *euvc_find_device(struct caminfo *camera_info);
static unsigned char euvc_read_vendor_register( usb_dev_handle *dev, unsigned char reg );
static usb_dev_handle *euvc_open(struct usb_device *udev);
static void euvc_close(usb_dev_handle *dev);


struct caminfo camera_infos[] =
{
	{ DEV_DXK21xUC_OBC, 0, 0, "files/dfk21auc03_obc_[0-9]*.euvc" },
	{ DEV_DXK21xUC, 1, 0,     "files/dmk21auc03_[0-9]*.euvc" },
	{ DEV_DXK21xUC, 2, 0,     "files/dfk21auc03_[0-9]*.euvc" },
	{ DEV_DXK21xUC_UVC, 1, 0, "files/dmk21auc03_[0-9]*.euvc" },
	{ DEV_DXK21xUC_UVC, 2, 0, "files/dfk21auc03_[0-9]*.euvc" },
	{ DEV_DXK21xUC, 1, 1,     "files/dmk21buc03_[0-9]*.euvc" },
	{ DEV_DXK21xUC, 2, 1,     "files/dfk21buc03_[0-9]*.euvc" },
	{ DEV_DFK61xUC, 2, 0,     "files/dfk61auc02_[0-9]*.euvc" },
	{ DEV_DFK61xUC, 2, 1,     "files/dfk61buc02_[0-9]*.euvc" },
	{ DEV_DMK41xUC, 1, 0,     "files/dmk41auc02_[0-9]*.euvc" },
	{ DEV_DMK41xUC, 2, 1,     "files/dmk41buc02_[0-9]*.euvc" },
	{ DEV_DFK51xUC, 2, 0,     "files/dfk51buc02_[0-9]*.euvc" },
	{ DEV_DFK41xUC, 2, 0,     "files/dfk41buc02_[0-9]*.euvc" },
	{ DEV_DFK71xUC, 1, 0,     "files/dmk72uc02_[0-9]*.euvc" },
	{ DEV_DFK71xUC_UVC, 1, 0, "files/dmk72uc02_[0-9]*.euvc" },
	{ DEV_DFK71xUC, 2, 0,     "files/dfk72uc02_[0-9]*.euvc" },
	{ DEV_DFK71xUC_UVC, 2, 0, "files/dfk72uc02_[0-9]*.euvc" },
	{ DEV_DFK42xUC, 1, 0,     "files/dfk42uc02_[0-9]*.euvc" },
	{ DEV_DFK42xUC_UVC, 1, 0,     "files/dfk42uc02_[0-9]*.euvc" },
	{ DEV_EMPIA,    2, 0,     "none" },

};

struct options
{
	gboolean upload;
	gboolean download;
	gboolean print_eeprom;
	gboolean get_version;
	gboolean force;
	gchar *mode;
};

static struct options options = {
  .upload = FALSE,
  .download = FALSE,
  .print_eeprom = FALSE,
  .get_version = FALSE,
  .force = FALSE,
  .mode = NULL,
};


static GOptionEntry options_entries[] = {
	{ "upload", 'u', 0, G_OPTION_ARG_NONE, &options.upload, "Upload", NULL },
	{ "printeeprom", 'p', 0, G_OPTION_ARG_NONE, &options.print_eeprom, "Print eeprom contents", NULL },
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &options.get_version, "Get Firmware version", NULL },
	{ "force", 'f', 0, G_OPTION_ARG_NONE, &options.force, "Force firmware upload (might break device!)", NULL},
	{ "mode", 'm', 0, G_OPTION_ARG_STRING, &options.mode, "Set interface mode", NULL },
	{ NULL }
};





static struct usb_device *euvc_find_device(struct caminfo *camera_info)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	char name[10];

	for( bus = usb_get_busses(); bus != NULL; bus = bus->next ){
		for( dev = bus->devices; dev != NULL; dev = dev->next ){
			int i;

			for( i = 0; i < ( sizeof( camera_infos ) / sizeof( camera_infos[0] ) ); i++ ){
				if( ( dev->descriptor.idVendor == camera_infos[i].idVendor ) &&
				    ( dev->descriptor.idProduct == camera_infos[i].idProduct )){
					memcpy( camera_info, &camera_infos[i], sizeof( struct caminfo ) );
/*	       printf( "Found: %x:%x\n", dev->descriptor.idVendor, dev->descriptor.idProduct ); */
					return dev;
				}
			}
		}
	}

	return NULL;
}

static usb_dev_handle *euvc_open(struct usb_device *udev)
{
	usb_dev_handle *dev;

	dev = usb_open(udev);
	if (dev == NULL) {
		printf("Unable to open device.\n");
		return NULL;
	}

	if( usb_claim_interface( dev, 0 ) < 0 )
		{
/*       printf("Unable to claim interface 0.\n"); */

			// This error is OK when a video driver is loaded
		}

	return dev;
}

static void euvc_close(usb_dev_handle *dev)
{
	usb_release_interface(dev, 0);
	usb_close(dev);
}


static int euvc_get_firmware_version( usb_dev_handle *dev )
{
	int ret;
	unsigned int SET_CUR = 0x1;
	unsigned int GET_CUR = 0x81;
	unsigned short value;

	// read IIC value
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
			       GET_CUR,
			       0x2d << 8,
			       0x1 << 8,
			       (char*)&value, 2, 10000 );

	return value;
}


static unsigned int euvc_get_capability (usb_dev_handle *dev)
{
	int ret;
	unsigned int SET_CUR = 0x1;
	unsigned int GET_CUR = 0x81;
	unsigned int value;

	// read IIC value
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
			       GET_CUR,
			       0x20 << 8,
			       0x1 << 8,
			       (char*)&value, 4, 10000 );

	return value;
}

static int euvc_read_camconfig( usb_dev_handle *dev, unsigned char *value )
{
	int ret;
	unsigned int SET_CUR = 0x1;
	unsigned int GET_CUR = 0x81;


	// read IIC value
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
			       GET_CUR,
			       0x02 << 8,
			       0x1 << 8,
			       value, 1, 10000 );

	return ret;
}


static int euvc_read_eeprom( usb_dev_handle *dev, unsigned char *value )
{
	int ret;
	unsigned int SET_CUR = 0x1;
	unsigned int GET_CUR = 0x81;


	// read IIC value
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
			       GET_CUR,
			       0x44 << 8,
			       0x1 << 8,
			       value, 4, 10000 );

	return ret;
}

static int usbbuffer_to_string( unsigned char *usbbuffer, int buffer_size, char *string, int string_size )
{
	int s;
	int i;

	s = usbbuffer[0];

	if( s > buffer_size ){
			return -1;
		}

	if( usbbuffer[1] != 0x03 ){
			return -1;
		}

	if( ( (s-2)/2 ) > string_size ){
			return -1;
		}

	for( i = 2; i < s; i+=2 ){
			string[ i/2 - 1 ] = usbbuffer[i];
		}

	return 0;
}

static int string_to_usbbuffer( unsigned char *usbbuffer, int buffer_size, const char *string )
{
	int len = strlen( string ) * 2 + 4;
	int i;

	if( buffer_size < len ){
			return 0;
		}

	usbbuffer[0] = len;
	usbbuffer[1] = 0x03;

	for( i = 0; i < strlen( string ); i++ ){
			usbbuffer[ 2 + ( 2 * i ) ] = string[i];
			usbbuffer[ 3 + ( 2 * i ) ] = 0x0;
		}

	usbbuffer[ 2 + ( 2 * i ) ] = 0x0;
	usbbuffer[ 3 + ( 2 * i ) ] = 0x0;


	return len;
}


static int write_eeprom( usb_dev_handle *dev, int addr, unsigned char *write_buffer, int size, int verify )
{
	int i;
	int ret = 0;
	char buffer[256];
	char vbuf[256];
	int len = ( size < 32 ) ? size : 32;

	memset( buffer, 0xab, sizeof( buffer ) );


	for( i = 0; (i+len) < size; i+=len ){
			buffer[0] = ( (addr+i) >> 8 ) & 0xff;
			buffer[1] = (addr+i) & 0xff;

			memcpy( buffer + 2, write_buffer + i, len );

			ret = usb_control_msg( dev,
					       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x2,
					       0, 0xa1,
					       buffer, len + 2, 10000 );
			if( ret < 0 ){
					fprintf( stderr, "EEProm Write Failed\n" );
					break;
				}
			if( ( i % 1024 ) == 0 ){
/*	 printf( "%dk written\n", i / 1024 ); */
				}


			ret = usb_control_msg( dev,
					       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x2, // No-Stop Serial Bus Request
					       0, 0xa0,
					       buffer, len, 10000 );
			if( ret < 0 ){
					fprintf( stderr, "EEProm Read Failed\n" );
					break;
				}

			usleep( 5000 );
		}

	if( (ret>=0) && ( i < (size ) ) ){
			len = (size-1) - i;

			buffer[0] = ( (addr+i) >> 8 ) & 0xff;
			buffer[1] = (addr+i) & 0xff;

			memcpy( buffer + 2, write_buffer + i, len );

			ret = usb_control_msg( dev,
					       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x2,
					       0, 0xa1,
					       buffer, len + 2, 10000 );
			if( ret < 0 ){
					fprintf( stderr, "EEProm Write Failed\n" );
					return ret;
				}
			if( ( i % 1024 ) == 0 ){
/*	 printf( "%dk written\n", i / 1024 ); */
				}


			ret = usb_control_msg( dev,
					       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x2, // No-Stop Serial Bus Request
					       0, 0xa0,
					       buffer, len, 10000 );
			if( ret < 0 ){
					fprintf( stderr, "EEProm Read Failed\n" );
					return ret;
				}

			usleep( 5000 );
		}


	return ret;
}

static unsigned char euvc_read_vendor_register( usb_dev_handle *dev, unsigned char reg )
{
	int ret;
	unsigned char value[255];
	unsigned int index = reg;
	unsigned char length = 1;

	ret = usb_control_msg(dev,
			      USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0x0, // UVCD Register Request
			      0x0, 
			      index, (char*)value, length, 10000);
	if (ret < 0)
		{
/*       printf( "read_vendor_register request failed!\n" ); */
			return -1;
		}
	return value[0];
}

static int read_eeprom( usb_dev_handle *dev, int addr, unsigned char *buffer, int size )
{
	char tmp[2];
	const int len = (64 < size) ? 64: size;
	int i;
	int ret;

	memset( buffer, 0x0, size );

	tmp[0] = ( addr >> 8 ) & 0xff;
	tmp[1] = addr & 0xff;

	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x3,
			       0, 0xa1,
			       tmp, 2, 10000 );

	usleep( 2000 );

	for( i = 0; i < size; i+=len ){
			ret = usb_control_msg( dev,
					       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x2,
					       0, 0xa0,
					       (char*)buffer+i, len, 10000 );
			if( ret < 0 ){
					perror( "EEProm Read Failed\n" );
					break;
				}
		}

	return ret;
}


static int euvc_write_strings( usb_dev_handle *dev, const char *strvendor, const char *strproduct, const char *strserial )
{
	int len;
	unsigned char config_buffer[512];
	int ret = 0;
	int poss[3];
	unsigned char lens[3];
	const char *strs[3];
	int idxvendor = -1, idxproduct = -1, idxserial = -1;
	int i;

	// Hack to find string indexes of newly uploaded firmware
	for( i = 0; i < 3; i++ ){
		char *tmp = read_string( dev, i );

		if( !strcmp( tmp, "vv" ) ){
			idxvendor = i;
			strs[i] = strvendor;
		}else if( !strcmp( tmp, "pp" ) ){
			idxproduct = i;
			strs[i] = strproduct;
		}else if( !strcmp( tmp, "123" ) ){
			idxserial = i;
			strs[i] = strserial;
		}
	}

	if( ( idxvendor == -1 ) || ( idxproduct == -1 ) || ( idxserial == -1 ) ){
		fprintf( stderr, "Failed to validate string indexes - invalid EEPROM image [%d:%d:%d]\n", idxvendor, idxproduct, idxserial );
		return -1;
	}

	ret = read_eeprom( dev, 0, config_buffer, sizeof( config_buffer ) );
	if( ret < 0 ){
		fprintf( stderr, "Failed to read EEPROM\n" );
		return -1;
	}

	poss[0] = 0xbe;
	lens[0] = string_to_usbbuffer( config_buffer + poss[0], sizeof( config_buffer ) - poss[0], strs[0] );
	poss[1] = poss[0] + lens[0];
	lens[1] = string_to_usbbuffer( config_buffer + poss[1], sizeof( config_buffer ) - poss[1], strs[1] );
	poss[2] = poss[1] + lens[1];
	lens[2] = string_to_usbbuffer( config_buffer + poss[2], sizeof( config_buffer ) - poss[2], strs[2] );
	if( !lens[0] || !lens[1] || !lens[2] ){
		fprintf( stderr, "String too large\n" );
		return -1;
	}

	config_buffer[ 0x60 ] = poss[0] - 0x54;
	config_buffer[ 0x61 ] = lens[0];

	config_buffer[ 0x62 ] = poss[1] - 0x54;
	config_buffer[ 0x63 ] = lens[1];

	config_buffer[ 0x64 ] = poss[2] - 0x54;
	config_buffer[ 0x65 ] = lens[2];

	ret = write_eeprom( dev, 0x0, config_buffer, sizeof( config_buffer ), 0 );


	return ret;
}

static char *read_string( usb_dev_handle *dev, int index )
{
	unsigned char usb_buffer[255];
	static char string[255];
	int strsize = 255;
	int addr, len;

	read_eeprom( dev, 0x60 + ( index * 2 ), usb_buffer, 2 );
	addr = usb_buffer[0] + 0x54;
	len = usb_buffer[1];

	read_eeprom( dev, addr, usb_buffer, len );
	usbbuffer_to_string( usb_buffer, len, string, strsize );

	return string;
}


static int euvc_verify_firmware( usb_dev_handle *dev, char *filename )
{
	FILE *f;
	int i;
	int ret;
	char buffer[256];
	char filebuffer[256];
	const int len = 64;
	unsigned short addr = 0;
	struct stat statbuf;
	int size;

	if( stat( filename, &statbuf ) < 0 ){
			fprintf( stderr, "Cannot stat: %s\n", filename );
			return -1;
		}

	f = fopen( filename, "r" );
	if( !f ){
			fprintf( stderr, "Could not open file for reading\n" );
			return -1;
		}


	size = statbuf.st_size;

	memset( buffer, 0xab, sizeof( buffer ) );
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x2,
			       0, 0xa1,
			       (char*)&addr, 2, 10000 );

	usleep( 2000 );
	for( i = 0; i < size; i+=len ){
			ret = usb_control_msg( dev,
					       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
					       0x3, // No-Stop Serial Bus Request
					       0, 0xa0,
					       buffer, len, 10000 );
			if( ret < 0 ){
					fprintf( stderr, "EEProm Read Failed\n" );
					break;
				}

			if( fread( filebuffer, len, 1, f ) < 1 ){
					fprintf( stderr, "File Read Failed\n" );
					break;
				}

			if( memcmp( buffer, filebuffer, len ) != 0 ){
					printf( "Verification mismatch at offset: %d\n", i );
					fclose( f );
					return 1;
				}
		}

	fclose( f );
	return 0;
}

static int euvc_get_pid (usb_dev_handle *dev, unsigned short *pid)
{
	unsigned short addr = PID_LOCATION_LO;
	int ret;
	unsigned char buffer[2];

	ret = read_eeprom (dev, addr, buffer, 2);
	*pid = buffer[0] | buffer[1] << 8;

	return ret;
}

static int euvc_set_pid (usb_dev_handle *dev, unsigned short pid)
{
	unsigned short addr = PID_LOCATION_LO;
	int ret;
	unsigned char buffer[4];
	buffer[0] = ( addr >> 8 ) & 0xff;
	buffer[1] = addr & 0xff;
	buffer[2] = pid & 0xff;
	buffer[3] = (pid >> 8) & 0xff;

	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x2,
			       0, 0xa1,
			       buffer, sizeof(buffer), 10000 );

	return ret;
}

static int euvc_set_flags (usb_dev_handle *dev, int _flags)
{
	unsigned short addr = FLAGS_LOCATION;
	int ret;
	unsigned char buffer[3];
	buffer[0] = ( addr >> 8 ) & 0xff;
	buffer[1] = addr & 0xff;
	buffer[2] = _flags & 0xff;

	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x2,
			       0, 0xa1,
			       (char*)&addr, 2, 10000 );

	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x2,
			       0, 0xa1,
			       buffer, 3, 10000 );

	return ret;
}


static int euvc_download_eeprom( usb_dev_handle *dev, int size, char *filename )
{
	FILE *f;

	f = fopen( filename, "w" );
	int i;
	int ret;
	char buffer[256];
	const int len = 64;
	unsigned short addr = 0;

	memset( buffer, 0xab, sizeof( buffer ) );
	ret = usb_control_msg( dev,
			       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			       0x2,
			       0, 0xa1,
			       (char*)&addr, 2, 10000 );


	usleep( 2000 );
	for( i = 0; i < size; i+=len ){
		ret = usb_control_msg( dev,
				       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				       0x3, // No-Stop Serial Bus Request
				       0, 0xa0,
				       buffer, len, 10000 );
		if( ret < 0 ){
			fprintf( stderr, "EEProm Read Failed\n" );
			break;
		}
		fwrite( buffer, len, 1, f );
	}

	fclose( f );

	return ret;
}

static unsigned char get_firmware_file_id (char *filename)
{
	FILE *f;
	unsigned char buffer[256];
	unsigned char ret;
	f = fopen( filename, "r" );
	if (!f){
		fprintf (stderr, "Failed to read file '%s'\n", filename);
		return 0;
	}
	if (fread (buffer, PID_LOCATION_LO + 1, 1, f) < (1)){
		fprintf (stderr, "Error reading header of file '%s'\n", filename);
		ret = 0;
	} else {
		ret = buffer[PID_LOCATION_LO];
	}

	fclose (f);
	return ret;
}

static int euvc_upload_eeprom( usb_dev_handle *dev, int size, char *filename, unsigned char firmware_id )
{
	FILE *f;

	f = fopen( filename, "r" );
	int i;
	int ret;
	unsigned char buffer[256];
	char vbuf[256];
	const int len = 32;

	if (fread (buffer, PID_LOCATION_LO + 1, 1, f) < (1)){
		fprintf (stderr, "Error reading header of file '%s'\n", filename);
		fclose (f);
		return -1;
	}

	f = freopen( filename, "r", f );

	if (firmware_id && (firmware_id != buffer[PID_LOCATION_LO])){
		fprintf (stderr, "Error: Firmware ID missmatch: EEprom=%x File=%x\n", firmware_id, buffer[PID_LOCATION_LO]);
		fclose (f);
		return -1;
	}

	memset( buffer, 0xab, sizeof( buffer ) );


	for( i = 0; i < size; i+=len ){
		buffer[0] = ( i >> 8 ) & 0xff;
		buffer[1] = i & 0xff;

		if( fread( buffer + 2, len, 1, f ) < 1 ){
			fprintf( stderr, "Error reading file '%s' @ %d: %s!\n", filename, i, strerror( errno ) );
			break;
		}

		ret = usb_control_msg( dev,
				       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				       0x2,
				       0, 0xa1,
				       buffer, len + 2, 10000 );
		if( ret < 0 ){
			fprintf( stderr, "EEProm Write Failed\n" );
			break;
		}
		if( ( i % 1024 ) == 0 ){
			printf( "%dk written\n", i / 1024 );
		}


		ret = usb_control_msg( dev,
				       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				       0x2, // No-Stop Serial Bus Request
				       0, 0xa0,
				       buffer, len, 10000 );
		if( ret < 0 ){
			fprintf( stderr, "EEProm Read Failed\n" );
			break;
		}

		usleep( 5000 );
	}

	fclose( f );

	return ret;
}

static int euvc_set_mode (usb_dev_handle *dev, euvc_mode_t mode)
{
	unsigned short pid;
	int ret;
	ret = euvc_get_pid (dev, &pid);
	if (ret < 0)
		return ret;
	switch (mode){
	case CAMERA_INTERFACE_MODE_UVC:
		pid = (0x83<<8) | (pid&0xff);
		ret = euvc_set_pid (dev, pid);
		usleep (10000);
		ret |= euvc_set_flags (dev, 1);
		break;
	case CAMERA_INTERFACE_MODE_PROPRIETARY:
		pid = (0x82<<8) | (pid&0xff);
		ret = euvc_set_pid (dev, pid);
		usleep (10000);
		ret |= euvc_set_flags (dev, 0);
		break;
	default:
		fprintf (stderr, "Invalid mode\n");
		break;
	}
	return ret;
}


static int euvc_clear_eeprom( usb_dev_handle *dev, int size )
{
	int i;
	int ret;
	char buffer[256];
	char vbuf[256];
	const int len = 32;

	memset( buffer, 0x0, sizeof( buffer ) );


	for( i = 0; i < size; i+=len ){
		buffer[0] = ( i >> 8 ) & 0xff;
		buffer[1] = i & 0xff;

		ret = usb_control_msg( dev,
				       USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				       0x2,
				       0, 0xa1,
				       buffer, len + 2, 10000 );
		if( ret < 0 ){
			fprintf( stderr, "EEProm Write Failed\n" );
			break;
		}
		if( ( i % 1024 ) == 0 ){
			printf( "%dk written\n", i / 1024 );
		}


		ret = usb_control_msg( dev,
				       USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				       0x2, // No-Stop Serial Bus Request
				       0, 0xa0,
				       buffer, len, 10000 );
		if( ret < 0 ){
			fprintf( stderr, "EEProm Read Failed\n" );
			break;
		}

		usleep( 5000 );
	}


	return ret;
}

static void euvc_print_eeprom_info( usb_dev_handle *dev )
{
	struct usb_device *usbdev = usb_device( dev );
	char buf[128];
	int res = 0;
	unsigned short pid = 0;
	unsigned char flags = 0;

	usb_get_string_simple( dev, usbdev->descriptor.iManufacturer, buf, sizeof( buf ) );
	printf( "Manufacturer String: %s\n", buf );
	usb_get_string_simple( dev, usbdev->descriptor.iProduct, buf, sizeof( buf ) );
	printf( "Product String: %s\n", buf );
	usb_get_string_simple( dev, usbdev->descriptor.iSerialNumber, buf, sizeof( buf ) );
	printf( "Serial Number String: %s\n", buf );
	euvc_get_pid (dev, &pid);
	printf( "PID: %x\n", pid);
	read_eeprom( dev, FLAGS_LOCATION, &flags, 1);
	printf( "Flags: %x\n", flags);
}

static void euvc_get_strings( usb_dev_handle *dev, char *strvendor, char *strproduct, char *strserial, int bufsize )
{
	struct usb_device *usbdev = usb_device( dev );

	usb_get_string_simple( dev, usbdev->descriptor.iManufacturer, strvendor, bufsize );
	usb_get_string_simple( dev, usbdev->descriptor.iProduct, strproduct, bufsize );
	usb_get_string_simple( dev, usbdev->descriptor.iSerialNumber, strserial, bufsize );

}

static int get_file_firmware_version (char *filename)
{
	gchar *end = NULL;
	gchar *start = NULL;
	int ret = 0;

	end = g_strrstr (filename, ".");
	start = g_strrstr (filename, "_")+1;

	if (!start || !end){
		return -1;
	}

	ret = strtol (start, NULL, 10);

	return ret;
}

static int get_backup_count( void )
{
	int count = 0;
	glob_t gs;

	if( glob( "backups/backup_[0-9]*.euvc", 0, NULL, &gs ) == 0){
		if( gs.gl_pathc > 0 ){
			count = gs.gl_pathc;
		}
		globfree(&gs);
	}

	return count;
}


static void
usage(const char *argv0)
{
	printf("Usage: %s <options> [firmware-file]\n", argv0);
	printf("Supported options:\n");
	printf("-h\t\tShow this help screen.\n");
	printf("-v\t\tPrint firmware version.\n");
	printf("-m <uvc|proprietary>\tSet camera interface mode.\n");
	printf("-p\t\tPrint eeprom info.\n");
}

int
main(int argc, char *argv[])
{
	struct usb_device *udev;
	usb_dev_handle *dev;
	int ret = 1;
	int byte_index = 0;
	int byte_value = 0;
	struct caminfo camera_info;
	char *fileexp = NULL;
	char *filename = NULL;
	char backup_filename[256];
	gboolean recover_backup = FALSE;

	GOptionContext *context;
/*    g_type_init(); */

	context = g_option_context_new( "- euvccam tool" );
	g_option_context_add_main_entries( context, options_entries, NULL );
	g_option_context_parse( context, &argc, &argv, NULL );


	if( argv[optind] ){
		filename = argv[optind];
	}

	if (filename && !strncmp (filename, "backup", strlen("backup")))
		recover_backup = TRUE;

	/* Initialise libusb. */
	usb_init();
	usb_find_busses();
	usb_find_devices();


	udev = euvc_find_device(&camera_info);
	if (udev == NULL) {
		printf("Device not found.\n" );
		return 1;
	}

	dev = euvc_open(udev);
	if (dev == NULL) {
		printf("Unable to open device %hd:%hd.\n", camera_info.idVendor, camera_info.idProduct );
		return 1;
	}

	if( !fileexp )
		fileexp = camera_info.firmware_file;

	if( !filename ){
		glob_t gs;

		if( glob( fileexp, 0, NULL, &gs ) == 0){
			if( gs.gl_pathc > 0 ){
				filename = malloc( strlen( gs.gl_pathv[gs.gl_pathc-1] ) + 1 );
				strcpy( filename, gs.gl_pathv[gs.gl_pathc-1] );
			}
			globfree(&gs);
		}
	}

	char camconfig;

	/* if (euvc_read_camconfig (dev, &camconfig) < 0){ */
	/* 	perror ("camconfig"); */
	/* 	fprintf (stderr, "Failed to access camera (are you root?)\n"); */
	/* 	return 1; */
	/* } */

	sprintf( backup_filename, "backups/backup_%d.euvc", get_backup_count() );

/*    printf( "Device opened.\n" ); */
	if( options.upload || options.download ){
		printf( "firmware basename: '%s'\n", fileexp );
		printf( "->using file: '%s'\n", filename );
		if (!recover_backup)
			printf( "->backup file: '%s'\n", backup_filename );
	}

	/* // Set UVC compatibility flag */
	/* if (!recover_backup) */
	/*	   euvc_set_flags (dev, 1); */

	if( options.download ){
		euvc_download_eeprom( dev, FIRMWARE_END, filename );
	}

	if( options.upload ){
		char strvendor[128], strproduct[128], strserial[128];
		unsigned short pid;
		unsigned char file_id;
		int file_version = get_file_firmware_version (filename);

		if (euvc_get_pid (dev, &pid) < 0 ){
			perror ("Upload failed: Failed to read device ID");
			if (!options.force)
				return 1;
			else
				fprintf (stderr, "Continuing anyway due to 'force'\n");
		}

		if( !filename ){
			fprintf( stderr, "Upload failed: No matching firmware file found!\n" );
			return 1;
		}

		printf( "Upload firmware version: %d\n", get_file_firmware_version (filename) );
		file_id = get_firmware_file_id (filename);
		if (file_id == 0){
			fprintf (stderr, "Failed to read firmware file -- Aborting!");
			return -1;
		}

		if (file_id != (pid&0xff)){
			if (!options.force){
				fprintf (stderr, "Camera firmware ID [%x] does not match file ID [%x]! Aborting!\n", pid&0xff, file_id);
				return 1;
			} else {
				fprintf (stderr, "Camera firmware ID does not match file ID! Forcing anyway!\n");
			}
		}

		if( !recover_backup ){
			printf( "Creating backup file...\n" );

			if( !euvc_download_eeprom( dev, 16384, backup_filename ) ){
				fprintf( stderr, "Failed to create a backup of the eeprom image!\n" );
				return 1;
			}

			euvc_get_strings( dev, strvendor, strproduct, strserial, sizeof( strvendor ) );

			printf( "Current config: Vendor = '%s', Product = '%s', Serial = '%s'\n", strvendor, strproduct, strserial );
		}


		printf( "Uploading image file...\n" );

		if( euvc_upload_eeprom( dev, recover_backup ? EEPROM_SIZE : FIRMWARE_END,
					filename,
					(recover_backup||options.force) ? 0: pid & 0xff ) < 0 ){
			fprintf( stderr, "Failed to write image file '%s'\n", filename );
			printf( "Trying to upload backup image file\n" );
			if( !recover_backup ){
				if( euvc_upload_eeprom( dev, 16364, backup_filename, 0 ) < 0 ){
					fprintf( stderr, "Failed to upload backup image\n" );
					printf( "Disconnect and reconnect the device. Then try to upload the '%s' file.\n", backup_filename );
				}
				recover_backup = TRUE;
			}
		}

		if( !recover_backup ){
			printf( "Updating strings...\n" );

			euvc_write_strings( dev, strvendor, strproduct, strserial );

			printf ("!!!IMPORTANT!!!\n");
			printf ("Please save this application and the backup files for future reference!\n");
			printf ("A backup of the old firmware was created here: %s\n", backup_filename);
			printf ("To return to the original firmware, use the following command:\n");
			printf ("%s -u %s\n\n", argv[0], backup_filename);
		}
	}

	if (options.mode){
		unsigned short pid = 0;
		if (euvc_get_firmware_version (dev) < 129){
			fprintf (stderr, "The firmware version of the device is too old. Please update!\n");
			return 1;
		}
		switch (options.mode[0]){
		case 'u':
			printf ("Switching camera to UVC mode\n");
			if (euvc_set_mode (dev, CAMERA_INTERFACE_MODE_UVC) < 0)
				fprintf (stderr, "Failed\n");
			break;
		case 'p':
			printf ("Switching camera to proprietary mode\n");
			if (euvc_set_mode (dev, CAMERA_INTERFACE_MODE_PROPRIETARY) < 0)
				fprintf (stderr, "Failed\n");
			break;
		default:
			fprintf (stderr, "Invalid mode specification\n");
			break;
		}
	}

	if( options.print_eeprom ){
		euvc_print_eeprom_info( dev );
	}

	if (options.get_version){
		printf ("Firmware version: %d\n", euvc_get_firmware_version (dev));
		printf ("Capability: %x\n", euvc_get_capability(dev));
	}

/*    if( do_verify ) */
/*    { */
/*       printf( "Verifying\n" ); */
/*       if( euvc_verify_firmware( dev, filename ) != 0 ) */
/*       { */
/*	 printf( "Verification failed!\n" ); */
/*       } */
/*    } */


	euvc_close(dev);
	return ret;
}
