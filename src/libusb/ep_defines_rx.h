#ifndef __EP_DEFINES_RX__
#define __EP_DEFINES_RX__

#define VENDOR_ID 0x2951

#define PRODUCT_ID_R5V1  0x0401
#define PRODUCT_ID_R42V2 0x0801
#define PRODUCT_ID_R8V2  0x0802
#define PRODUCT_ID_R10V2 0x0803
#define PRODUCT_ID_C42V2 0x0804
#define PRODUCT_ID_R12V1 0x0C01
#define PRODUCT_ID_R12V2 0x0C02
#define PRODUCT_ID_C12V2 0x0C03


#define HOST_TO_DEVICE ((unsigned char)(0x40)) // VENDOR, DEVICE, INPUT
#define DEVICE_TO_HOST ((unsigned char)(0xC0)) // VENDOR, DEVICE, OUTPUT
// LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,

/* Endpoint definition for UVC application */
#define USB_EP_BULK_VIDEO     ((unsigned char)(0x83)) /* EP 1 IN */
#define USB_EP_CONTROL_STATUS ((unsigned char)(0x82)) /* EP 2 IN */
#define USB_EP_BULK_BENCHMARK ((unsigned char)(0x84))
#define USB_EP_INTR_FRAME     ((unsigned char)(0x85))


#define START_STREAM_VALUE_SOFTWARE_SINGLE     ((unsigned char)(0xFF))
#define START_STREAM_VALUE_SOFTWARE_CONTINUOUS ((unsigned char)(0x00))
#define START_STREAM_VALUE_HARDWARE_TRIGGER    ((unsigned char)(0x66))

#define BASIC_PC_TO_USB_STOP_STREAM 0x15

#define BASIC_PC_TO_USB_DO_SW_SINGLE_TRIGGER 0xBB
#define BASIC_PC_TO_USB_RESET                0x01
#define BASIC_PC_TO_USB_START_STREAM         0x21
#define BASIC_PC_TO_USB_DATA_FLASH           0x33
#define BASIC_PC_TO_USB_ERASE_SECTOR_FLASH   0x34
#define BASIC_PC_TO_USB_EXPOSURE             0x05
#define BASIC_PC_TO_USB_GAIN                 0xEA
#define BASIC_PC_TO_USB_FPS                  0x0A
#define BASIC_PC_TO_USB_FOCUS                0x0B
#define BASIC_PC_TO_USB_HDR                  0xED
#define BASIC_PC_TO_USB_SHUTTER              0xEE
#define BASIC_PC_TO_USB_TEST_PATTERN         0xEF
#define BASIC_PC_TO_USB_HARD_RESET           0xDB
#define BASIC_PC_TO_USB_CLEANROOM_MODE       0xDC
#define BASIC_PC_TO_USB_FLASH_STROBE         0x0C
#define BASIC_PC_TO_USB_REGISTER_SENSOR      0xB0
#define BASIC_PC_TO_USB_ROI_POS              0xC4 // wValue == x_start, wIndex == y_start
#define BASIC_PC_TO_USB_UPDATING_MODE        0xC5
#define BASIC_PC_TO_USB_SET_BIT_DEPTH        0xC6


#define BASIC_USB_TO_PC_GET_DMA_OV_COUNT 0x22
#define BASIC_USB_TO_PC_DATA_FLASH       0x33
#define BASIC_USB_TO_PC_GET_FLASH_WIP    0x34
#define BASIC_USB_TO_PC_RES_HORIZONTAL   0x72
#define BASIC_USB_TO_PC_RES_VERTICAL     0x73
#define BASIC_USB_TO_PC_VERSION_FIRMWARE 0xFE
#define BASIC_USB_TO_PC_VERSION_HARDWARE 0xFF
#define BASIC_USB_TO_PC_FOCUS            0x0B
#define BASIC_USB_TO_PC_PHY_LNK          0xF8
#define BASIC_USB_TO_PC_RES_FPS          0x74
#define BASIC_USB_TO_PC_DEBUG            0x01
#define BASIC_USB_TO_PC_GET_LAST_ERROR   0xDD
#define BASIC_USB_TO_PC_TEMP_SENSOR      0x90
#define BASIC_USB_TO_PC_TEMP_CYPRESS     0x91
#define BASIC_USB_TO_PC_FLASH_STROBE     0x0C
#define BASIC_USB_TO_PC_REGISTER_SENSOR  0xB0
#define BASIC_USB_TO_PC_MAX_FPS          0xB6
#define BASIC_USB_TO_PC_MIN_FPS          0xB7
#define BASIC_USB_TO_PC_GET_EXPOSURE     0x05
#define BASIC_USB_TO_PC_MAX_EXP          0xC1
#define BASIC_USB_TO_PC_MIN_EXP          0xC2
#define BASIC_USB_TO_PC_GET_BIT_DEPTH    0xC6
#define BASIC_USB_TO_PC_GET_MAX_RESO     0xC7
#define BASIC_USB_TO_PC_SENS_TEMP        0xC8
#define BASIC_USB_TO_PC_FLASH_DELAY_MAX  0xC9
#define BASIC_USB_TO_PC_FPS              0x0A
#define BASIC_USB_TO_PC_HDR              0xED
#define BASIC_USB_TO_PC_SHUTTER          0xEE
#define BASIC_USB_TO_PC_TEST_PATTERN     0xEF

namespace tcam::afu420
{

enum class AFU420Property
{
    ExposureTime,
    Gain,
    Iris,
    Focus,
    HDR,
    WB_Red,
    WB_Green,
    WB_Blue,
    StrobeDelay,
    StrobeDuration,
    StrobeDelaySecond,
    StrobeDurationSecond,
    StrobePolarity,
    StrobeMode,
    OffsetX,
    OffsetY,
    OffsetAuto,
    BinningHorizontal,
    BinningVertical,
    OISMode,
    OISPosX,
    OISPosY,
    SensorWidth,
    SensorHeight,
};

} // namespace tcam::afu420

#endif
