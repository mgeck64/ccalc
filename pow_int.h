#pragma once
#ifndef POW_INT_H
#define POW_INT_H

// so, e,g., we can accurately compute pow(2, 63) for 64 bit unsigned int, which
// std::pow (with 64 bit double) can't

#include <cstdint>
#include <cmath>
#include <type_traits>

namespace calc_val {    

namespace helper { // implementation helper; not meant for public use
    auto pow_uint(std::uintmax_t x, std::uintmax_t e) -> std::uintmax_t;
}

template <typename T1, typename T2>
inline auto pow(T1 x, T2 e) -> std::enable_if_t<
    std::is_integral_v<T1> && std::is_integral_v<T2> && std::is_unsigned_v<T2>, T1>
{return helper::pow_uint(x, e);}

template <typename T1, typename T2>
inline auto pow(T1 x, T2 e) -> std::enable_if_t<
    std::is_integral_v<T1> && std::is_integral_v<T2> && std::is_signed_v<T2>, T1>
{return e < 0 ? 0 : helper::pow_uint(x, e);}

// forward to std version for mixed integer and floating point cases
// (very specific here to avoid conflict with version in complex_type.h)
template <typename T1, typename T2>
auto pow(T1 x, T2 e) -> std::enable_if_t<
    (std::is_integral_v<T1> && std::is_floating_point_v<T2>) ||
    (std::is_floating_point_v<T1> && std::is_integral_v<T2>) ||
    (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>), T1>
{return std::pow(x, e);}

} // namespace calc_val

#endif // POW_INT_H
