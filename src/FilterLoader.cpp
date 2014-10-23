

#include "FilterLoader.h"
#include "FilterBase.h"
#include "logging.h"

#include "filter/AutoPassFilter.h"
#include "filter/BayerRgbFilter.h"

using namespace tis_imaging;


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
