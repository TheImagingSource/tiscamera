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

#ifndef _GIGEVISION_H_
#define _GIGEVISION_H_

#ifdef __linux__
#include <sys/types.h>
#include <arpa/inet.h>
#endif


#define STANDARD_GVCP_PORT 3956

namespace Commands
{

    static const unsigned int DISCOVERY_CMD    = 0x02;
    static const unsigned int DISCOVERY_ACK    = 0x03;
    static const unsigned int FORCEIP_CMD      = 0x04;
    static const unsigned int FORCEIP_ACK      = 0x05;
    static const unsigned int READREG_CMD      = 0x80;
    static const unsigned int READREG_ACK      = 0x81;
    static const unsigned int WRITEREG_CMD     = 0x82;
    static const unsigned int WRITEREG_ACK     = 0x83;
    static const unsigned int READMEM_CMD      = 0x84;
    static const unsigned int READMEM_ACK      = 0x85;
    static const unsigned int WRITEMEM_CMD     = 0x86;
    static const unsigned int WRITEMEM_ACK     = 0x87;
    static const unsigned int PACKETRESEND_CMD = 0x40;
    static const unsigned int PACKETRESEND_ACK = 0x41;
    static const unsigned int EVENT_CMD        = 0xC0;
    static const unsigned int EVENT_ACK        = 0xC1;
    static const unsigned int EVENTDATA_CMD    = 0xC2;
    static const unsigned int EVENTDATA_ACK    = 0xC3;
    static const unsigned int INVALID_COMMAND  = 0xFF;

} /* namespace Commands */

namespace Flags
{

    static const unsigned int NEEDACK    = 0x1;
    static const unsigned int RETRYCMD   = 0x2;

} /* namespace Flags */

namespace Status
{

    static const unsigned int SUCCESS              = 0x0000;  //  Command executed successfully
    static const unsigned int INVALID_PARAMETER    = 0x8002;  // At least one parameter provided in the command
                                                              // is invalid (or out of range) for the device
    static const unsigned int INVALID_ADDRESS      = 0x8003;  // An attempt was made to access a non existent address space location.
    static const unsigned int WRITE_PROTECT        = 0x8004;  // The addressed register cannot be written to
    static const unsigned int BAD_ALIGNMENT        = 0x8005;  // A badly aligned address offset or data size was specified.
    static const unsigned int ACCESS_DENIED        = 0x8006;  // An attempt was made to access an address location which
                                                              // is currently/momentary not accessible. This depends on the
                                                              // current state of the device, in particular the current
                                                              // privilege of the application.
    static const unsigned int BUSY                 = 0x8007;  // A required resource to service the request isn't currently
                                                              // available. The request may be retried at a later time.
    static const unsigned int NO_MSG               = 0x800B;  // Timeout, no message received
    static const unsigned int PACKET_UNAVAILABLE   = 0x800C;  // The request packet is not available anymore.
    static const unsigned int INVALID_HEADER       = 0x800E;  // The message header is not valid. Some of its fields do not

    static const unsigned int TIMEOUT              = 0x8FFE;  // The packet response timed out
    static const unsigned int FAILURE              = 0x8FFF;  // Generic/unknown failure
} /* namespace Status */

namespace Register
{
    static const unsigned int VERSION_REGISTER                         = 0x0000;
    static const unsigned int DEVICEMODE_REGISTER                      = 0x0004;
    static const unsigned int DEVICEMODE_BIGENDIAN                     = 0x80000000;
    static const unsigned int DEVICEMODE_LITTLEENDIAN                  = 0x00000000;
    static const unsigned int DEVICEMODE_CHARSET_UTF8                  = 0x00000001;
    static const unsigned int MAC_HI_REGISTER                          = 0x0008;
    static const unsigned int MAC_LO_REGISTER                          = 0x000C;
    static const unsigned int SUPPORTED_IPCFG_REGISTER                 = 0x0010;
    static const unsigned int IPCFG_LLA                                = 0x00000004;
    static const unsigned int IPCFG_DHCP                               = 0x00000002;
    static const unsigned int IPCFG_PERSISTANT                         = 0x00000001;
    static const unsigned int CURRENT_IPCFG_REGISTER                   = 0x0014;
    static const unsigned int CURRENT_IPADDRESS_REGISTER               = 0x0024;
    static const unsigned int CURRENT_SUBNETMASK_REGISTER              = 0x0034;
    static const unsigned int CURRENT_DEFAULTGATEWAY_REGISTER          = 0x0044;
    static const unsigned int MANUFACTURER_NAME_REGISTER               = 0x0048;
    static const unsigned int MODEL_NAME_REGISTER                      = 0x0068;
    static const unsigned int DEVICE_VERSION_REGISTER                  = 0x0088;
    static const unsigned int MANUFACTURER_INFO_REGISTER               = 0x00A8;
    static const unsigned int SERIAL_NUMBER_REGISTER                   = 0x00D8;
    static const unsigned int USER_DEFINED_NAME_REGISTER               = 0x00E8;
    static const unsigned int NUM_NETWORK_INTERFACES_REGISTER          = 0x0600;
    static const unsigned int PERSISTANT_IPADDRESS_REGISTER            = 0x064C;
    static const unsigned int PERSISTANT_SUBNETMASK_REGISTER           = 0x065C;
    static const unsigned int PERSISTANT_DEFAULTGATEWAY_REGISTER       = 0x066C;
    static const unsigned int NUM_MESSAGE_CHANNELS_REGISTER            = 0x0900;
    static const unsigned int NUM_STREAM_CHANNELS_REGISTER             = 0x0904;
    static const unsigned int GVCP_SUPPORTED_COMMANDS_REGISTER         = 0x0934;
    static const unsigned int GVCP_SUPPORTS_WRITEMEM                   = 0x00000002;
    static const unsigned int GVCP_SUPPORTS_CONCATENATION              = 0x00000001;
    static const unsigned int HEARTBEAT_TIMEOUT_REGISTER               = 0x0938;
    static const unsigned int CONTROLCHANNEL_PRIVELEGE_REGISTER        = 0x0A00;
    static const unsigned int CONTROLCHAN_PRIV_MONITOR                 = 0x00000004;
    static const unsigned int CONTROLCHAN_PRIV_CONTROL                 = 0x00000002;
    static const unsigned int CONTROLCHAN_PRIV_EXCLUSIVE               = 0x00000001;

} /* namespace Register */


