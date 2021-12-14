/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_LIBRARYHANDLE_H
#define TCAM_LIBRARYHANDLE_H

#include <memory>
#include <functional>
#include <string>

namespace tcam
{

class LibraryHandle
{
public:
    /// @name: open
    /// @param: name - name of the library that shall be opened
    /// @param: optional path to the library
    /// @return shared_ptr or nullptr
    /// Tries to open the specified library
    static std::shared_ptr<LibraryHandle> open(const std::string& name,
                                               const std::string& path = "");

private:
    LibraryHandle(const std::string& name, const std::string& path = "");

    LibraryHandle(const LibraryHandle&) = delete;

    LibraryHandle& operator=(const LibraryHandle&) = delete;

public:
    ~LibraryHandle();

    template<class T> std::function<T> load(std::string const& functionName)
    {
        return reinterpret_cast<T*>(load_raw_function(functionName));
    }
private:
    static void* open_library(const std::string& name, const std::string& path = "");

    void* load_raw_function( const std::string& functionName );

    void* handle_ = nullptr;
}; // class LibraryHandle

} /* namespace tcam */

#endif /* TCAM_LIBRARYHANDLE_H */
