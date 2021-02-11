#ifndef __STRUCT_DEFINES_RX__
#define __STRUCT_DEFINES_RX__

#if 1
#include <cstdint>
#elif defined WIN32
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

#define STRING_LENGTH   (127)
#define DESC_LENGTH     (79)
#define FLASH_SIZE      (2 * 1024 * 1024)
#define FOOTER_POS      (0x7FF00)
#define DATA_ARRAY_SIZE (192)

#define FLASH_ADDR_FIRMWARE     0x000000 // size =    256 KiB
#define FLASH_ADDR_INFO_HEADER0 0x050000 // size =    816 B (maximal 60 KiB)
//#define FLASH_ADDR_INFO_HEADER1            0x05FC20		// size =      0 B (maximal 992 byte) (just an example, FLASH_ADDR_INFO_HEADER1 could also be somewhere else)
#define FLASH_ADDR_INFO               0x05F000 // size =   3104 B
#define FLASH_ADDR_COLOR              0x060000 // size = 119.75 kiB
#define FLASH_ADDR_OIS                0x07DF00 // size =      8 KiB (only R42)
#define FLASH_ADDR_DEVIGNETTING_IMAGE 0x080000 // size =    320 KiB
#define FLASH_ADDR_FPN                0x0D0000 // size =     56 KiB - 102 KiB (max 192)
#define FLASH_ADDR_FPGA               0x100000 // size =  453.3 KiB or larger (only R12)

// u64Features0
#define POS_FIRMWARE            (0)
#define POS_INFO_HEADER0        (1)
#define POS_COLOR_CALIB         (2)
#define POS_FPN_DATA            (3)
#define POS_FPGA_FIRMWARE       (4)
#define POS_DEVIGNETTING_MATRIX (5)
#define POS_OIS                 (6)
// u64Features1
#define POS_WHATEVER (64)

#define BITMASK_FEATURE0(a) (1ULL << (a - 0))
#define BITMASK_FEATURE1(a) (1ULL << (a - 64))
#define BITMASK_FEATURE2(a) (1ULL << (a - 128))

typedef enum EMountType
{
    None = 0, // no mount (i.e. module C42)
    CMount = 1, // C mount
    FMount = 2, // F mount

} EMountType;


typedef struct SFeatureHeader
{
    uint64_t u64HeaderVersion;
    uint64_t u64Features0;
    uint64_t u64Features1;
    uint64_t u64Features2;
} SFeatureHeader;

typedef struct SSlot
{
    uint32_t uSize;
    uint32_t uAddress;
    uint64_t u64Version;
} SSlot;


// Fixed = hardware
// Locked = software
typedef struct SInfoHeader0 // 824 bytes
{
    uint16_t pshVendorName[STRING_LENGTH + 1]; // 64 bit aligned
    uint16_t pshProductString[STRING_LENGTH + 1]; // 64 bit aligned
    uint16_t pshSerial[STRING_LENGTH + 1]; // 64 bit aligned
    uint8_t ucLengthVendor; // 64 bit aligned
    uint8_t ucLengthProduct; //
    uint8_t ucLengthSerial; //
    uint8_t ucColor; //
    uint8_t ucMount; //
    uint8_t ucLockedResolution; //
    uint8_t ucLockedFirmware; //
    uint8_t ucLockedUsb2; //
    uint32_t uRoiStartX; // 64 bit aligned
    uint32_t uRoiStartY; //
    uint32_t uRoiSizeX; // 64 bit aligned
    uint32_t uRoiSizeY; //
    uint32_t uBinningXHW; // 64 bit aligned
    uint32_t uBinningYHW; //
    uint32_t uBinningXSW; // 64 bit aligned
    uint32_t uBinningYSW; //
    uint16_t ushPID; // 64 bit aligned
    uint16_t ushPID_USB2; //
    uint16_t ushVID; //
    uint8_t ucFixedFocus; //
    uint8_t ucFixedOis; //
    uint8_t ucLockedFocus; // 64 bit aligned
    uint8_t ucLockedOis; //
    uint8_t ucMla; // has mla yes/no
    uint8_t ucBayerToMono; // bayer to mono Flag
    uint8_t ucDummy0[4]; //
} SInfoHeader0;

typedef struct SVersion
{
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
} SVersion;

#endif
