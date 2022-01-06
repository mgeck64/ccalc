#ifndef COMPLEX_EXTRAS_HPP
#define COMPLEX_EXTRAS_HPP

// extra functionality not provided by complex_type's library

#include "complex_type.hpp"

namespace calc_val {

inline auto log2(const complex_type& z) -> complex_type
{return log(z) / log(float_type(2));}

inline auto cbrt(const complex_type& z) -> complex_type
{return pow(z, float_type(1) / float_type(3));}

struct domain_positive_real_only {}; // exception
struct domain_real_only {}; // exception

auto lgamma(const complex_type& z) -> complex_type;
// log gamma.
// may throw domain_positive_real_only

auto tgamma(const complex_type& z) -> complex_type;
// gamma function: use same name as std::tgamma.
// may throw domain_real_only

auto dfac(const complex_type& z) -> complex_type;
// double factorial; see: https://mathworld.wolfram.com/DoubleFactorial.html
// may throw domain_real_only

} // namespace calc_val

#endif // COMPLEX_EXTRAS_HPP
