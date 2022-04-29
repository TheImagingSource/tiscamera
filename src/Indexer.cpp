
#include "Indexer.h"

#include "logging.h"
#include "utils.h"
#include "DeviceInterface.h"

#include <algorithm>

using namespace tcam;

std::weak_ptr<Indexer> Indexer::indexer_ptr;

Indexer::Indexer()
    : continue_thread_(true), wait_period_(2), have_list_(false)
{
    work_thread_ = std::thread(&Indexer::update_device_list_thread, this);
}


Indexer::~Indexer()
{
    continue_thread_ = false;
    wait_for_next_run_.notify_all();

    try
    {
        if (work_thread_.joinable())
        {
            work_thread_.join();
        }
    }
    catch (const std::system_error& err)
    {
        SPDLOG_ERROR("Unable to join thread. Exception: {}", err.what());
    }
}


std::shared_ptr<Indexer> Indexer::get_instance()
{
    auto obj = indexer_ptr.lock();

    if (!obj)
    {
        // Indexer() is private
        // we have to create a wrapper to pass is to make_shared
        struct make_shared_enabler : public Indexer
        {
        };

        obj = std::make_shared<make_shared_enabler>();


        indexer_ptr = obj;
    }

    return obj;
}


void Indexer::update_device_list_thread()
{
    tcam::set_thread_name("tcam_indexer");
    auto first_list = fetch_device_list_backend();

    std::unique_lock<std::mutex> lock(mtx_);
    device_list_ = first_list;
    have_list_ = true;
    wait_for_list_.notify_all();

    while (continue_thread_)
    {
        wait_for_next_run_.wait_for(lock, std::chrono::seconds(wait_period_));
        if (!continue_thread_)
        {
            break;
        }

        lock.unlock();

        auto tmp_dev_list = fetch_device_list_backend();

        lock.lock();

        std::vector<DeviceInfo> lost_list;

        for (const auto& d : device_list_)
        {
            auto f = [&d](const DeviceInfo& info)
            {
                if (d.get_serial().compare(info.get_serial()) == 0)
                {
                    return true;
                }
                return false;
            };

            auto found = std::find_if(tmp_dev_list.begin(), tmp_dev_list.end(), f);

            if (found == tmp_dev_list.end())
            {
                SPDLOG_INFO(
                    "Lost device {} - {}. Contacting callbacks", d.get_name(), d.get_serial());
                lost_list.push_back(d);
            }
        }

        device_list_ = std::move(tmp_dev_list);

        auto cbs = callbacks_;

        lock.unlock();

        for (auto&& d : lost_list)
        {
            for (auto& c : cbs)
            {
                if (c.serial.empty() || c.serial.compare(d.get_serial()) == 0)
                {
                    c.callback(d, c.data);
                }
            }
        }

        lock.lock();
    }
}


std::vector<DeviceInfo> Indexer::fetch_device_list_backend() const
{
    auto tmp_dev_list = tcam::get_device_list();

    sort_device_list(tmp_dev_list);
    return tmp_dev_list;
}


void Indexer::sort_device_list(std::vector<DeviceInfo>& lst)
{
    /*
      Sorting of devices shall create the following order:

      local before remote/network meaning v4l2, libusb, aravis
      sorted after user defined names
      sorted serials, if no name is given
    */

    auto compareDeviceInfo = [](const DeviceInfo& info1, const DeviceInfo& info2)
    {
        if (info1.get_device_type() >= info2.get_device_type())
        {
            if (info1.get_serial() > info2.get_serial())
            {
                return false;
            }
        }
        return true;
    };

    std::sort(lst.begin(), lst.end(), compareDeviceInfo);
}


std::vector<DeviceInfo> Indexer::get_device_list() const
{

    // wait for work_thread to deliver first valid list
    // since get_aravis_device_list is a blocking function
    // our thread would retrieve an empty list without this wait loop

    std::unique_lock<std::mutex> lock(mtx_);
    while (!have_list_) { wait_for_list_.wait_for(lock, std::chrono::seconds(wait_period_)); }
    return device_list_;
}


void Indexer::register_device_lost(dev_callback cb, void* user_data)
{
    std::lock_guard<std::mutex> lock(mtx_);
    callbacks_.push_back({ cb, user_data, "" });
}


void Indexer::register_device_lost(dev_callback cb, void* user_data, const std::string& serial)
{
    std::lock_guard<std::mutex> lock(mtx_);
    callbacks_.push_back({ cb, user_data, serial });
}


void Indexer::remove_device_lost(dev_callback callback)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = std::begin(callbacks_); //std::begin is a free function in C++11
    for (auto& value : callbacks_)
    {

        if (value.callback == callback)
        {
            callbacks_.erase(it);
            break;
        }
        it++; //at the end OR make sure you do this in each iteration
    }
}


void Indexer::remove_device_lost(dev_callback callback, const std::string& serial)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = std::begin(callbacks_);
    for (auto& value : callbacks_)
    {
        if (value.callback == callback && value.serial.compare(serial) == 0)
        {
            callbacks_.erase(it);
            break;
        }
        it++;
    }
}
