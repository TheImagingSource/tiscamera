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

#include "FilterLoader.h"
#include "FilterBase.h"
#include "logging.h"

#include "filter/AutoPassFilter.h"
#include "filter/BayerRgbFilter.h"

using namespace tcam;


FilterLoader::FilterLoader ()
{
    loaded_filter.push_back(std::make_shared<AutoPassFilter>());
    loaded_filter.push_back(std::make_shared<BayerRgbFilter>());
}


std::vector<std::shared_ptr<FilterBase>> FilterLoader::get_all_filter ()
{
    return loaded_filter;
}


bool FilterLoader::drop_all_filter ()
{
    // loaded_filter.clear();

    return true;
}


void FilterLoader::index_possible_filter ()
{}


void FilterLoader::open_possible_filter ()
{}