namespace Packet
{

    typedef struct COMMAND_HEADER
    {
        unsigned char    magic;
        unsigned char    flag;
        unsigned short   command;

        unsigned short   length;
        unsigned short   req_id;

    } COMMAND_HEADER;

    typedef struct ACK_HEADER
    {
        unsigned short    status;
        unsigned short    answer;

        unsigned short    length;
        unsigned short    ack_id;

    } ACK_HEADER;


    // Command Structures
    typedef struct CMD_DISCOVERY
    {
        COMMAND_HEADER    header;

    } CMD_DISCOVERY;

    typedef struct CMD_WRITEREG_OP
    {
        uint32_t    address;
        uint32_t    value;

    } CMD_WRITEREG_OP;

    typedef struct CMD_WRITEREG
    {
        COMMAND_HEADER  header;

        CMD_WRITEREG_OP ops[1];

    } CMD_WRITEREG;

    typedef struct
    {
        COMMAND_HEADER  header;

        uint32_t            address[1];

    } CMD_READREG;

    typedef struct
    {
        COMMAND_HEADER  header;

        uint32_t            address;
        uint16_t            reserved;
        uint16_t            count;
    } CMD_READMEM;

    typedef struct
    {
        COMMAND_HEADER  header;

        uint32_t            address;
        uint32_t            data[1];

    } CMD_WRITEMEM;

    typedef struct
    {
        COMMAND_HEADER  header;

        uint16_t            stream_channel_index;
        uint16_t            block_id;
        uint16_t            reserved0;
        uint16_t            first_packet_id;
        uint16_t            reserved1;
        uint16_t            last_packet_id;
    } CMD_PACKETRESEND;

    typedef struct
    {
        COMMAND_HEADER    header;

        uint16_t reserved0;
        uint16_t DeviceMACHigh;
        uint32_t DeviceMACLow;

        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
        uint32_t StaticIP;

        uint32_t reserved4;
        uint32_t reserved5;
        uint32_t reserved6;
        uint32_t StaticSubnetMask;

        uint32_t reserved7;
        uint32_t reserved8;
        uint32_t reserved9;
        uint32_t StaticGateway;
    } CMD_FORCEIP;

    // acknowledge structures

    typedef struct ACK_DISCOVERY
    {
        ACK_HEADER header;

        uint16_t spec_version_major;
        uint16_t spec_version_minor;

        uint32_t device_mode;
        uint16_t reserved0;
        uint16_t DeviceMACHigh;
        uint32_t DeviceMACLow;
        uint32_t IPConfigOptions;
        uint32_t IPConfigCurrent;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
        uint32_t CurrentIP;
        uint32_t reserved4;
        uint32_t reserved5;
        uint32_t reserved6;
        uint32_t CurrentSubnetMask;
        uint32_t reserved7;
        uint32_t reserved8;
        uint32_t reserved9;
        uint32_t DefaultGateway;

        char ManufacturerName[32];
        char ModelName[32];
        char DeviceVersion[32];
        char ManufacturerSpecificInformation[48];
        char Serialnumber[16];
        char UserDefinedName[16];
    } ACK_DISCOVERY;


    typedef struct ACK_WRITEREG
    {
        ACK_HEADER  header;

        uint16_t    reserved;
        uint16_t    index;

    } ACK_WRITEREG;


    typedef struct
    {
        ACK_HEADER  header;

        uint32_t    data[1];

    } ACK_READREG;


    typedef struct
    {
        ACK_HEADER  header;

        uint32_t    address;
        uint32_t    data[0];

    } ACK_READMEM;


    typedef struct
    {
        ACK_HEADER  header;

        uint16_t    reserved;
        uint16_t    index;

    } ACK_WRITEMEM;

} /* namespace Packet */

#endif /* _GIGEVISION_H_ */
