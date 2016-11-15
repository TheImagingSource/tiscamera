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

#include "FileHandling.h"

#include <zip.h>


std::vector<unsigned char> load_file (const std::string& filename)
{
    std::vector<unsigned char> contents;

    FILE* pF = fopen(filename.c_str(), "rb");
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


bool is_package_file (const std::string& fileName)
{
    std::string ending = ".fwpack";
    if (fileName.length() >= ending.length())
    {
        if (fileName.compare (fileName.length() - ending.length(), ending.length(), ending) == 0)
        {
            return true;
        }
    }

    return false;
}


std::vector<unsigned char> extract_file_from_package (const std::string& packageFileName,
                                                      const std::string& fileName)
{
    //Open the ZIP archive
    int err = 0;
    zip *z = zip_open(packageFileName.c_str(), 0, &err);

    //Search for the file of given name
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(z, fileName.c_str(), 0, &st);

    //Alloc memory for its uncompressed contents
    std::vector<unsigned char> result(st.size);

    //Read the compressed file
    zip_file *f = zip_fopen(z, fileName.c_str(), 0);

    zip_fread(f, result.data(), st.size);

    zip_close(z);

    return result;
}


bool has_ending (const std::string& fullString,  const std::string& ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}


bool is_valid_firmware_file (const std::string& firmware)
{
    std::string usb2_ending = ".euvc";
    std::string usb3_ending = ".fw";
    std::string usb3pack_ending = ".fwpack";

    if (has_ending(firmware, usb2_ending))
    {
        return true;
    }

    else if (has_ending(firmware, usb3_ending))
    {
        auto f = load_file(firmware);

        if (f.empty())
        {
            return false;
        }
        // all usb3 cameras have to begin with 0x4359 i.e. CY
        if (f[0] == 0x43 && f[1] == 0x59)
            return true;
    }
    else if (has_ending(firmware, usb3pack_ending))
    {
        return true;
    }

    return false;
}
