#include "complex_type.hpp"

namespace calc_val {

auto helper::pow_uint_e(const complex_type& z_in, max_uint_type e) -> complex_type {
    auto z = z_in;
    auto z_ = (e & 1) ? z : complex_type(1);
    while (e >>= 1) {
        z *= z;
        if (e & 1)
            z_ *= z;
    }
    return z_;
}

auto helper::pow_int_e(const complex_type& z, max_int_type e) -> complex_type {
    return e < 0
        ? 1 / pow_uint_e(z, -max_uint_type(e))
        : pow_uint_e(z, max_uint_type(e));
}

auto pow(const complex_type& z, const complex_type& e) -> complex_type {
    auto e_int = max_int_type(e.real());
    if (e_int == e.real() && e.imag() == 0)
        // special case where e is an integer value. this produces less
        // rounding/precision error
        return helper::pow_int_e(z, e_int);

    if (calc_val::e == z) // special case for e constant; gives more accurate result
        return exp(e);

    return boost::multiprecision::pow(z, e); // need qualified name here else get seg fault
}

auto arg_wrapper(const complex_type& z) -> complex_type
{return arg(z);}

auto norm_wrapper(const complex_type& z) -> complex_type
{return norm(z);}

} // namespace calc_val
