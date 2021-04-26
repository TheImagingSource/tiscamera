
#pragma once

#include <type_traits>  // std::integral_constant
#include <memory>       // std::unique_ptr

namespace dutils 
{
    template <auto fn>
    using deleter_from_fn = std::integral_constant<decltype(fn), fn>;

    template <typename T, auto fn>
    using state_ptr_type = std::unique_ptr<T, deleter_from_fn<fn>>;
}

