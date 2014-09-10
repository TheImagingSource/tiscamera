



#ifndef FILTERLOADER_H_
#define FILTERLOADER_H_

#include "FilterBase.h"

#include <string>
#include <vector>
#include <memory>

namespace tis_imaging
{

class FilterLoader
{

public:

    FilterLoader ();

    ~FilterLoader ();

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

} /* namespace tis_imaging */

#endif /* FILTERLOADER_H_ */










