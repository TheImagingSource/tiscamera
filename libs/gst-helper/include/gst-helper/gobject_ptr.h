

#pragma once

#include <glib-object.h>
#include <utility>  // std::exchange

/** gobject_ptr
 * 
 * gobject_ptr is a counting shared pointer.
 * You create a gobject_ptr using  (When in doubt use make_ptr)
    - make_ptr (the same as make_consume_ptr)
    - make_wrap_ptr
        When wrapping, the ref-count of the object is not touched.
    - make_addref_ptr
        When wrapping, the ref-count is increased by one, if the object is floating, the floating flag is removed.
    - make_consume_ptr (this is the same as make_ptr)
        When wrapping, the floating flag is removed, but addref is not called.
 * The destructor always decrements the ref-count.
 * 
 */

namespace gobject_helper
{
    /** Simple wrapper like shared_ptr/unique_ptr.
     * You can create instances via the make* functions.
     */
    template<class T> class gobject_ptr;

    /** The default way to create a gobject.
     * Wraps the passed in pointer and 'consumes' the gobject floating flag.
     * Note: this calls make_consume_ptr
     */
    template<class T> gobject_ptr<T> make_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and increments the reference count of the passed in object.
     * If the object is floating, the floating flag is removed (via g_object_ref_sink and no separate addref is called).
     */
    template<class T> gobject_ptr<T> make_addref_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and 'consumes' the gobject floating flag.
     * If the object is floating, the floating flag is removed,
     * If the object is not floating its reference count is not touched.
     */
    template<class T> gobject_ptr<T> make_consume_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and does not change the ref-count or floating flag.
     * Note: The destructor does still run and decreases the ref-count
     */
    template<class T> gobject_ptr<T> make_wrap_ptr( T* ptr ) noexcept;

namespace detail
{
template<class T>
inline bool consume_floating( T* ptr ) noexcept
{
    if( g_object_is_floating( ptr ) )
    {
        g_object_ref_sink( ptr );
        return true;
    }
    return false;
}

} // namespace detail

template<class T> class gobject_ptr
{
public:
    gobject_ptr() = default;

    gobject_ptr(std::nullptr_t) noexcept {}

    gobject_ptr(const gobject_ptr& op2) noexcept : ptr_ { internal_ref_ptr(op2.ptr_) } {}
    gobject_ptr(gobject_ptr&& op2) noexcept : ptr_ { std::exchange(op2.ptr_, nullptr) } {}
    gobject_ptr& operator=(const gobject_ptr& op2) noexcept
    {
        internal_unref_ptr(ptr_);
        ptr_ = internal_ref_ptr(op2.ptr_);
        return *this;
    }
    gobject_ptr& operator=(gobject_ptr&& op2) noexcept
    {
        internal_unref_ptr(ptr_);
        ptr_ = std::exchange(op2.ptr_, nullptr);
        return *this;
    }
    ~gobject_ptr()
    {
        internal_unref_ptr( ptr_ );
    }

    /** Wraps the passed in pointer and does not change the ref-count or floating flag.
     * Note: The destructor does still run and decreases the ref-count
     */
    static gobject_ptr wrap(T* ptr) noexcept
    {
        return gobject_ptr { ptr };
    }
    /** Wraps the passed in pointer and 'consumes' the gobject_ptr floating flag.
     * If the object is floating, the floating flag is removed,
     * If the object is not floating its reference count is not touched.
     */
    static gobject_ptr consume(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gobject_ptr {};
        }
        detail::consume_floating( ptr );
        return gobject_ptr { ptr };
    }
    /** Wraps the passed in pointer and increments the reference count of the passed in object.
     * If the object is floating, the floating flag is removed (via g_object_ref_sink and no separate addref is called).
     */
    static gobject_ptr addref(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gobject_ptr {};
        }
        if (!detail::consume_floating(ptr))
        {
            internal_ref_ptr(ptr);
        }
        return gobject_ptr { ptr };
    }

    /**
     * Checks if the internal pointer is == nullptr
     */
    bool empty() const noexcept
    {
        return ptr_ == nullptr;
    }

    T* get() const noexcept
    {
        return ptr_;
    }

    explicit operator bool() const noexcept
    {
        return !empty();
    }

    T& operator*() noexcept
    {
        return *ptr_;
    }
    const T& operator*() const noexcept
    {
        return *ptr_;
    }

    T* operator->() noexcept
    {
        return ptr_;
    }
    const T* operator->() const noexcept
    {
        return ptr_;
    }
    /**
     * Decrements the reference counter and then sets the internal pointer to nullptr.
     * Post-condition: empty() == true
     * Note: decrement_ref only decrements the reference counter, reset also sets the internal pointer to nullptr
     */
    void reset() noexcept
    {
        internal_unref_ptr(ptr_);
    }
    /**
     * Increments the reference counter.
     */
    void    add_ref() noexcept {
        internal_ref_ptr( ptr_ );
    }
    /**
     * Decrements the reference counter. N
     * This does _not_ set the internal pointer to nullptr.
     * Note: decrement_ref only decrements the reference counter, reset also sets the internal pointer to nullptr
     */
    void    decrement_ref() noexcept {
        if( !ptr_ ) return;
        
        g_object_unref( ptr_ );
    }

    /**
     * Returns the internal pointer. Then sets the internal pointer to nullptr.
     * Note: this does not call unref
     * Post-condition: empty() == true
     */
    [[nodiscard]] T* release() noexcept
    {
        return std::exchange( ptr_, nullptr );
    }
private:
    gobject_ptr(T* ptr) noexcept : ptr_ { ptr } {}

    T* ptr_ = nullptr;

    static void internal_unref_ptr(T*& ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return;
        }
        g_object_unref(std::exchange( ptr, nullptr ));
    }
    static T* internal_ref_ptr(T* ptr) noexcept
    {
        if (ptr == nullptr) {
            return nullptr;
        }

        g_object_ref(ptr);
        return ptr;
    }
};

template<class T> bool operator==(std::nullptr_t, const gobject_ptr<T>& ptr) noexcept
{
    return ptr.empty();
}
template<class T> bool operator==(const gobject_ptr<T>& ptr, std::nullptr_t) noexcept
{
    return ptr.empty();
}

template<class T> bool operator!=(std::nullptr_t, const gobject_ptr<T>& ptr) noexcept
{
    return !ptr.empty();
}
template<class T> bool operator!=(const gobject_ptr<T>& ptr, std::nullptr_t) noexcept
{
    return !ptr.empty();
}

template<class T> gobject_ptr<T> make_ptr(T* ptr) noexcept
{
    return gobject_ptr<T>::consume(ptr);
}
template<class T> gobject_ptr<T> make_addref_ptr(T* ptr) noexcept
{
    return gobject_ptr<T>::addref(ptr);
}
template<class T> gobject_ptr<T> make_consume_ptr(T* ptr) noexcept
{
    return gobject_ptr<T>::consume(ptr);
}
template<class T> gobject_ptr<T> make_wrap_ptr(T* ptr) noexcept
{
    return gobject_ptr<T>::wrap(ptr);
}


} // namespace gst_helper
