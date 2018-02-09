
#pragma once

#include <atomic>
#include <mutex>
#include <cassert>
#include <condition_variable>

namespace threading
{
// Simple latch with reset, note that reset depends on all waiting threads to be already released
class latch
{
public:
    explicit latch( int cnt = 0 ) noexcept
        : count_( cnt )
    {
    }
    ~latch() = default;

    void    count_down() noexcept
    {
        std::unique_lock<std::mutex> lck( signal_locked_ );
        int res = count_.fetch_sub( 1 );
        if( res == 1 ) {
            signal_.notify_all();
        }
    }
    void    wait() noexcept
    {
        if( count_ == 0 ) {       // if count is already 0, we don't have to wait
            return;
        }
        std::unique_lock<std::mutex> lck( signal_locked_ );
        signal_.wait( lck, [this] { return count_ == 0; } );     // count is still > 0, so wait on the signal
    }
    bool    try_wait() noexcept
    {
        return count_ == 0;
    }
    void    count_down_and_wait() noexcept
    {
        std::unique_lock<std::mutex> lck( signal_locked_ );
        int res = count_.fetch_sub( 1 );
        if( res > 1 ) {
            signal_.wait( lck, [this]{ return count_ == 0; } );     // count is still > 0, so wait on the signal
        }
        else
        {
            signal_.notify_all();    // we were the last
        }
    }

    void    reset( int cnt ) noexcept
    {
        count_ = cnt;
    }
private:
    std::atomic<int>        count_;

    std::mutex              signal_locked_;

    std::condition_variable signal_;
};
}
