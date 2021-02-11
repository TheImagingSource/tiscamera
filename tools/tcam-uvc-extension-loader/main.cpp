/*
 * Copyright 2018 The Imaging Source Europe GmbH
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

#include "CLI11.hpp"
#include "uvc-extension-loader.h"

#include <cstring>
#include <fcntl.h> // O_RDWR
#include <unistd.h> // close()
#include <vector>


int main(int argc, char** argv)
{
    // The following exit codes exist:
    // 0 - everything is ok
    // 1 - device was not found
    // 3 - file was not found/loaded

    CLI::App app { "UVC extension unit loader" };

    std::string device = "/dev/video0";
    app.add_option(
        "-d,--device", device, "Device to which the extension unit shall be applied.", true);


    std::string description_file;
    auto f = app.add_option("-f,--file",
                            description_file,
                            "JSON file containing the extension unit that shall be applied.",
                            false);

    f->required();
    f->check(CLI::ExistingFile);

    bool verbose_output = false;
    app.add_flag("-v,--verbose", verbose_output, "Print additional output.");

    CLI11_PARSE(app, argc, argv);

    auto message_cb = [&verbose_output](const std::string& message) {
        if (verbose_output)
        {
            printf("%s\n", message.c_str());
        }
    };

    // for loading the file we always want verbose output
    bool tmp_output = verbose_output;
    verbose_output = true;
    // load file
    auto mappings = tcam::uvc::load_description_file(description_file, message_cb);
    if (mappings.empty())
    {
        return 3;
    }
    verbose_output = tmp_output;

    // load device
    int fd = open(device.c_str(), O_RDWR | O_NONBLOCK, 0);

    if (fd == -1)
    {
        printf("Unable to open device \"%s\": %s\n", device.c_str(), strerror(errno));
        return 1;
    }

    // apply file to device
    tcam::uvc::apply_mappings(fd, mappings, message_cb);

    close(fd);

    return 0;
}
