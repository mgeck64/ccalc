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



#ifdef USE_ALTERNATIVE_COMPLEX_TYPE //------------------------------------------

#include <cassert>

namespace calc_val {

inline constexpr complex_type::complex_type(const value_type& real, const value_type& imag)
: real_{real}, imag_{imag} {}

namespace helper { // implementation helpers; not meant for public use
    static constexpr auto i = complex_type{0, 1};
    static constexpr auto neg_i = complex_type{0, -1};
}



inline auto complex_type::real() const -> value_type
{return real_;}

inline void complex_type::real(const complex_type& z)
{real_ = z.real_;}

inline void complex_type::real(const value_type& real)
{real_ = real;}

inline auto real(const complex_type& z) -> complex_type::value_type
{return z.real();}



inline auto complex_type::imag() const -> value_type
{return imag_;}

inline void complex_type::imag(const complex_type& z)
{imag_ = z.imag_;}

inline void complex_type::imag(const value_type& imag)
{imag_ = imag;}

inline auto imag(const complex_type& z) -> complex_type::value_type
{return z.imag();}



inline complex_type conj(const complex_type& z)
{return complex_type(z.real(), -z.imag());}

inline auto abs(const complex_type& z) -> complex_type::value_type
{return std::sqrt(z.real() * z.real() + z.imag() * z.imag());}

inline auto arg(const complex_type& z) -> complex_type::value_type
{return std::atan2(z.imag(), z.real());}

inline auto norm(const complex_type& z) -> complex_type::value_type
{return z.real() * z.real() + z.imag() * z.imag();}

inline auto polar(const complex_type::value_type& r, const complex_type::value_type& t) -> complex_type {
    assert(r >= 0);
    return {r * std::cos(t), r * std::sin(t)};
}



inline auto operator==(const complex_type& z1, const complex_type& z2) -> bool
{return z1.real() == z2.real() && z1.imag() == z2.imag();}

inline auto operator!=(const complex_type& z1, const complex_type& z2) -> bool
{return z1.real() != z2.real() || z1.imag() != z2.imag();}



inline auto operator+(const complex_type& z) -> complex_type
{return z;}

inline auto operator+(const complex_type& z1, const complex_type& z2) -> complex_type
{return {z1.real() + z2.real(), z1.imag() + z2.imag()};}

inline auto operator+(const complex_type& z, const complex_type::value_type& a) -> complex_type
{return {z.real() + a, z.imag()};}

inline auto operator+(const complex_type::value_type& a, const complex_type& z) -> complex_type
{return {a + z.real(), z.imag()};}



inline auto operator-(const complex_type& z) -> complex_type
{return {-z.real(), -z.imag()};}

inline auto operator-(const complex_type& z1, const complex_type& z2) -> complex_type
{return {z1.real() - z2.real(), z1.imag() - z2.imag()};}

inline auto operator-(const complex_type& z, const complex_type::value_type& a) -> complex_type
{return {z.real() - a, z.imag()};}

inline auto operator-(const complex_type::value_type& a, const complex_type& z) -> complex_type
{return {a - z.real(), -z.imag()};}



inline auto operator*(const complex_type& z1, const complex_type& z2) -> complex_type {
    return {z1.real() * z2.real() - z1.imag() * z2.imag(),
        z1.real() * z2.imag() + z1.imag() * z2.real()};
}

inline auto operator*(const complex_type& z, const complex_type::value_type& a) -> complex_type
{return {z.real() * a, z.imag() * a};}

inline auto operator*(const complex_type::value_type& a, const complex_type& z) -> complex_type
{return {a * z.real(), a * z.imag()};}



inline auto operator/(const complex_type& z, const complex_type::value_type& a) -> complex_type
{return {z.real() / a, z.imag() / a};}

inline auto operator/(const complex_type& z1, const complex_type& z2) -> complex_type 
{return (z1 * conj(z2)) / norm(z2);}

inline auto operator/(const complex_type::value_type& a, const complex_type& z) -> complex_type
{return (a * conj(z)) / norm(z);}



inline auto complex_type::operator+=(const complex_type& z) -> complex_type&
{*this = *this + z; return *this;}

inline auto complex_type::operator-=(const complex_type& z) -> complex_type&
{*this = *this - z; return *this;}

inline auto complex_type::operator*=(const complex_type& z) -> complex_type&
{*this = *this * z; return *this;}

inline auto complex_type::operator/=(const complex_type& z) -> complex_type&
{*this = *this / z; return *this;}

inline auto sin(const complex_type& z) -> complex_type
{return 0.5 * helper::neg_i * exp(helper::i * z) - 0.5 * helper::neg_i * exp(helper::neg_i * z);}

inline auto cos(const complex_type& z) -> complex_type
{return 0.5 * exp(helper::i * z) + 0.5 * exp(helper::neg_i * z);}

inline auto tan(const complex_type& z) -> complex_type
{return sin(z) / cos(z);}

inline auto sec(const complex_type& z) -> complex_type
{return 1 / cos(z);}

inline auto csc(const complex_type& z) -> complex_type
{return 1 / sin(z);}

inline auto cot(const complex_type& z) -> complex_type
{return cos(z) / sin(z);}

inline auto sinh(const complex_type& z) -> complex_type
{return (exp(z) - exp(-z)) / 2;}

inline auto cosh(const complex_type& z) -> complex_type
{return (exp(z) + exp(-z)) / 2;}

inline auto tanh(const complex_type& z) -> complex_type
{return sinh(z) / cosh(z);}

inline auto sech(const complex_type& z) -> complex_type
{return 1 / cosh(z);}

inline auto csch(const complex_type& z) -> complex_type
{return 1 / sinh(z);}

inline auto coth(const complex_type& z) -> complex_type
{return cosh(z) / sinh(z);}

inline auto asin(const complex_type& z) -> complex_type
{return helper::neg_i * log(helper::i * z + sqrt(1 - z * z));}

inline auto acos(const complex_type& z) -> complex_type
{return helper::neg_i * log(z + sqrt(z * z - 1));}

inline auto atan(const complex_type& z) -> complex_type
{return (0.5 * helper::i) * log((helper::i + z) / (helper::i - z));}

inline auto asinh(const complex_type& z) -> complex_type
{return log(z + sqrt(z * z + 1));}

inline auto acosh(const complex_type& z) -> complex_type
{return log(z + sqrt(z * z - 1));}

inline auto atanh(const complex_type& z) -> complex_type
{return 0.5 * log((1 + z) / (1 - z));}

} // namespace calc_val

#endif // USE_ALTERNATIVE_COMPLEX_TYPE
