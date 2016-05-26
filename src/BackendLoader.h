



#ifndef TCAM_BACKEND_LOADER_H
#define TCAM_BACKEND_LOADER_H


#include <vector>
#include <string>
#include <memory>

#include "base_types.h"

#include "DeviceInterface.h"

namespace tcam
{

class BackendLoader
{
private:

    BackendLoader ();
    ~BackendLoader ();

    BackendLoader& operator= (const BackendLoader&) = delete;

    struct backend
    {
        enum TCAM_DEVICE_TYPE type;
        std::string name;
        void* handle;

        // std::function<std::vector<DeviceInfo>()> get_device_list;
        // std::function<std::shared_ptr<DeviceInterface>(const DeviceInfo&)> open_device;
        std::function<size_t(struct tcam_device_info*, size_t)> get_device_list;
        std::function<size_t()> get_device_list_size;
        std::function<tcam::DeviceInterface*(const struct tcam_device_info*)> open_device;
    };

    std::vector<backend> backends;

    void load_backends ();


    void unload_backends ();

public:

    static BackendLoader& getInstance ();

    std::shared_ptr<DeviceInterface> open_device (const DeviceInfo&);

    std::vector<DeviceInfo> get_device_list_all_backends ();

    std::vector<DeviceInfo> get_device_list (enum TCAM_DEVICE_TYPE);

}; /* class BackendLoader*/


} /* namespace tcam */

#endif /* TCAM_BACKEND_LOADER_H */
