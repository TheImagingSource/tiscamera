


#include "libusb_utils.h"

#include "../utils.h"
#include "UsbHandler.h"

using namespace tcam;


std::vector<DeviceInfo> tcam::libusb::get_libusb_device_list()
{
    return UsbHandler::get_instance().get_device_list();
}

bool libusb::deliver_thread::push(std::shared_ptr<ImageBuffer>&& ptr)
{
    std::scoped_lock lck { mutex_ };
    if (end_thread_)
    {
        return false;
    }
    queue_.push_back(std::move(ptr));
    cv_.notify_one();
    return true;
}


void libusb::deliver_thread::stop()
{
    if (!thread_.joinable())
    {
        return;
    }

    {
        std::scoped_lock lck { mutex_ };
        end_thread_ = true;
        cv_.notify_all();
    }

    thread_.join();

    sink_.reset();
    queue_.clear();
}

void libusb::deliver_thread::start(const std::shared_ptr<IImageBufferSink>& sink)
{
    end_thread_ = false;

    sink_ = sink;

    thread_ = std::thread { [this]
                            {
                                thread_main();
                            } };
}

void libusb::deliver_thread::thread_main()
{
    tcam::set_thread_name("tcam-usb-dlv");

    while (true)
    {
        std::shared_ptr<tcam::ImageBuffer> ptr;
        {
            std::unique_lock lck { mutex_ };
            if (!end_thread_ && queue_.empty())
            {
                cv_.wait(lck);
            }
            if (end_thread_)
            {
                return;
            }
            if (!queue_.empty())
            {
                ptr = queue_.front();
                queue_.erase(queue_.begin());
            }
        }
        if (ptr)
        {
            sink_->push_image(ptr);
        }
    }
}