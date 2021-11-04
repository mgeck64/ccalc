// calc_val::complex inline definitions; this file is included by complex_type.h

#include <cmath>

namespace calc_val {

namespace helper { // implementation helpers; not meant for public use
    auto pow_uint_e(const complex_type& z, std::uintmax_t e) -> complex_type;
    auto pow_int_e(const complex_type& z, std::intmax_t e) -> complex_type;
}

template <typename T>
inline auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_unsigned_v<T>, complex_type>
{return helper::pow_uint_e(z, e);}

template <typename T>
inline auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_signed_v<T>, complex_type>
{return helper::pow_int_e(z, e);}

} // namespace calc_val
