/*
 * ep_defines.h
 *
 *  Created on: 04.07.2014
 *      Author: vornholz
 */

#ifndef EP_DEFINES_R42_H_
#define EP_DEFINES_R42_H_


#define SETTING_SIZE 8

struct SSetting
{
    unsigned short pixelX;
    unsigned short pixelY;
    unsigned char fps;
    unsigned char dump0;
    unsigned short dump1;
};


typedef struct SSetting SSSetting;


#define FLASH_STROBE_ADJUSTMENT (200.0)


#define R42_FIRMWARE_VERSION (200)


/* PC out, camera in
#define STOP_STREAM							0x15
#define REQUEST_IN_FOCUS					0x0B	// controls focus
#define REQUEST_IN_EXPOSURE					0x05	// controls exposure
#define REQUEST_IN_EEPROM_WRITE				0x03
#define REQUEST_IN_REGISTER_SENSOR			0xB0
#define REQUEST_IN_REGISTER_CONTROLLER		0xB1
#define REQUEST_IN_REGISTER_FOCUS			0xB2
#define REQUEST_IN_REGISTER_OIS				0xB3
#define REQUEST_IN_LOAD_OIS_FIRMWARE		0xB4
#define REQUEST_IN_SET_COLOR_GAIN			0xB5
#define REQUEST_IN_FPS						0x0A
#define REQUEST_IN_CHANGE_GAIN				0xEA
#define REQUEST_IN_CHANGE_PIX_CORRECT		0xEB
#define REQUEST_IN_CHANGE_AF				0xEC
#define REQUEST_IN_CHANGE_HDR				0xED
#define REQUEST_IN_CHANGE_SHUTTER			0xEE
#define REQUEST_IN_CHANGE_TEST_PATTERN		0xEF
#define REQUEST_IN_DATA_FLASH				0x33
#define REQUEST_IN_ERASE_SECTOR				0x34
#define REQUEST_IN_START					0xBB
#define REQUEST_IN_UPDATE_FIRMWARE			0xC0
#define REQUEST_IN_BASE_CONFIG				0xF0	// load vid, pid and camera stuff like image size
#define REQUEST_IN_RES_FPS					0x74	// set one resolution and fps
#define REQUEST_IN_OIS_POS					0x75	// OIS position
#define REQUEST_IN_OIS_MODE					0x76	// OIS mode
*/


/* PC in, camera out
#define REQUEST_OUT_GET_MAX_C_INT_TIME		0xC1
#define REQUEST_OUT_GET_FOCUS				0x0B
#define REQUEST_OUT_REGISTER_SENSOR			0xB0
#define REQUEST_OUT_REGISTER_CONTROLLER		0xB1
#define REQUEST_OUT_REGISTER_FOCUS			0xB2
#define REQUEST_OUT_REGISTER_OIS			0xB3
#define REQUEST_OUT_GET_COLOR_GAIN			0xB5
#define REQUEST_OUT_GET_MAX_FPS				0xB6
#define REQUEST_OUT_LINE_LENGTH				0x70
#define REQUEST_OUT_PIX_CLK					0x71
#define REQUEST_OUT_RES_HORIZONTAL			0x72
#define REQUEST_OUT_RES_VERTICAL			0x73
#define REQUEST_OUT_RES_FPS					0x74	// read all resolution settings
#define REQUEST_OUT_EEPROM_READ_EP0			0x03
#define REQUEST_OUT_BASE_CONFIG				0xF0	// load vid, pid and camera stuff like image size
#define REQUEST_OUT_DATA_FLASH				0x33
#define REQUEST_OUT_VERSION					0xFF
*/


#define ADVANCED_PC_TO_USB_REGISTER_CONTROLLER 0xB1
#define ADVANCED_PC_TO_USB_REGISTER_FOCUS      0xB2
#define ADVANCED_PC_TO_USB_REGISTER_OIS        0xB3
#define ADVANCED_PC_TO_USB_COLOR_GAIN          0xB5
#define ADVANCED_PC_TO_USB_SET_LINE_DELAY      0xBA
#define ADVANCED_PC_TO_USB_RES_FPS             0x74 // read all resolution settings
#define ADVANCED_PC_TO_USB_BASE_CONFIG         0xF0 // load vid, pid and camera stuff like image size
#define ADVANCED_PC_TO_USB_DATA_FLASH          0x33
#define ADVANCED_PC_TO_USB_AF                  0xEC
#define ADVANCED_PC_TO_USB_PIX_CORRECT         0xEB
#define ADVANCED_PC_TO_USB_UPDATE_FIRMWARE     0xC0
#define ADVANCED_PC_TO_USB_WRITE               0x03
#define ADVANCED_PC_TO_USB_OIS_MODE            0x76
#define ADVANCED_PC_TO_USB_OIS_POS             0x75
#define ADVANCED_PC_TO_USB_SINGLE_SHOT_MODE    0x77
#define ADVANCED_PC_TO_USB_PIC_CLK             0x71

#define ADVANCED_USB_TO_PC_EEPROM_READ_EP0 0x03
#define ADVANCED_USB_TO_PC_LINE_LENGTH     0x70
#define ADVANCED_USB_TO_PC_PIX_CLK         0x71
#define ADVANCED_PC_TO_USB_BINNING_WEIGHT  0x78
#define ADVANCED_USB_TO_PC_BINNING_WEIGHT  0x78

#define ADVANCED_USB_TO_PC_REGISTER_CONTROLLER 0xB1
#define ADVANCED_USB_TO_PC_REGISTER_FOCUS      0xB2
#define ADVANCED_USB_TO_PC_REGISTER_OIS        0xB3
#define ADVANCED_USB_TO_PC_COLOR_GAIN          0xB5

#define ADVANCED_USB_TO_PC_FOCUS_MIN_MAX  0xB8 //4 bytes in, macro (2byte), infinity (2byte)
#define ADVANCED_USB_TO_PC_GET_LENSCALIB  0xB9 //869 bytes lenscalib
#define ADVANCED_USB_TO_PC_GET_LINE_DELAY 0xBA
#define ADVANCED_USB_TO_PC_GET_GYRO       0xBC

#define ADVANCED_USB_TO_PC_RES_HORIZONTAL 0x72
#define ADVANCED_USB_TO_PC_RES_VERTICAL   0x73

#define ADVANCED_USB_TO_PC_BASE_CONFIG          0xF0 // load vid, pid and camera stuff like image size
#define ADVANCED_USB_TO_PC_ORIG_MIN_LINE_LENGTH 0x23
#define ADVANCED_USB_TO_PC_HAS_OPTICS           0xF1


#endif /* EP_DEFINES_H_ */
