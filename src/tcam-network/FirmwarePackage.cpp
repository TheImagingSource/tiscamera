/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "FirmwarePackage.h"

#include <zip.h>

namespace
{
	template<typename TContainer>
	void internalExtractFile (const std::string& packageFileName,
                              const std::string& fileName,
                              TContainer& dest)
	{
        int err = 0;
        zip* z = zip_open(packageFileName.c_str(), 0, &err);

		if (!z)
        {
            return;
        }

        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(z, fileName.c_str(), 0, &st);

        //Alloc memory for its uncompressed contents
        char contents [st.size];

        //Read the compressed file
        zip_file *f = zip_fopen(z, fileName.c_str(), 0);
        if (f == nullptr)
        {
            return;
        }
        int ret = zip_fread(f, contents, st.size);

        if (ret != st.size)
        {
            return;
        }

        dest.assign(contents, contents + st.size);

        zip_close(z);
	}
}


std::vector<uint8_t> FirmwareUpdate::FirmwarePackage::extractFile (const std::string& packageFileName,
                                                                   const std::string& fileName)
{
	std::vector<uint8_t> data;
	internalExtractFile(packageFileName, fileName, data);

	return data;
}


std::string FirmwareUpdate::FirmwarePackage::extractTextFile (const std::string& packageFileName,
                                                              const std::string& fileName)
{
	std::string data;
	internalExtractFile(packageFileName, fileName, data);
	data.push_back('\0');

	return data;
}
