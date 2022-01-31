
#ifndef TCAM_LIBUSB_UTILS_H
#define TCAM_LIBUSB_UTILS_H

#include "../DeviceInfo.h"
#include "../ImageBuffer.h"
#include "../SinkInterface.h"

#include <vector>

#include <condition_variable>
#include <thread>
#include <mutex>

namespace tcam::libusb
{

std::vector<DeviceInfo> get_libusb_device_list();


class deliver_thread
{
public:
    bool push(std::shared_ptr<tcam::ImageBuffer>&& ptr);

    void start(const std::shared_ptr<IImageBufferSink>& list);
    void stop();
private:
    void thread_main();

    std::thread thread_;
    std::vector<std::shared_ptr<tcam::ImageBuffer>> queue_;
    std::condition_variable cv_;
    std::mutex mutex_;

    bool end_thread_ = false;

    std::shared_ptr<IImageBufferSink> sink_;
};

} // namespace tcam::libusb

#endif /* TCAM_LIBUSB_UTILS_H */
