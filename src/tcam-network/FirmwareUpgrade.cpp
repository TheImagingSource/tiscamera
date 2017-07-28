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

#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>

#include <zip.h>
#include <tinyxml.h>

#include "FirmwareUpgrade.h"
#include "GigE3Update.h"


namespace
{

static bool parseManufacturerSpecificInformation (const std::string& info, int& typeId, std::string& modelName)
{
    // a string typically would look like:  @Type=1@Model=ICX618 C T@

    // determine type
    size_t type_pos = info.find("Type=");
    if (type_pos == std::string::npos)
    {
        return false;
    }
    else
    {
        size_t end = info.find("@", type_pos);
        size_t begin = type_pos + sizeof("Type=") - 1;

        std::string s = info.substr(begin, end - begin);
        typeId = atoi(s.c_str());
    }

    // determine model
    size_t model_pos = info.find("Model=");
    if (model_pos == std::string::npos)
    {
        return false;
    }
    else
    {
        size_t end = info.find("@", model_pos);
        size_t begin = model_pos + sizeof("Model=") - 1;

        std::string s = info.substr(begin, end - begin);
        modelName=s;
    }

    return true;
}


bool getDeviceInfo (const Packet::ACK_DISCOVERY& discoveryResponse, int& typeId, std::string& modelName)
{
    std::string manufacturerSpecificInformation = discoveryResponse.ManufacturerSpecificInformation;

    // "old" (blackfin-based) camera does not contain manufacturer-specific information
    if (manufacturerSpecificInformation.length() == 0)
    {
        typeId = 0;
        modelName = "";
        return true;
    }
    return parseManufacturerSpecificInformation(manufacturerSpecificInformation, typeId, modelName);
}


bool queryDeviceFPGAVersion (FirmwareUpdate::IFirmwareWriter& dev, uint32_t& version)
{
    return dev.read( 0xF0000024, version );
}


std::vector<unsigned char> loadFile (const std::string& fileName)
{
    std::vector<unsigned char> contents;

    FILE* pF = fopen(fileName.c_str(), "rb");
    if (pF == NULL)
    {
        return contents;
    }

    fseek(pF, 0, SEEK_END);
    long fileLen = ftell(pF);
    if(fileLen)
    {
        contents.resize(fileLen);

        fseek(pF, 0, SEEK_SET);
        fread(&contents[0], 1, fileLen, pF);
        fclose(pF);

        while (contents.size() % 4)
        {
            contents.push_back(0);
        }
    }
    return contents;
}


bool isPackageFile (const std::string& fileName)
{
    std::string ending = ".fwpack";
    if (fileName.length() >= ending.length())
    {
        return (0 == fileName.compare (fileName.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}


std::vector<byte> extractFileFromPackage (const std::string& packageFileName, const std::string& fileName)
{
    std::vector<unsigned char> result;

    //Open the ZIP archive
    int err = 0;
    zip *z = zip_open(packageFileName.c_str(), 0, &err);

    //Search for the file of given name
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(z, fileName.c_str(), 0, &st);

    //Alloc memory for its uncompressed contents
    char contents [st.size];

    //Read the compressed file
    zip_file *f = zip_fopen(z, fileName.c_str(), 0);
    zip_fread(f, contents, st.size);

    std::copy ( contents, contents + st.size, std::back_inserter(result));
    // zip_fclose(f);

    //And close the archive
    zip_close(z);

    return result;
}


bool rebootCamera (FirmwareUpdate::IFirmwareWriter& dev)
{
    return dev.write( 0xEF000004, 0xB007B007 );
}

} /* namespace */


namespace FirmwareUpdate
{

Status uploadBlackfinFirmware (IFirmwareWriter& dev, std::vector<unsigned char>& data)
{
    if( !dev.write( 0xEF000000, 0xA35FB241 ) ) // unlock
    {
        return Status::WriteError;
    }

    dev.write( 0xEF000004, (1<<2), 3000 /* longer timeout, eeprom may take a while */ ); // erase app

    sleep( 1000 );

    if( !dev.write( 0xEE020000, &data[0], (uint32_t)data.size(), 3000 ) )
    {
        return Status::WriteVerificationError;
    }

    dev.write( 0xEF000000, 0x0 ); // lock

    return Status::SuccessDisconnectRequired;
}


Status upgradeBlackfinFirmware (IFirmwareWriter& dev, const std::string& fileName, std::function<void(int, const std::string&)> /* progressFunc */)
{
    if( isPackageFile(fileName) )
    {
        return Status::DeviceSupportsFwOnly;
    }

    std::vector<unsigned char> data = loadFile( fileName );
    if( data.size() != 0x10000 )
    {
        return Status::InvalidFile;
    }

    return uploadBlackfinFirmware( dev, data );
}


Status findFirmwareInPackage (const std::string& fileName, const std::string& modelName, std::string& firmwareName, std::string& FPGAConfigurationName, unsigned int& requiredFPGAVersion)
{
    // assure we find the index.xml even if it is differently capitalized
    std::vector<unsigned char> xmlData = extractFileFromPackage( fileName, "index.xml" );
    if (xmlData.empty())
    {
        xmlData = extractFileFromPackage( fileName, "Index.xml" );
    }
    xmlData.push_back( 0 );

    TiXmlDocument xdoc;
    xdoc.Parse( (char*)&xmlData[0] );
    if( xdoc.Error() )
    {
        return Status::InvalidFile;
    }

    TiXmlHandle xhandle( &xdoc );
    auto fwnode = xhandle.FirstChild( "firmwares" ).FirstChild( "firmware" ).ToElement();

    for( ; fwnode; fwnode = fwnode->NextSiblingElement() )
    {
        auto nameAttr = fwnode->Attribute( "name" );
        if( nameAttr == modelName )
        {
            const char* fwFileAttr = fwnode->Attribute( "firmwarefile" );
            const char* fpgaConfigurationAttr = fwnode->Attribute( "fpgafile" );
            int res = fwnode->QueryIntAttribute( "requiredfpga", (int*)&requiredFPGAVersion );

            if (fwFileAttr == 0) return Status::InvalidFile;
            if (fpgaConfigurationAttr == 0) return Status::InvalidFile;
            if (res != TIXML_SUCCESS) return Status::InvalidFile;

            firmwareName = fwFileAttr;
            FPGAConfigurationName = fpgaConfigurationAttr;

            return Status::Success;
        }
    }

    return Status::NoMatchFoundInPackage;
}


Status uploadAndVerify (IFirmwareWriter& dev, unsigned int address, unsigned char* block, unsigned int blockSize)
{
    int retry = 5;
    bool success = true;
    do
    {
        success = dev.write( address, block, blockSize, 3000 );
        if( success )
        {
            std::vector<byte> verificationBuf( blockSize );
            unsigned int bytesRead;

            success = dev.read( address, blockSize, &verificationBuf[0], bytesRead, 3000 );
            if( success )
            {
                if( memcmp( block, &verificationBuf[0], blockSize ) != 0 )
                {
                    if( retry-- > 0 )
                    {
                        continue;
                    }
                    else
                    {
                        // failed
                        success = false;
                    }
                }
            }
        }
    }
    while( false );

    return success ? Status::Success : Status::WriteVerificationError;
}


Status uploadGigEFPGAFirmware (IFirmwareWriter& dev, std::vector<unsigned char>& data, std::function<void(int,const std::string&)> progressFunc)
{
    progressFunc( 0, "Uploading" );

    if( !dev.write( 0xEF000000, 0xA35FB241 ) ) // unlock
    {
        return Status::WriteError;
    }

    unsigned int base = 0xEE000000;
    Status status = Status::Success;
    for( unsigned int offset = 0; offset < data.size() && succeeded(status); offset += 128 )
    {
        unsigned int blockSize = std::min( 128u, (unsigned int)data.size() - offset );

        status = uploadAndVerify( dev, base + offset, &data[0] + offset, blockSize );

        progressFunc( (offset * 100) / data.size(), "" );
    }

    dev.write( 0xEF000000, 0x0 ); // lock

    if( succeeded(status) )
    {
        progressFunc( 100, "Finished" );
    }

    return status;
}


Status uploadFPGAConfiguration (IFirmwareWriter& dev, std::vector<byte>& data, std::function<void(int, const std::string&)> progressFunc)
{
    progressFunc( 0, "Start Erase" );

    if( !dev.write( 0xC1000000, 0xA35FB241 ) ) // unlock
    {
        return Status::WriteError;
    }

    unsigned int base = 0xC0000000;

    // erase
    for( unsigned int offset = 0; offset < 0x80000; offset += 0x10000 )
    {
        if( !dev.write( 0xC1000004, base + offset, 5000 ) ) // erase
        {
            return Status::WriteVerificationError;
        }
        progressFunc( 100 * offset / 0x80000, "Erasing" );
    }

    progressFunc( 100, "" );

    progressFunc( 0, "Uploading" );

    Status status = Status::Success;
    for (unsigned int offset = 0; offset < data.size() && succeeded(status); offset += 256)
    {
        unsigned int blockSize = std::min(256u, (unsigned int)data.size() - offset);

        status = uploadAndVerify(dev, base + offset, &data[0] + offset, blockSize);

        progressFunc((offset * 100) / data.size(), "");
    }

    dev.write(0xC1000000, 0x0); // lock

    if (succeeded(status))
    {
        progressFunc(100, "Finished");
    }

    return status;
}


Status upgradeFPGAFirmwareFromPackage (IFirmwareWriter& dev, const std::string& fileName, const std::string& modelName, std::function<void(int, const std::string&)> progressFunc)
{
    std::string firmwareName;
    std::string fpgaConfigurationName;
    unsigned int requiredFPGAVersion;
    Status status = findFirmwareInPackage(fileName, modelName, firmwareName, fpgaConfigurationName, requiredFPGAVersion);
    if (failed(status))
    {
        return status;
    }

    unsigned int deviceFPGAVersion = 0;
    bool fpgaVersionOK = queryDeviceFPGAVersion( dev, deviceFPGAVersion ) &&
        (deviceFPGAVersion == requiredFPGAVersion);

    bool fpgaUpgradeRequired = !fpgaVersionOK;

    // Just for debugging
    //fpgaUpgradeRequired = true;

    std::vector<byte> fpgaData = extractFileFromPackage(fileName, fpgaConfigurationName);
    std::vector<byte> fwData = extractFileFromPackage(fileName, firmwareName);
    if (fpgaUpgradeRequired && fpgaData.size() == 0)
    {
        return Status::InvalidFile;
    }
    if (fwData.size() != 0xB000)
    {
        return Status::InvalidFile;
    }

    if (fpgaUpgradeRequired)
    {
        status = uploadFPGAConfiguration( dev, fpgaData, progressFunc );
        if (failed(status))
        {
            return status;
        }
    }

    status = uploadGigEFPGAFirmware(dev, fwData, progressFunc);

    if (succeeded(status) && fpgaUpgradeRequired)
    {
        // We succeeded, but FPGA configuration was updated: hw disconnect is required!
        return Status::SuccessDisconnectRequired;
    }
    else
    {
        if (rebootCamera(dev))
        {
            return status;
        }
        else
        {
            // Automatic reboot did not work, hw disconnect required
            return Status::SuccessDisconnectRequired;
        }
    }
}


Status upgradeFPGAFirmwareDirect (IFirmwareWriter& dev, const std::string& fileName, std::function<void(int, const std::string&)> progressFunc)
{
    std::vector<byte> fwData = loadFile(fileName);
    if (fwData.size() != 0xB000)
    {
        return Status::InvalidFile;
    }

    Status status = uploadGigEFPGAFirmware(dev, fwData, progressFunc);

    if (succeeded(status))
    {
        if (rebootCamera(dev))
        {
            return status;
        }
        else
        {
            // Automatic reboot did not work, hw disconnect required
            return Status::SuccessDisconnectRequired;
        }
    }
    return status;
}


Status upgradeFPGAFirmware (IFirmwareWriter& dev, const std::string& fileName, const std::string& modelName, std::function<void(int, const std::string&)> progressFunc)
{
    if (isPackageFile(fileName))
    {
        return upgradeFPGAFirmwareFromPackage(dev, fileName, modelName, progressFunc);
    }
    else
    {
        return upgradeFPGAFirmwareDirect(dev, fileName, progressFunc);
    }
}


Status upgradeFirmware (IFirmwareWriter& dev,
                        const Packet::ACK_DISCOVERY& disc,
                        const std::string& fileName,
                        const std::string& overrideModelName,
                        std::function<void(int,const std::string&)> progressFunc)
{
    int typeId;
    std::string cameraModelName;
    if (!getDeviceInfo(disc, typeId, cameraModelName))
    {
        return Status::DeviceNotRecognized;
    }

    auto modelName = overrideModelName.empty() ? cameraModelName : overrideModelName;

    uint32_t version;
    queryDeviceFPGAVersion( dev, version);

    Status rval = Status::DeviceNotRecognized;
    switch (typeId)
    {
        case 0: // Blackfin-based GigECam
            rval = upgradeBlackfinFirmware(dev, fileName, progressFunc);
            break;
        case 1: // GigEFPGA
        case 2: // GigEFPGA with MachXO2
            rval = upgradeFPGAFirmware(dev, fileName, modelName, progressFunc);
            break;
        case 3: // GigE3 (new .fwpack format)
        case 4:
            auto func = [=] (int i, const std::string& s)
                {
                    progressFunc(i, s);
                };
            rval = GigE3::upgradeFirmware(dev, fileName, modelName, cameraModelName, func);
            break;
    }

    return rval;
}

} /* namespace FirmwareUpdate */
