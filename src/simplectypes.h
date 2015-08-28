/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef TCAM_SIMPLECTYPES_H
#define TCAM_SIMPLECTYPES_H


// #include "udshl_defs.h"

// #include "compilation_environment.h"

typedef long long REFERENCE_TIME;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                          \
    ((DWORD)(BYTE)(ch0) |                                       \
     ((DWORD)(BYTE)(ch1) << 8) |                                \
     ((DWORD)(BYTE)(ch2) << 16) |                               \
     ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)


// #ifndef __midl
namespace tcam
{
// #endif


/** the possible error types */
typedef enum
{
    eNOERROR = 0,               ///< no error occurred
    eUNKNOWN,                   ///< an unknown error occurred

    eDSHOWLIB_EXCEPTION,        ///< an unexpected DShowLibException occurred, please contact the producer's support
    eUNEXPECTED_DSHOWLIB_BEHAVIOUR, ///< DShowLib behaves unexpected, please contact the producer's support

    eOUT_OF_MEMORY,             ///< out of memory
    eWRITE_ERROR,               ///< an error occurred while writing
    eNO_VIDEO_HARDWARE_FOUND,   ///< no video hardware found on this system

    eINVALID_PARAM_VAL,         ///< an invalid parameter was passed in
    eAUTOMATION_ENABLED,        ///< automation is enabled for given property, could not set/get value
    eNO_CURRENT_VALUE,          ///< no current value for setting is available
    eINVALID_MEMBUFFER,         ///< the MemBuffer is not compatible to the grabber format or the MemBuffer is not valid
    eMODE_ALREADY_ACTIVE,       ///< the requested mode is already the active one, we cannot switch
    eNO_DEVICE_OPENED,          ///< no device opened, please open one to operate
    eDEVICE_INVALID,            ///< the device has become invalid (e. g. it was unplugged)
    eDRIVER_INSTALLATION,       ///< driver component not found; please check your driver installation
    eNOT_AVALILABLE_WITH_CURRENT_DEVICE, ///< the called function is not available with currently set device
    eAUTOMATION_NOT_AVAILABLE,  ///< automation is not available for the given property
    eDEVICE_NOT_FOUND,          ///< the device was not found
    eITEM_DOES_NOT_FIT_TO_DEV,  ///< the given item does not fit to the current device
    eFUNC_NOT_AVAIL_IN_LIVEMODE, ///< this function cannot be called if live mode is active
    eFUNC_ONLY_AVAIL_IN_LIVEMODE, ///< this function cannot be called if live mode is not active
    eNOT_INITIALIZED,           ///< the object is not initialized
    eNO_FRAMEGRABBER_SINK,      ///< a function was called, which relies on having a FrameGrabberSink as sink_type
    eSERIALNUMBER_INVALID,      ///< the serial number is invalid
    eVIDEOFORMAT_INVALID,       ///< the current VideoFormat is invalid
    eUNEXPECTED_SINKFORMAT_CHANGE, ///< during building the filter graph, the sink format changed unexpectedly and the MemBufferCollection got invalid to this new format
    eNO_EXTERNALTRANSPORT_AVAILABLE, ///< getETMode() and setETMode() don't function without an ExternalTransport capable device
    eTIMEOUT_PREMATURLY_ELAPSED, ///< the timeout passed to snapImages( ... ) did elapse before all images could be snaped

    ePASSED_DATA_DOES_NOT_FIT_TO_COMPRESSOR, ///< the data passed to the AviSink does not fit to the Compressor

    eOPTION_NOT_AVAILABLE,      ///< an option is not available, e.g. you called setFlipH and the VideoCaptureDevice does not support flipping
    eCOMPONENT_NOT_FOUND,       ///< a component of the installation was not found, please check your installation

    eNO_CODECS_FOUND,           ///< no codec installed on this system

    eINCOMPATIBLE_VERSION,      ///< the version of the data format passed to the function is incompatible with the function
    eREAD_ERROR,                ///< an error occurred while trying to read data from a file
    eINCOMPLETE,                ///< The operation was only partially successful, e.g. not all properties of the grabber could be restored

    eFILENOTFOUND,

} tErrorEnum;


/** the available sink types */
typedef enum
{
    eFRAME_GRABBER_SINK = 0x1, ///< the sink is of type FrameGrabberSink
    eMEDIASTREAM_SINK   = 0x2, ///< the sink is a type which creates avi/mpeg/... files
    eAVI_SINK           = 0x6, ///< the sink is of type AviSink
    eFRAMEHANDLER_SINK  = 0x8,
} tSinkType;


/** possible OutputColorformats
 */
typedef enum tColorformatEnum
{
    eInvalidColorformat = 0, ///< invalid color format (do not use !!)
    eRGB32 = 1,              ///< 32 bit BGRA
    eRGB24,                  ///< 24 bit BGR
    eRGB565,                 ///< 5-6-5 BGR, 16 bit
    eRGB555,                 ///< 5-5-5 BGR, 16 bit
    eRGB8,                   ///< 8 bit grey
    eY8 = eRGB8,             ///< because of old versions, eY8 and eRGB8 are equal
    eUYVY,                   ///< 16 bit YUV format layout U0Y0V0Y1, top down
    eY800,                   ///< 8 bit Y format, top down (this means no transformation between input Y800 and
                             ///<   the sink is needed)
    eYGB1,                   ///< 16 bit Y (10 bit valid) grey, top down, bits ordered per pixel [76543210______98],
    /** Algorithm for converting this layout to eY800 :
        BYTE convYGB1toY8( __int16 y )
        {
        BYTE* p = (BYTE*) &y;
        unsigned __int16 x = ((unsigned int)p[0] << 8) | p[1];
        return (BYTE) ((x >> 2) & 0xFF );
        }
    */
    eYGB0,                    ///< 16 bit Y (10 bit valid) grey, top down, bits ordered per pixel [10______98765432],
    /** Algorithm for converting this layout to eY800 :
        BYTE convYGB0toY8( __int16 y )
        {
        return (BYTE) (y & 0xFF);
        }
    */
    eBY8,                     ///< Bayer Y800 Format
    eY16,                     ///< 16-bit gray, top down. Each pixel is represented
                              ///<   by an unsigned 16 bit integer (unsigned short, uint16_t)
    /** Algorithm for converting this layout to eY800 :
        BYTE convY16toY8( uint16_t y )
        {
        return (BYTE) (y >> 8);
        }
    */
} tColorformatEnum;


/// Mask for RGB565-Format
typedef enum
{
    eRGB565_R = 0xf800, ///< Mask for blue (1111100000000000)
    eRGB565_G = 0x07e0, ///< Mask for green (0000011111100000)
    eRGB565_B = 0x001f, ///< Mask for red (0000000000011111)
} RGB565Mask;


/// Mask for RGB555-Format
typedef enum
{
    eRGB555_R = 0x7c00, ///< Mask for blue (0111110000000000)
    eRGB555_G = 0x03e0, ///< Mask for green (0000001111100000)
    eRGB555_B = 0x001f, ///< Mask for red (0000000000011111)
} RGB555Mask;


/** Graph positions
 * the graph most likely will look like this :
 *
 * VCD -> x1 -> tee filter -> x2 -> sink
 * -> x3 -> renderer
 *
 * The positions marked with x(n) can be configured by the following enum
 */
typedef enum
{
    ePP_NONE    = 0x0,
    ePP_DEVICE  = 0x1, ///< x1, directly behind the source
    ePP_SINK    = 0x2, ///< x2, directly in front of a frame grabber sink/codec
    ePP_DISPLAY = 0x4, ///< x3, directly in front of the video renderer
} tPathPosition;


// #ifndef __midl

/// Enumeration of frame type. Member of tsMediaSampleDesc.
typedef enum
{
    eFRAME_INTERLEAVED = 0, ///< interleaved frame
    eFRAME_FIELD1      = 1, ///< the first field of a frame
    eFRAME_FIELD2      = 2, ///< the second field of a frame
} tFrameDesc;


/// Structure describing the properties of a frame (e.g. a MemBuffer)
typedef struct tsMediaSampleDesc
{

    REFERENCE_TIME SampleStart; ///< the start time of the sample as set by the video capture device
    REFERENCE_TIME SampleEnd;   ///< the end time of the sample as set by the video capture device
    unsigned long  FrameNumber; ///< the frame number of the sample as set by the video capture device
    ///< the device does not need to set this field, or keep it updated
    tFrameDesc FrameType; ///< the type of the frame

} tsMediaSampleDesc;


/** Public subtypes which you can use in applications.
 * Other common subtypes (which are included by the directshow headers) are :
 MEDIASUBTYPE_RGB32
 MEDIASUBTYPE_RGB24
 MEDIASUBTYPE_RGB565
 MEDIASUBTYPE_RGB555
 MEDIASUBTYPE_RGB8
 MEDIASUBTYPE_UYVY
 MEDIASUBTYPE_YUY2
 ...

 * When you want to create your own subtypes, then you need to only alter the first DWORD, which is the
 * String Representation of your FourCC.
 * You may also create your completely own GUIDs which do not correspond to FourCCs, but then no string
 * representation can be given by the library.
 */
// static const GUID MEDIASUBTYPE_Y800 = { 0x30303859,                     0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
// static const GUID MEDIASUBTYPE_BY8  = { MAKEFOURCC('B', 'Y', '8', ' '), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
// static const GUID MEDIASUBTYPE_YGB0 = { MAKEFOURCC('Y', 'G', 'B', '0'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
// static const GUID MEDIASUBTYPE_YGB1 = { MAKEFOURCC('Y', 'G', 'B', '1'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
// static const GUID MEDIASUBTYPE_YUY2 = { MAKEFOURCC('Y', 'U', 'Y', '2'), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
// static const GUID MEDIASUBTYPE_Y16  = { MAKEFOURCC('Y', '1', '6', ' '), 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

/// Pattern to user for a buffer fill (to use when debugging)
typedef enum
{
    eTERMINATION, ///< fills with 0 and all bits set for all values of the last pixel
    eBWH,         ///< fills with horizontal lines, black/white
    eBWV,         ///< fills with vertical lines, black/white
    eBWQ,         ///< fills with alternate pixels (mini quads) black/white
} tPatternEnum;
// #endif

// #ifndef __midl
};
// #endif

#endif /* TCAM_SIMPLECTYPES_H */
