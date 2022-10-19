

#pragma once

#include <gst/gst.h>
#include <utility>  // std::exchange

/** gst_ptr
 * 
 * gst_ptr is a counting shared pointer.
 * You create a gst_ptr using  (When in doubt use make_ptr)
    - make_ptr (the same as make_consume_ptr)
    - make_wrap_ptr
        When wrapping, the ref-count of the object is not touched.
    - make_addref_ptr
        When wrapping, the ref-count is increased by one, if the object is floating, the floating flag is removed.
    - make_consume_ptr (this is the same as make_ptr)
        When wrapping, the floating flag is removed, but addref is not called.
 * The destructor always decrements the ref-count.
 * 
 * Note: gst_ptr<GstStructure> has no ref count and cannot be copied.
 */

namespace gst_helper
{
    /** Simple wrapper like shared_ptr/unique_ptr. If the gstreamer object supports ref-counts the gst_ptr is copyable, otherwise it is only moveable
     * You can create instances via the make* functions.
     */
    template<class T> class gst_ptr;

    /** The default way to create a gst_ptr.
     * Wraps the passed in pointer and 'consumes' the gobject floating flag.
     * Note: this calls make_consume_ptr
     */
    template<class T> gst_ptr<T> make_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and increments the reference count of the passed in object.
     * If the object is floating, the floating flag is removed (via g_object_ref_sink and no separate addref is called).
     */
    template<class T> gst_ptr<T> make_addref_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and 'consumes' the gobject floating flag.
     * If the object is floating, the floating flag is removed,
     * If the object is not floating its reference count is not touched.
     */
    template<class T> gst_ptr<T> make_consume_ptr( T* ptr ) noexcept;
    /** Wraps the passed in pointer and does not change the ref-count or floating flag.
     * Note: The destructor does still run and decreases the ref-count
     */
    template<class T> gst_ptr<T> make_wrap_ptr( T* ptr ) noexcept;

namespace detail
{
inline void unref_object(GstCaps* ptr) noexcept
{
    gst_caps_unref(ptr);
}
inline void unref_object(GstPad* ptr) noexcept
{
    gst_object_unref(ptr);
}
inline void unref_object(GstElement* ptr) noexcept
{
    gst_object_unref(ptr);
}
inline void unref_object(GstPlugin* ptr) noexcept
{
    gst_object_unref(ptr);
}
inline void unref_object(GstBufferPool* ptr) noexcept
{
    gst_object_unref(ptr);
}

inline void ref_object(GstCaps* ptr) noexcept
{
    gst_caps_ref(ptr);
}
inline void ref_object(GstPad* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void ref_object(GstElement* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void ref_object(GstPlugin* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void ref_object(GstBufferPool* ptr) noexcept
{
    gst_object_ref(ptr);
}

inline void ref_object(GstDevice* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void unref_object(GstDevice* ptr) noexcept
{
    gst_object_unref(ptr);
}

inline void ref_object(GstDeviceMonitor* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void unref_object(GstDeviceMonitor* ptr) noexcept
{
    gst_object_unref(ptr);
}

inline void ref_object(GstElementFactory* ptr) noexcept
{
    gst_object_ref(ptr);
}
inline void unref_object( GstElementFactory* ptr) noexcept
{
    gst_object_unref(ptr);
}

template<class T>
inline bool consume_floating( T* ptr ) noexcept
{
    if( g_object_is_floating( ptr ) )
    {
        gst_object_ref_sink( ptr );
        return true;
    }
    return false;
}
template<>
inline bool consume_floating<GstCaps>( GstCaps* /*ptr*/ ) noexcept 
{
    return false;
}
} // namespace detail

template<class T> class gst_ptr
{
public:
    gst_ptr() = default;

    gst_ptr(std::nullptr_t) noexcept {}

    gst_ptr(const gst_ptr& op2) noexcept : ptr_ { ref(op2.ptr_) } {}
    gst_ptr(gst_ptr&& op2) noexcept : ptr_ { std::exchange(op2.ptr_, nullptr) } {}
    gst_ptr& operator=(const gst_ptr& op2) noexcept
    {
        unref(ptr_);
        ptr_ = ref(op2.ptr_);
        return *this;
    }
    gst_ptr& operator=(gst_ptr&& op2) noexcept
    {
        unref(ptr_);
        ptr_ = std::exchange(op2.ptr_, nullptr);
        return *this;
    }
    ~gst_ptr()
    {
        unref( ptr_ );
    }

    /** Wraps the passed in pointer and does not change the ref-count or floating flag.
     * Note: The destructor does still run and decreases the ref-count
     */
    static gst_ptr wrap(T* ptr) noexcept
    {
        return gst_ptr { ptr };
    }
    /** Wraps the passed in pointer and 'consumes' the gobject floating flag.
     * If the object is floating, the floating flag is removed,
     * If the object is not floating its reference count is not touched.
     */
    static gst_ptr consume(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gst_ptr {};
        }
        detail::consume_floating( ptr );
        return gst_ptr { ptr };
    }
    /** Wraps the passed in pointer and increments the reference count of the passed in object.
     * If the object is floating, the floating flag is removed. Address is callled in every case.
     */
    static gst_ptr addref(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gst_ptr {};
        }
        detail::consume_floating( ptr );
        ref(ptr);
        return gst_ptr { ptr };
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
        unref(ptr_);
    }
    /**
     * Increments the reference counter.
     */
    void    add_ref() noexcept {
        ref( ptr_ );
    }
    /**
     * Decrements the reference counter. N
     * This does _not_ set the internal pointer to nullptr.
     * Note: decrement_ref only decrements the reference counter, reset also sets the internal pointer to nullptr
     */
    void    decrement_ref() noexcept {
        if( !ptr_ ) return;
        
        using namespace detail;
        unref_object( ptr_ );
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
    gst_ptr(T* ptr) noexcept : ptr_ { ptr } {}

    T* ptr_ = nullptr;

    static void unref(T*& ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return;
        }
        using namespace detail;
        unref_object(std::exchange( ptr, nullptr ));
    }
    static T* ref(T* ptr) noexcept
    {
        if (ptr == nullptr) {
            return nullptr;
        }

        using namespace detail;
        ref_object(ptr);
        return ptr;
    }
};

template<class T> bool operator==(std::nullptr_t, const gst_ptr<T>& ptr) noexcept
{
    return ptr.empty();
}
template<class T> bool operator==(const gst_ptr<T>& ptr, std::nullptr_t) noexcept
{
    return ptr.empty();
}

template<class T> bool operator!=(std::nullptr_t, const gst_ptr<T>& ptr) noexcept
{
    return !ptr.empty();
}
template<class T> bool operator!=(const gst_ptr<T>& ptr, std::nullptr_t) noexcept
{
    return !ptr.empty();
}

template<class T> gst_ptr<T> make_ptr(T* ptr) noexcept
{
    return gst_ptr<T>::consume(ptr);
}
template<class T> gst_ptr<T> make_addref_ptr(T* ptr) noexcept
{
    return gst_ptr<T>::addref(ptr);
}
template<class T> gst_ptr<T> make_consume_ptr(T* ptr) noexcept
{
    return gst_ptr<T>::consume(ptr);
}
template<class T> gst_ptr<T> make_wrap_ptr(T* ptr) noexcept
{
    return gst_ptr<T>::wrap(ptr);
}

template<> class gst_ptr<GstStructure>
{
    using T = GstStructure;
public:
    gst_ptr() = default;

    gst_ptr( std::nullptr_t ) noexcept {}

    gst_ptr( const gst_ptr& op2 ) = delete;
    gst_ptr& operator=( const gst_ptr& op2 ) = delete;

    gst_ptr( gst_ptr&& op2 ) noexcept : ptr_{ std::exchange( op2.ptr_, nullptr ) } {}
    gst_ptr& operator=( gst_ptr&& op2 ) noexcept
    {
        unref( ptr_ );
        ptr_ = std::exchange( op2.ptr_, nullptr );
        return *this;
    }
    ~gst_ptr()
    {
        unref( std::exchange( ptr_, nullptr ) );
    }

    /**
     * Wrap the passed in pointer. So do not increase the reference count.
     */
    static gst_ptr wrap( T* ptr ) noexcept
    {
        return gst_ptr{ ptr };
    }
    /**
     * Consume the passed in reference. This means that if the pointer is floating, we remove this floating flag.
     * The reference counter is not increased.
     */
    static gst_ptr consume( T* ptr ) noexcept
    {
        return gst_ptr{ ptr };
    }

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

    void reset() noexcept
    {
        unref( std::exchange( ptr_, nullptr ) );
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
    gst_ptr( T* ptr ) noexcept : ptr_{ ptr } {}

    T* ptr_ = nullptr;

    static void unref( T* ptr ) noexcept
    {
        if( ptr == nullptr )
        {
            return;
        }
        gst_structure_free( ptr );
    }
};

} // namespace gst_helper
