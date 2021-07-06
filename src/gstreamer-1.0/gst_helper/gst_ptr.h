#pragma once

#include <gst/gst.h>
#include <utility>


namespace gst_helper
{
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
//inline void    unref_object( GstStructure* ptr ) noexcept { gst_structure_free( ptr ); }

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
} // namespace detail

template<class T> class gst_ptr
{
public:
    gst_ptr() = default;

    gst_ptr(nullptr_t) noexcept {}

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

    /**
         * Wrap the passed in pointer. So do not increase the reference count.
         */
    static gst_ptr wrap(T* ptr) noexcept
    {
        return gst_ptr { ptr };
    }
    /**
         * Consume the passed in reference. This means that if the pointer is floating, we remove this floating flag.
         * The reference counter is not increased.
         */
    static gst_ptr consume(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gst_ptr {};
        }
        if (g_object_is_floating(ptr))
        {
            g_object_ref_sink(ptr);
        }
        return gst_ptr { ptr };
    }

    static gst_ptr addref(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return gst_ptr {};
        }
        if (g_object_is_floating(ptr))
        {
            g_object_ref_sink(ptr);
        }
        else
        {
            ref(ptr);
        }
        return gst_ptr { ptr };
    }

    bool empty() const noexcept
    {
        return ptr_ == nullptr;
    }
    T* release() noexcept
    {
        return std::exchange(ptr_, nullptr);
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
        unref(ptr_);
    }
    void reset(T* ptr)
    {
        unref(ptr_);
        ptr_ = ref(ptr);
    }


private:
    gst_ptr(T* ptr) noexcept : ptr_ { ptr } {}

    T* ptr_ = nullptr;

    static void unref(T* ptr) noexcept
    {
        if (ptr == nullptr)
        {
            return;
        }
        using namespace detail;
        unref_object(ptr);
    }
    static T* ref(T* ptr) noexcept
    {
        if (ptr == nullptr)
            return nullptr;

        using namespace detail;
        ref_object(ptr);
        return ptr;
    }
};

template<class T> bool operator==(nullptr_t, const gst_ptr<T>& ptr) noexcept
{
    return ptr.empty();
}
template<class T> bool operator==(const gst_ptr<T>& ptr, nullptr_t) noexcept
{
    return ptr.empty();
}

template<class T> bool operator!=(nullptr_t, const gst_ptr<T>& ptr) noexcept
{
    return !ptr.empty();
}
template<class T> bool operator!=(const gst_ptr<T>& ptr, nullptr_t) noexcept
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
} // namespace gst_helper
