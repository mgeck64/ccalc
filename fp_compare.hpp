#ifndef FP_COMPARE_HPP
#define FP_COMPARE_HPP

#include <algorithm>
#include <limits>

namespace fp_compare {

template <typename T>
inline constexpr auto epsilon() -> T
// default epslion; this can be redefined if we ever want a different one
{return std::numeric_limits<T>::epsilon();}

namespace helper { // implementation helper, not intended for public use
    template <typename T>
    typename std::enable_if_t<std::is_floating_point_v<T>, bool>
    eq(T x, T y, decltype(x) tolerance, decltype(x) tolerance0) {
    // this is a slightly modified version of the final version of almostEqual from:
    // https://www.reidatcheson.com/floating%20point/comparison/2019/03/20/floating-point-comparison.html
        auto min_xy = std::min(x, y);
        if (min_xy == 0)
            return std::abs(x - y) <= tolerance0;
        T threshold = std::numeric_limits<T>::min(); // threshold denominator so we don't divide by zero
        return (std::abs(x - y) / std::max(threshold, min_xy)) <= tolerance;
    }
}

template <typename T>
inline bool eq(T x, T y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x == y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool lt(T x, T y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x < y && !helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool gt(T x, T y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x > y && !helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool lte(T x, T y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x <= y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool gte(T x, T y, decltype(x) tolerance = epsilon<T>(), decltype(x) tolerance0 = epsilon<T>())
{return x >= y || helper::eq(x, y, tolerance, tolerance0);}

template <typename T>
inline bool is_0(T x, T tolerance = epsilon<T>(), T tolerance0 = epsilon<T>())
{return x == 0 || helper::eq(x, T(0), tolerance, tolerance0);}

} // namespace fp_compare

#endif // FP_COMPARE_HPP
