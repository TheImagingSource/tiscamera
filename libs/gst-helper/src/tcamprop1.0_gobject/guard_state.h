
#pragma once

#include <memory>

namespace tcamprop1_gobj::impl
{
    namespace detail {
        struct guard_state_obj;
    }

    using guard_state_handle = std::shared_ptr<detail::guard_state_obj>;

    guard_state_handle create_guard_state_handle();

    enum class guard_state_enum {
        empty_guard,
        alive,
        closed, // state marked as closed
    };


    class guard_state_raii
    {
    private:
        guard_state_handle    data_;
        bool owning_lock_ = false;
    public:
        guard_state_raii( const guard_state_handle& wp );
        ~guard_state_raii();

        bool    owning_lock() const noexcept;
        
        guard_state_enum    state() const noexcept;

        explicit operator bool() const noexcept { return owning_lock(); }
    };

    class guard_state_raii_exclusive
    {
    private:
        guard_state_handle    data_;
    public:
        guard_state_raii_exclusive( const guard_state_handle& wp );
        ~guard_state_raii_exclusive();

        void    mark_closed() noexcept;
    };
}

