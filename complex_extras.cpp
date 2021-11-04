#include "complex_extras.hpp"

namespace boost {namespace math {namespace constants {

BOOST_DEFINE_MATH_CONSTANT(c0, 0.99999999999980993227684700473478e+00, "0.99999999999980993227684700473478e+00")
BOOST_DEFINE_MATH_CONSTANT(c1, 676.520368121885098567009190444019e+00, "676.520368121885098567009190444019e+00")
BOOST_DEFINE_MATH_CONSTANT(c2, -1259.13921672240287047156078755283e+00, "-1259.13921672240287047156078755283e+00")
BOOST_DEFINE_MATH_CONSTANT(c3, 771.3234287776530788486528258894e+00, "771.3234287776530788486528258894e+00")
BOOST_DEFINE_MATH_CONSTANT(c4, -176.61502916214059906584551354e+00, "-176.61502916214059906584551354e+00")
BOOST_DEFINE_MATH_CONSTANT(c5, 12.507343278686904814458936853e+00, "12.507343278686904814458936853e+00")
BOOST_DEFINE_MATH_CONSTANT(c6, -0.13857109526572011689554707e+00, "-0.13857109526572011689554707e+00")
BOOST_DEFINE_MATH_CONSTANT(c7, 9.984369578019570859563e-06, "9.984369578019570859563e-06")
BOOST_DEFINE_MATH_CONSTANT(c8, 1.50563273514931155834e-07, "1.50563273514931155834e-07")

}}} // namespace boost::math::onstants

namespace calc_val {

// defined here are fairly simple implementations of log gamma (lgamma) and
// gamma (tgamma) for complex numbers. these were designed for double precision
// floating point and produce less accurate results than the library lgamma and
// tgamma functions, but those are limited to real numbers. a more elaborate
// implementation for double precision numbers can be found here:
// https://www.hipparchus.org/hipparchus-core/jacoco/org.hipparchus.special/Gamma.java.html
// but i don't know if it would be applicable/adaptable to the complex number
// class we're using

static const auto cpi = complex_type(pi);
static const auto sqrt_2_pi = complex_type(sqrt(two_pi));
static const auto ln_sqrt_2_pi = complex_type(log(sqrt_2_pi));
static const auto i = complex_type(0, 1);
static const auto neg_i = complex_type(0, -1);
static const auto one_half = complex_type(0.5, 0);

// pre-calculated coefficients from
// https://mrob.com/pub/ries/lanczos-gamma.html
static const complex_type c[] = {
    {boost::math::constants::c0<float_type>()},
    {boost::math::constants::c1<float_type>()},
    {boost::math::constants::c2<float_type>()},
    {boost::math::constants::c3<float_type>()},
    {boost::math::constants::c4<float_type>()},
    {boost::math::constants::c5<float_type>()},
    {boost::math::constants::c6<float_type>()},
    {boost::math::constants::c7<float_type>()},
    {boost::math::constants::c8<float_type>()},
};
static auto g = complex_type{7};
static auto N = sizeof(c) / sizeof(*c);

auto lgamma(const complex_type& z_in) -> complex_type {
// this is adapted from the sample code that appears at
// https://mrob.com/pub/ries/lanczos-gamma.html
// note: can't simply use log(gamma(z)) because gamma overflows for fairly small
// values
    auto z = z_in;

    if (z.real() < one_half.real()) { // use Euler's reflection formula
        if (z.real() <= 0 && z.imag() == 0 && trunc(z.real()) == z.real()) // need to treat negative whole reals specially
            return log(1 / complex_type(0)); // ln(complex infinity)
        return log(cpi / sin(cpi * z)) - lgamma(1 - z);
    }

    assert(z.real() > 0);
    if (z.imag() == 0)
        return lgamma(z.real());
        // library lgamma for reals produces better results for positive real
        // numbers

    z -= 1;

    // sum the series; start with the terms that have the smallest coefficients
    // and largest denominator
    complex_type sum = 0;
    for (auto i = N - 1; i > 0; --i)
        sum += c[i] / (z + i);
    sum += c[0];

    auto base = z + g + one_half;
    return ((ln_sqrt_2_pi + log(sum)) - base) + log(base) * (z + one_half);
}

auto tgamma(const complex_type& z_in) -> complex_type {
// this is adapted from the sample code that appears at
// https://mrob.com/pub/ries/lanczos-gamma.html
// this could alternatively be implemented as exp(lgamma(z)) but i chose to
// implement it this way for good measure
    auto z = z_in;

    if (z.real() < one_half.real()) { // use Euler's reflection formula
        if (z.real() <= 0 && z.imag() == 0 && trunc(z.real()) == z.real()) // make sure we handle negative whole reals properly
            return 1 / complex_type(0); // complex infinity
        return cpi / (sin(cpi * z) * tgamma(1 - z));
    }

    assert(z.real() > 0);
    if (z.imag() == 0)
        return tgamma(z.real());
        // library tgamma for reals produces better results for positive real
        // numbers, in particular whole numbers for factorial

    z -= 1;

    // sum the series; start with the terms that have the smallest coefficients
    // and largest denominator
    complex_type sum = 0;
    for (auto i = N - 1; i > 0; --i)
        sum += c[i] / (z + i);
    sum += c[0];

    auto base = z + g + one_half;
    return sqrt_2_pi * sum * pow(base, z + one_half) / exp(base);
}



auto dfac(const complex_type& z) -> complex_type {
// the double factorial formula extended to complex arguments is from
// https://mathworld.wolfram.com/DoubleFactorial.html
    auto cos_pi_z = cos(cpi * z);
    return pow(2, (1 + 2 * z - cos_pi_z) / 4)
        * pow(cpi, (cos_pi_z - 1) / 4) * tgamma(1 + one_half * z);
}

} // namespace calc_val
