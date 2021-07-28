
#pragma once

#include "outcome.hpp"

namespace outcome = OUTCOME_V2_NAMESPACE;

namespace OUTCOME_V2_NAMESPACE
{
    template<class T> constexpr bool failed( const result<T>& res ) noexcept { return res.has_error(); }
    template<class T> constexpr bool succeeded( const result<T>& res ) noexcept { return !failed( res ); }
}
