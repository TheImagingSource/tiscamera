
#ifndef TCAM_INDEXER
#define TCAM_INDEXER

#include "DeviceInfo.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "compiler_defines.h"

// VISIBILITY_INTERNAL

namespace tcam
{

#ifndef dev_callback

typedef void (*dev_callback)(const DeviceInfo&, void* user_data);

#endif /* dev_callback */


class Indexer
{
public:
    static std::shared_ptr<Indexer> get_instance();

    std::vector<DeviceInfo> get_device_list() const;

    void register_device_lost(dev_callback cb, void* user_data);

    void register_device_lost(dev_callback cb, void* user_data, const std::string& serial);

    void remove_device_lost(dev_callback callback);
    void remove_device_lost(dev_callback callback, const std::string& serial);

private:
    // The Indexer is a pseudo singleton
    // It stores a weak_ptr to itself and returns it,
    // should multiple instances of the class be requested
    // This design exists because the dynamically loaded backends
    // can cause problems when being unloaded during stack unwinding
    // while the indexing thread is still running.
    // To prevent this the indexing thread is stopped once all DeviceIndex
    // instances cease to exist.
    static std::weak_ptr<Indexer> indexer_ptr;

    Indexer();
    ~Indexer();

    void update_device_list_thread();
    std::vector<DeviceInfo> fetch_device_list_backend() const;
    static void sort_device_list(std::vector<DeviceInfo>& lst);

    bool continue_thread_ = true;
    mutable std::mutex mtx_;
    unsigned int wait_period_ = 2;
    std::atomic<bool> have_list_ = false;
    std::thread work_thread_;

    mutable std::condition_variable wait_for_list_;
    mutable std::condition_variable wait_for_next_run_;

    std::vector<DeviceInfo> device_list_;

    struct callback_data
    {
        dev_callback callback;
        void* data;
        std::string serial;
    };

    std::vector<callback_data> callbacks_;
};


} // namespace tcam

// VISIBILITY_POP

#endif /* TCAM_INDEXER */
