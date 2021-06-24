
#include "scope_profiler_impl.h"

#include <array>
#include <chrono>
#include <cassert>
#include <cstring>

namespace
{
    using namespace scope_profiler;

    static constexpr const int scope_timing_pre_alloc_element_count = 128;
    static constexpr const int scope_timing_max_direct_children_count = 32;

    //struct monotonic_pool_alloc
    //{
    //    using value_type = std::byte;

    //    monotonic_pool_alloc( size_t buffer_size ) : data_( buffer_size ) {}

    //    monotonic_pool_alloc( const monotonic_pool_alloc& ) = delete;
    //    monotonic_pool_alloc( monotonic_pool_alloc&& ) = delete;
    //    monotonic_pool_alloc& operator=( const monotonic_pool_alloc& ) = delete;
    //    monotonic_pool_alloc& operator=( monotonic_pool_alloc&& ) = delete;

    //    [[nodiscard]] std::byte* allocate( std::size_t bytes_needed )
    //    {
    //        if( (cur_pos_ + bytes_needed) > data_.size() ) {
    //            throw std::bad_alloc();
    //        }
    //        auto p = data_.data() + cur_pos_;
    //        cur_pos_ += bytes_needed;
    //        return p;
    //    }

    //    void    deallocate( std::byte* /*p*/, std::size_t /*n*/ ) noexcept {
    //        // do nothing
    //    }
    //    void    reset() noexcept {
    //        cur_pos_ = 0;
    //    }
    //private:
    //    std::vector<std::byte>  data_;
    //    int                     cur_pos_ = 0;
    //};

    class accu_section_provider : public scope_profiler::profiler_interface
    {
    private:
        struct section_entry
        {
        public:
            const auto&         children() const noexcept { return children_; }
            std::string_view    section_id() const noexcept { return std::string_view{ name_, name_len_ }; }
            scope_timing        to_scope_timing( int layer ) const { return scope_timing{ std::string( name_, name_len_ ), time_accu_, 0, call_count_, layer }; }

            void    reset() noexcept {
                children_ = {};
                call_count_ = 0;
                time_accu_ = 0;
            }

            void    setup( std::string_view txt, section_entry* prev_entry, int64_t start_time ) noexcept
            {
                name_len_ = std::min( sizeof( name_ ) - 1, txt.size() );
                memcpy( name_, txt.data(), name_len_ );

                prev_layer_ = prev_entry;
                enter_time_ = start_time;
            }
            section_entry*  enter_layer( int64_t now ) noexcept
            {
                enter_time_ = now;
                return this;
            }

            section_entry*  leave_layer( int64_t now ) noexcept
            {
                time_accu_ += now - enter_time_;
                ++call_count_;
                return prev_layer_;
            }

            section_entry*    push_child_layer( accu_section_provider& alloc, std::string_view name, int64_t now )
            {
                for( auto& entry : children_ )
                {
                    if( entry == nullptr ) {
                        entry = alloc.alloc_entry( this, name, now );
                        return entry;
                    }
                    if( entry->section_id() == name ) {
                        return entry->enter_layer( now );
                    }
                }
                return nullptr;
            }
            auto            time_accu() const noexcept { return time_accu_; }
        private:
            char            name_[128] = {};
            size_t          name_len_ = 0;

            section_entry*  prev_layer_ = nullptr;
            int64_t         time_accu_ = 0;
            int             call_count_ = 0;

            std::array<section_entry*, scope_timing_max_direct_children_count>	children_ = {};

            int64_t         enter_time_ = 0;
        };
    public:
        accu_section_provider() noexcept = default;

        void*   push_layer_entry( std::string_view text, int64_t now ) noexcept final
        {
            section_entry* entry = push_layer( cur_layer_, text, now );
            assert( entry || "Failed to find existing entry and did not have enough room to add new entry" );
            if( entry ) {
                cur_layer_ = entry;
            }
            return entry;
        }
        void	pop_layer_entry( void* entry, int64_t now ) noexcept final
        {
            cur_layer_ = static_cast<section_entry*>(entry)->leave_layer( now );
        }

        void	reset() noexcept final
        {
            for( auto& entry : entry_prealloc_list_ ) {
                entry.reset();
            }
            entries_allocated_ = 0;
            root_layer_ = {};
        }

        std::vector<std::vector<scope_timing>>  dump_scope_timing_lists() final
        {
            std::vector<std::vector<scope_timing>> rval;
            for( auto* e : root_layer_ ) {
                if( e == nullptr ) {
                    break;
                }
                rval.push_back( enumerate_scoped_timing_sub_entries( *e, 0 ) );
            }
            return rval;
        }
    private:
        std::vector<scope_timing> enumerate_scoped_timing_sub_entries( const section_entry& e, int layer )
        {
            int64_t time_accu_direct_children = 0;

            std::vector<scope_timing> lst;
            lst.push_back( e.to_scope_timing( layer ) );
            for( auto* child : e.children() )
            {
                if( child == nullptr ) {
                    break;
                }

                time_accu_direct_children += child->time_accu();

                for( auto&& child_timing_entry : enumerate_scoped_timing_sub_entries( *child, layer + 1 ) )
                {
                    lst.push_back( std::move( child_timing_entry ) );
                }
            }
            lst.front().time_accu_exclusive = lst.front().time_accu_inclusive - time_accu_direct_children;
            return lst;
        }

        section_entry* alloc_entry( section_entry* parent, std::string_view text, int64_t now ) noexcept
        {
            assert( entries_allocated_ < entry_prealloc_list_.size() );

            auto& res = entry_prealloc_list_[entries_allocated_++];
            res.setup( text, parent, now );
            return &res;
        }

        section_entry*      push_layer( section_entry* prev, std::string_view section_id, int64_t now ) noexcept
        {
            if( prev != nullptr ) {
                return prev->push_child_layer( *this, section_id, now );
            }
            for( auto& entry : root_layer_ )
            {
                if( entry == nullptr ) {
                    entry = alloc_entry( prev, section_id, now );
                    return entry;
                }
                if( entry->section_id() == section_id ) {
                    return entry->enter_layer( now );
                }
            }
            return nullptr;
        }

        size_t          entries_allocated_ = 0;
        std::array<section_entry, scope_timing_pre_alloc_element_count>     entry_prealloc_list_;
        std::array<section_entry*, scope_timing_max_direct_children_count>  root_layer_ = {};

        section_entry* cur_layer_ = nullptr;
    };

    thread_local    profiler_interface* thread_local_profiler_ = nullptr;
}

scope_profiler::profiler_interface* detail::get_instance() noexcept
{
    return thread_local_profiler_;
}

int64_t    scope_profiler::detail::query_time_now_in_us() noexcept
{
    using namespace std::chrono;
    return duration_cast<microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

profiler_interface* scope_profiler::register_threadlocal_profiler( profiler_interface* ptr ) noexcept
{
    auto tmp = thread_local_profiler_;
    thread_local_profiler_ = ptr;
    return tmp;
}

std::unique_ptr<scope_profiler::profiler_interface> scope_profiler::create_profiler()
{
    return std::make_unique<accu_section_provider>();
}

void*   scope_profiler::detail::push_threaded_entry( profiler_interface& itf, std::string_view name ) noexcept
{
    return itf.push_layer_entry( name, query_time_now_in_us() );
}

void    scope_profiler::detail::pop_threaded_entry( void* entry_to_pop ) noexcept
{
    auto ptr = get_instance();
    if( ptr == nullptr ) {
        return;
    }
    return ptr->pop_layer_entry( entry_to_pop, query_time_now_in_us() );
}
