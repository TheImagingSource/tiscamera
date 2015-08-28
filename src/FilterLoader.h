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

#ifndef TCAM_FILTERLOADER_H
#define TCAM_FILTERLOADER_H

#include "FilterBase.h"

#include <string>
#include <vector>
#include <memory>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

class FilterLoader
{

public:

    FilterLoader ();

    /**
     * @return vector containing shared_ptr to all found filter
     */
    std::vector<std::shared_ptr<FilterBase>> get_all_filter ();

    /**
     * Forcefully closes all available filter
     * shared_ptr to filter will be invalid after this method
     */
    bool drop_all_filter ();

    void index_possible_filter ();

    /**
     * Drops opened filter and reinterates all known filter found through index_possible_filter
     */
    void open_possible_filter ();

private:

    std::vector<std::string> filter_filenames;

    struct filter_desc
    {
        void* lib;
        std::shared_ptr<FilterBase> filter;
    };

    std::vector<filter_desc> filter_collection;

    std::vector<std::shared_ptr<FilterBase>> loaded_filter;

};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_FILTERLOADER_H */
