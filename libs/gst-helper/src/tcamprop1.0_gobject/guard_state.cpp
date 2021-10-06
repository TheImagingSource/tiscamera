
#include "guard_state.h"

#include <shared_mutex>

namespace tcamprop1_gobj::impl::detail
{
    struct guard_state_obj
    {
        std::shared_mutex   mtx;
        guard_state_enum    actual_state_ = guard_state_enum::alive;
    };
}

auto tcamprop1_gobj::impl::create_guard_state_handle() -> tcamprop1_gobj::impl::guard_state_handle
{
    return std::make_shared<tcamprop1_gobj::impl::detail::guard_state_obj>();
}

tcamprop1_gobj::impl::guard_state_raii::guard_state_raii( const guard_state_handle& ptr )
    : data_{ ptr }
{
    if( data_ != nullptr ) {
        data_->mtx.lock_shared();
        owning_lock_ = true;
    }
}

tcamprop1_gobj::impl::guard_state_raii::~guard_state_raii()
{
    if( data_ != nullptr ) {
        owning_lock_ = false;
        data_->mtx.unlock_shared();
    }
}

bool tcamprop1_gobj::impl::guard_state_raii::owning_lock() const noexcept
{
    if( data_ == nullptr ) {
        return true;
    }
    return data_->actual_state_ == guard_state_enum::alive;
}

auto    tcamprop1_gobj::impl::guard_state_raii::state() const noexcept -> guard_state_enum
{
    if( data_ == nullptr ) {
        return guard_state_enum::empty_guard;
    }
    return data_->actual_state_;
}

tcamprop1_gobj::impl::guard_state_raii_exclusive::guard_state_raii_exclusive( const guard_state_handle& ptr )
    : data_{ ptr }
{
    if( data_ != nullptr ) {
        data_->mtx.lock();
    }
}

tcamprop1_gobj::impl::guard_state_raii_exclusive::~guard_state_raii_exclusive()
{
    if( data_ != nullptr ) {
        data_->mtx.unlock();
    }
}

void tcamprop1_gobj::impl::guard_state_raii_exclusive::mark_closed() noexcept
{
    if( data_ ) {
        data_->actual_state_ = guard_state_enum::closed;
    }
}
