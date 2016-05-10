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

#pragma once

#include <functional>
#include <vector>
#include <cstdint>
#include <cstring>

namespace I2C
{

typedef std::vector<uint8_t> DataArray;
typedef std::function<void(uint8_t, const DataArray&, bool)> WriteAction;
typedef std::function<DataArray(uint8_t, uint16_t, bool)> ReadAction;

class I2CDevice
{
    uint8_t _dev;
    WriteAction _write;
    ReadAction _read;
    size_t _maxReadLength;

public:
    I2CDevice (uint8_t dev, WriteAction write, ReadAction read, size_t maxReadLength)
        : _dev( dev ),
          _write( write ),
          _read( read ),
          _maxReadLength( maxReadLength )
    {}

public:
    size_t maxReadLength () { return _maxReadLength; }

private:
    DataArray I2CTransaction (const DataArray& args, uint16_t requestLength)
    {
        _write( _dev, args, true);
        return _read(_dev, requestLength, true);
    }

public:
    template<typename T, uint16_t TCommandSize>
    T read (const uint8_t (&command)[TCommandSize])
    {
        DataArray arr(std::begin(command), std::end(command));

        return read<T>(arr);
    }

    template<typename T>
    T read (const DataArray& command)
    {
        auto buffer = I2CTransaction(command, sizeof(T));

        T result;
        memcpy(&result, buffer.data(), sizeof(T));
        return result;
    }

    template<uint16_t TCommandSize>
    void write (const uint8_t( &command )[TCommandSize])
    {
        DataArray arr(std::begin(command), std::end(command));

        return write(arr);
    }

    template<typename TArg, uint16_t TCommandSize>
    void write (const uint8_t(&command)[TCommandSize], const TArg& arg)
    {
        DataArray arr(std::begin(command), std::end(command));

        arr.resize(TCommandSize + sizeof(arg));
        memcpy(arr.data() + TCommandSize, &arg, sizeof(arg));

        return write( arr );
    }

    void write (const DataArray& command)
    {
        _write(_dev, command, false);
    }

    template<uint16_t TCommandSize>
    DataArray transaction (const uint8_t (&command)[TCommandSize], uint16_t requestLength)
    {
        DataArray arr (std::begin(command), std::end(command));

        return I2CTransaction(arr, requestLength);
    }

}; /* class I2CDevice*/

} /* I2C */
