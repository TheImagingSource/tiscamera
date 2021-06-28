
#pragma once

#include <string_view>

namespace scope_profiler
{
    struct profiler_interface;

    namespace detail
    {
        void*                   push_threaded_entry( profiler_interface& itf, std::string_view name ) noexcept;
        void                    pop_threaded_entry( void* entry_to_pop ) noexcept;

        profiler_interface*     get_instance() noexcept;
    }

    class profiler_threaded
    {
    public:
        profiler_threaded() = delete;
        profiler_threaded( const profiler_threaded& ) = delete;
        profiler_threaded( profiler_threaded&& ) = delete;

        static profiler_threaded   from_string( std::string_view section_name ) noexcept
        {
            if( auto ptr = detail::get_instance(); ptr ) {
                return profiler_threaded{ detail::push_threaded_entry( *ptr, section_name ) };
            }
            return profiler_threaded{ nullptr };
        }

        template<class TFunc>
        static profiler_threaded   from_func( TFunc func ) noexcept
        {
            if( auto ptr = detail::get_instance(); ptr ) {
                return profiler_threaded{ detail::push_threaded_entry( *ptr, func() ) };
            }
            return profiler_threaded{ nullptr };
        }
        ~profiler_threaded() { stop(); }

        void	stop() noexcept
        {
            if( section_entry_ == nullptr ) {
                return;
            }
            detail::pop_threaded_entry( section_entry_ );
        }
    private:
        explicit profiler_threaded( void* section ) noexcept : section_entry_( section ) {}

        void*           section_entry_ = nullptr;
    };
}