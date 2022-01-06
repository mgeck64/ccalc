#include "complex_extras.hpp"

namespace calc_val {

auto lgamma(const complex_type& z) -> complex_type {
    if (z.imag() == 0 && z.real() >= 0)
        return lgamma(z.real());
    throw domain_positive_real_only();
}

auto tgamma(const complex_type& z) -> complex_type {
    if (z.imag() == 0)
        return tgamma(z.real());
    throw domain_real_only();
}

auto dfac(const complex_type& z) -> complex_type {
// the double factorial formula extended to complex arguments is from
// https://mathworld.wolfram.com/DoubleFactorial.html
    auto cos_pi_z = cos(cpi * z);
    return pow(2, (1 + 2 * z - cos_pi_z) / 4)
        * pow(cpi, (cos_pi_z - 1) / 4) * tgamma(1 + 0.5 * z);
}

} // namespace calc_val
