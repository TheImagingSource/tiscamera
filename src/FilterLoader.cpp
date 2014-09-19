

#include "FilterLoader.h"
#include "FilterBase.h"
#include "tis_logging.h"

#include <glob.h>
#include <dlfcn.h> // dlopen

using namespace tis_imaging;


FilterLoader::FilterLoader ()
{

}


std::vector<std::shared_ptr<FilterBase>> FilterLoader::get_all_filter ()
{
    return loaded_filter;
}


bool FilterLoader::drop_all_filter ()
{
    loaded_filter.clear();

    return true;
}


void FilterLoader::index_possible_filter ()
{
    // TODO: find filter generically
    glob_t gs;
    int ret = glob("/home/edt/work/tis_imaging/filter/tis*.so", 0, NULL, &gs);

    if (ret != 0)
    {
        tis_log(TIS_LOG_ERROR, "Unable to query for filter libraries");
        // TODO: error
    }

    for (unsigned int i=0; i < gs.gl_pathc; ++i)
    {
        filter_filenames.push_back(std::string(gs.gl_pathv[i]));
    }
    globfree(&gs);

}


void FilterLoader::open_possible_filter ()
{
    if (filter_filenames.empty())
    {
        index_possible_filter();
    }

    loaded_filter.clear();

    for (const auto& f : filter_filenames)
    {
        filter_desc desc = {};

        desc.lib = dlopen(f.c_str(), RTLD_NOW);
        if (!desc.lib)
        {
            tis_log(TIS_LOG_ERROR,"Cannot load library: %s from file %s", dlerror(), f.c_str());
            continue;
        }

        create_filter* func_create = (create_filter*) dlsym(desc.lib, "create");
        destroy_filter* func_destroy = (destroy_filter*)dlsym(desc.lib, "destroy");

        FilterBase* fil = func_create();

        tis_log(TIS_LOG_INFO, "Found Filter \"%s\"", fil->getDescription().name.c_str());

        // loaded_filter.push_back(std::shared_ptr<FilterBase>(fil, *func_destroy));
        loaded_filter.push_back(std::shared_ptr<FilterBase>(fil));



    }

}

