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

#include "JedecFile.h"

#include <algorithm>
#include <string>

using namespace MachXO2;

namespace
{

enum class RowType
{
    COMMENT,
    FUSE_CHECKSUM,
    FUSE_DATA,
    END_DATA,
    FUSE_LIST,
    SECURITY_FUSE,
    FUSE_DEFAULT,
    FUSE_SIZE,
    USER_CODE,
    FEATURE_ROW,
    FEATURE_BITS,
    DONE
};


template<typename TCharIterator>
std::string getline (TCharIterator& begin, TCharIterator end, std::string delim)
{
    std::string line;

    for ( ; begin != end && !std::equal(delim.begin(), delim.end(), begin); ++begin)
    {
        line += *begin;
    }

    if (begin != end)
    {
        begin += delim.length();
    }

    return line;
}


bool string_startswith (const std::string& s, const std::string& start)
{
    return s.length() >= start.length() && s.substr(0, start.length()) == start;
}


bool string_contains (const std::string& str, const std::string& sub)
{
    return str.find(sub) != std::string::npos;
}


void ParseFuseLine (const std::string& line, std::vector<uint8_t>& dest)
{
    int n = 0;

    for (int i = 0; i < 16; i++)
    {
        uint8_t val = 0;
        for (int j = 0; j < 8; j++)
        {
            val = (uint8_t)((val << 1) | (line[n] - '0'));
            ++n;
        }

        dest.push_back(val);
    }
}


std::vector<uint8_t> ParseFeatureRow (const std::string& line, int len)
{
    std::vector<uint8_t> result(len);
    int n = 8 * len - 1;

    for (int i = 0; i < len; i++)
    {
        uint8_t val = 0;
        for (int j = 0; j < 8; j++)
        {
            val = (uint8_t)((val << 1) | (line[n] - '0'));
            --n;
        }

        result[i] = val;
    }

    return result;
}


uint32_t ParseUserCode (const std::string& data)
{
    if (data[0] == 'H')
    {
        uint32_t uc;
        sscanf( data.c_str(), "H%08X", &uc );
        return uc;
    }
    else
    {
        uint32_t val = 0;
        for (size_t i = 0; i < 32 && i < data.length(); ++i)
        {
            val = (val << 1) | (uint32_t)(data[i] - '0');
        }

        return val;
    }
}


std::string RemoveTrailingStar (const std::string& s)
{
    if (s[s.length() - 1] == '*')
        return s.substr(0, s.length() - 1);
    else
        return s;
}

} /* namespace */


JedecFile JedecFile::Parse (const std::vector<uint8_t>& data)
{
    RowType state = RowType::COMMENT;

    DeviceInfo devInfo;
    int pageCount = 0;
    int cfgPageCount = 0;
    int ufmPageCount = 0;
    std::vector<uint8_t> configurationData;
    std::vector<uint8_t> ufmData;
    std::vector<uint8_t> featureRow;
    std::vector<uint8_t> featureBits;
    uint32_t userCode = 0;

    auto pos = data.begin();
    while (pos != data.end())
    {
        auto line = getline(pos, data.end(), "\r\n");

        if (state == RowType::FEATURE_ROW)
            state = RowType::FEATURE_BITS;
        else if (line[0] == '0' || line[0] == '1')
            state = RowType::FUSE_DATA;
        else if (string_startswith(line, "NOTE"))
            state = RowType::COMMENT;
        else if (string_startswith( line, "G"))
            state = RowType::SECURITY_FUSE;
        else if (string_startswith(line, "L"))
            state = RowType::FUSE_LIST;
        else if (string_startswith( line, "C"))
            state = RowType::FUSE_CHECKSUM;
        else if(string_startswith( line, "*"))
            state = RowType::END_DATA;
        else if(string_startswith( line, "D"))
            state = RowType::FUSE_DEFAULT;
        else if(string_startswith( line, "U"))
            state = RowType::USER_CODE;
        else if(string_startswith(line, "E"))
            state = RowType::FEATURE_ROW;
        else if(string_startswith( line, "QF"))
            state = RowType::FUSE_SIZE;
        else if(string_startswith(line, "\x03"))
            state = RowType::DONE;

        switch (state)
        {
            case RowType::FUSE_DATA:
                ++pageCount;
                if (pageCount <= devInfo.numCfgPages())
                {
                    ++cfgPageCount;
                    ParseFuseLine(line, configurationData);
                }
                else
                {
                    ++ufmPageCount;
                    ParseFuseLine(line, ufmData);
                }

                break;

            case RowType::FEATURE_ROW:
                featureRow = ParseFeatureRow(line.substr(1), 8);
                break;
            case RowType::FEATURE_BITS:
                featureBits = ParseFeatureRow(line, 2);
                break;
            case RowType::USER_CODE:
                userCode = ParseUserCode(RemoveTrailingStar(line.substr(1)));
                break;
        }

        if (state == RowType::COMMENT && string_contains(line, "DEVICE NAME:"))
        {
            devInfo = DeviceInfo::Find(line);
        }

        if (state == RowType::DONE)
        {
            break;
        }
    }

    return { devInfo.type(), userCode, configurationData, featureRow, featureBits };
}
