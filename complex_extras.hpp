#ifndef COMPLEX_EXTRAS_HPP
#define COMPLEX_EXTRAS_HPP

// extra functionality not provided by std::complex nor the alternative complex
// number class

#include "complex_type.hpp"

namespace calc_val {

inline auto log2(const complex_type& z) -> complex_type
{return log(z) / std::log(2.0L);}

inline auto cbrt(const complex_type& z) -> complex_type
{return pow(z, 1.0 / 3.0L);}

auto tgamma(const complex_type& z) -> complex_type; // gamma function: use same name as std::tgamma
auto lgamma(const complex_type& z) -> complex_type; // log gamma

auto dfac(const complex_type& z) -> complex_type;
// double factorial; see: https://mathworld.wolfram.com/DoubleFactorial.html

} // namespace calc_val

#endif // COMPLEX_EXTRAS_HPP
