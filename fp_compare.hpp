#ifndef FP_COMPARE_HPP
#define FP_COMPARE_HPP

#include "basics.hpp"

namespace fp_compare {

template <typename T>
inline constexpr auto epsilon() -> T
// default epslion; this can be redefined if we ever want a different one
{return std::numeric_limits<T>::epsilon();}

namespace helper { // implementation helper, not intended for public use
    template <typename T, typename U>
    typename std::enable_if_t<calc_val::is_float_type<T>(), bool>
    eq(const T& x, const U& y, decltype(x) tolerance, decltype(x) tolerance0) {
    // this is a slightly modified version of the final version of almostEqual from:
    // https://www.reidatcheson.com/floating%20point/comparison/2019/03/20/floating-point-comparison.html
        auto abs_x = abs(x);
        auto abs_y = abs(static_cast<T>(y));
        auto min_xy = abs_x <= abs_y ? abs_x : abs_y;
        if (min_xy == 0)
            return abs(x - y) <= tolerance0;
        return (abs(x - y) / min_xy) <= tolerance;
    }
}

template <typename T, typename U> // U is assumed to be T or convertible to T
inline bool eq(const T& x, const U& y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x == y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T, typename U> // U is assumed to be T or convertible to T
inline bool lt(const T& x, const U& y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x < y && !helper::eq(x, y, tolerance, tolerance0);}

template <typename T, typename U> // U is assumed to be T or convertible to T
inline bool gt(const T& x, const U& y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x > y && !helper::eq(x, y, tolerance, tolerance0);}

template <typename T, typename U> // U is assumed to be T or convertible to T
inline bool lte(const T& x, const U& y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x <= y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T, typename U> // U is assumed to be T or convertible to T
inline bool gte(const T& x, const U& y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x >= y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool eq0(const T& x, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x == 0 || helper::eq(x, 0, tolerance, tolerance0);}

} // namespace fp_compare

#endif // FP_COMPARE_HPP
