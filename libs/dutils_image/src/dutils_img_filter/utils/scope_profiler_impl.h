
#pragma once

#include <cstdint>

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "scope_profiler_profiling.h"

namespace scope_profiler
{
    struct scope_timing
    {
        std::string     scope_name;

        int64_t	        time_accu_inclusive = 0;	            // Inclusive time for this entry and all its children (accumulated across all calls)
        int64_t         time_accu_exclusive = 0;    // Exclusive time for this entry, excluding children

        int	            call_count = 0;	// full call count
        int	            layer = 0;	    // the layer at which this was called
    };

    using scope_timing_lists = std::vector<std::vector<scope_timing>>;

    struct profiler_interface
    {
        virtual ~profiler_interface() = default;

        virtual void*   push_layer_entry( std::string_view text, int64_t now ) noexcept = 0;
        virtual void	pop_layer_entry( void* pEntry, int64_t now ) noexcept = 0;

        virtual void    reset() noexcept = 0;

        virtual scope_timing_lists  dump_scope_timing_lists() = 0;
    };

    bool                                    is_profiler_enabled() noexcept;
    std::unique_ptr<profiler_interface>     create_profiler();
    profiler_interface*                     register_threadlocal_profiler( profiler_interface* ptr ) noexcept;

    namespace detail
    {
        profiler_interface*     get_instance() noexcept;
        int64_t                 query_time_now_in_us() noexcept;
    }

    inline bool             is_profiler_enabled() noexcept { return detail::get_instance() != nullptr; }
}