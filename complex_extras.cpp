#include "complex_extras.hpp"

namespace calc_val {

// defined here are fairly simple implementations of log gamma (lgamma) and
// gamma (tgamma) for complex numbers. they produce less accurate results than
// std::lgamma and std::tgamma, but those are limited to real numbers. a more
// elaborate implementation for double precision numbers can be found here:
// https://www.hipparchus.org/hipparchus-core/jacoco/org.hipparchus.special/Gamma.java.html
// but i don't know if it would be applicable/adaptable to complex numbers

static constexpr auto cpi = complex_type{pi};
static constexpr auto sqrt_2_pi = complex_type{std::sqrt(2 * pi)};
static const     auto ln_sqrt_2_pi = complex_type{std::log(std::sqrt(2 * pi))};
static constexpr auto i = complex_type{0, 1};
static constexpr auto neg_i = complex_type{0, -1};

// pre-calculated coefficients from
// https://mrob.com/pub/ries/lanczos-gamma.html#fn_13
static constexpr complex_type c[] = {
    {0.99999999999980993227684700473478L},
    {676.520368121885098567009190444019L},
    {-1259.13921672240287047156078755283L},
    {771.3234287776530788486528258894L},
    {-176.61502916214059906584551354L},
    {12.507343278686904814458936853L},
    {-0.13857109526572011689554707L},
    {9.984369578019570859563e-6L},
    {1.50563273514931155834e-7L}};
static constexpr auto g = complex_type{7};
static constexpr auto N = sizeof(c) / sizeof(*c);

auto lgamma(const complex_type& z_in) -> complex_type {
// this is adapted from the sample code that appears at
// https://mrob.com/pub/ries/lanczos-gamma.html#fn_13
// note: can't simply use log(gamma(z)) because gamma overflows for fairly small
// values
    auto z = z_in;

    if (z.real() < 0.5) { // use Euler's reflection formula
        if (z.real() <= 0 && std::trunc(z.real()) == z.real())
            return log(1 / complex_type(0)); // ln(complex infinity)
        return log(cpi / sin(cpi * z)) - lgamma(1 - z);
    }

    if (z.imag() == 0 && z.real() > 0)
        return std::lgamma(z.real());
        // std::lgamma (for real numbers only) produces better results for
        // positive real numbers

    z -= 1;

    // sum the series; start with the terms that have the smallest coefficients
    // and largest denominator
    complex_type sum = 0;
    for (auto i = N - 1; i > 0; --i)
        sum += c[i] / (z + i);
    sum += c[0];

    auto base = z + g + 0.5L;
    return ((ln_sqrt_2_pi + log(sum)) - base) + log(base) * (z + 0.5L);
}

auto tgamma(const complex_type& z_in) -> complex_type {
// this is adapted from the sample code that appears at
// https://mrob.com/pub/ries/lanczos-gamma.html#fn_13
// this could alternatively be implemented as exp(lgamma(z)) but i chose to
// implement it this way for good measure
    auto z = z_in;

    if (z.real() < 0.5) { // use Euler's reflection formula
        if (z.real() <= 0 && std::trunc(z.real()) == z.real())
            return 1 / complex_type(0); // complex infinity
        return cpi / (sin(cpi * z) * tgamma(1 - z));
    }

    if (z.imag() == 0 && z.real() > 0)
        return std::tgamma(z.real());
        // std::tgamma (for real numbers only) produces better results for
        // positive real numbers, in particular whole numbers for factorial

    z -= 1;

    // sum the series; start with the terms that have the smallest coefficients
    // and largest denominator
    complex_type sum = 0;
    for (auto i = N - 1; i > 0; --i)
        sum += c[i] / (z + i);
    sum += c[0];

    auto base = z + g + 0.5L;
    return sqrt_2_pi * sum * pow(base, z + 0.5L) / exp(base);
}



auto dfac(const complex_type& z) -> complex_type {
// the double factorial formula extended to complex arguments is from
// https://mathworld.wolfram.com/DoubleFactorial.html
    auto cos_pi_z = cos(cpi * z);
    return pow(2, (1 + 2 * z - cos_pi_z) / 4)
        * pow(cpi, (cos_pi_z - 1) / 4) * tgamma(1 + 0.5L * z);
}

} // namespace calc_val
