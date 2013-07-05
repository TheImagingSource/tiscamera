///
/// @file firmware.h
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "FirmwareUpgrade.h"
#include "Camera.h"

namespace tis
{

class FwdFirmwareWriter : public FirmwareUpdate::IFirmwareWriter
{

public:
    FwdFirmwareWriter (Camera& cam ) : device_itf_(cam)
    {}


    ~FwdFirmwareWriter ()
    {}


    virtual bool write (uint32_t addr, void* pData, size_t data_size, unsigned int /* timeout_in_ms = 2000 */)
    {
        return device_itf_.sendWriteMemory( addr, data_size, (byte*) pData );
    }


    virtual bool write (uint32_t addr, uint32_t val, unsigned int /* timeout_in_ms = 2000 */)
    {
        uint32_t tmp_val = ntohl(val);
        return device_itf_.sendWriteMemory(addr, sizeof( tmp_val ), (byte*) &tmp_val);
    }


    virtual bool read (uint32_t addr, uint32_t& val, unsigned int /*timeout_in_ms = 2000 */)
    {
        uint32_t tmp_val;
        auto hr = device_itf_.sendReadMemory(addr, 4, (byte*) &tmp_val);

        if (hr)
        {
            val = ntohl(tmp_val);
        }
        return hr;
    }


    virtual bool read (uint32_t addr, unsigned int data_size, void* pData, unsigned int& /* read_count */ , unsigned int /* timeout_in_ms = 2000 */)
    {
        auto hr = device_itf_.sendReadMemory (addr, data_size, (byte*) pData);

        if (hr)
        {
            
        }
        return hr;

        /* if( SUCCEEDED( hr ) ) */
        /* { */
        /*     read_count = bytes_read; */
        /* } */
    }

private:
    Camera& device_itf_;

}; /* class FwdFirmwareWriter */

} /* namespace tis */


