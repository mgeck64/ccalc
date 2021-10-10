#include "complex_type.h"

namespace calc_val {

auto helper::pow_uint_e(const complex_type& z_in, std::uintmax_t e) -> complex_type {
    auto z = z_in;
    auto z_ = (e & 1) ? z : complex_type(1);
    while (e >>= 1) {
        z *= z;
        if (e & 1)
            z_ *= z;
    }
    return z_;
}

auto helper::pow_int_e(const complex_type& z, std::intmax_t e) -> complex_type {
    static_assert(sizeof(uintmax_t) == sizeof(intmax_t));
    return e < 0
        ? 1 / pow_uint_e(z, -std::uintmax_t(e))
        : pow_uint_e(z, std::uintmax_t(e));
}

auto pow(const complex_type& z, const complex_type& e) -> complex_type {
    auto e_int = std::intmax_t(e.real());
    if (e_int == e.real() && e.imag() == 0)
        // special case where e is an integer value. this produces less
        // rounding/precision error
        return helper::pow_int_e(z, e_int);

    return exp(e * log(z));
}

auto arg_wrapper(const complex_type& z) -> complex_type
{return arg(z);}

auto norm_wrapper(const complex_type& z) -> complex_type
{return norm(z);}

} // namespace calc_val



#ifdef USE_ALTERNATIVE_COMPLEX_TYPE
/*
this is a modified version of Michael F. Hutt's complex library. the following
notice is provided as required:

The MIT License (MIT)

Copyright (c) 1999 Michael F. Hutt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ieee_fp_parts.h" // needed for implementation of proj function

namespace calc_val {

static constexpr auto i = complex_type{0, 1};

auto proj(const complex_type& z) -> complex_type {
    auto real_parts = ieee_fp_parts<complex_type::value_type>(z.real());
    auto imag_parts = ieee_fp_parts<complex_type::value_type>(z.imag());
    if (real_parts.is_inf() || imag_parts.is_inf())
        return {std::numeric_limits<complex_type::value_type>::infinity(),
            imag_parts.is_negative() ? -0.0 : 0.0};
    return z;
}

auto sqrt(const complex_type& z) -> complex_type {
    auto sqrt_real = std::sqrt(0.5 * (abs(z) + z.real()));
    auto sqrt_imag = std::sqrt(0.5 * (abs(z) - z.real()));
    if (z.imag() >= 0)
        return {sqrt_real, sqrt_imag};
    return {sqrt_real, -sqrt_imag};
}

auto log(const complex_type& z) -> complex_type {
    if (z.real() < 0 && z.imag() == 0)
        return std::log(abs(z)) + i * pi;
    return std::log(abs(z)) + i * arg(z);
}

auto log10(const complex_type& z) -> complex_type
{return log(z) / std::log(10);}

auto exp(const complex_type& z) -> complex_type
{return std::exp(z.real()) * (std::cos(z.imag()) + i * std::sin(z.imag()));}

std::ostream& operator<<(std::ostream& stream, const complex_type& z)
// mirrors how std::complex outputs a complex_type number
{return stream << '(' << z.real() << ',' << z.imag() << ")";}

} // namespace calc_val

#endif // USE_ALTERNATIVE_COMPLEX_TYPE
