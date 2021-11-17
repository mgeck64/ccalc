#include "pow_int.hpp"

namespace calc_val {

auto helper::pow_uint(max_uint_type x, max_uint_type e) -> max_uint_type {
    auto x_ = (e & 1) ? x : 1;
    while (e >>= 1) {
        x *= x;
        if (e & 1)
            x_ *= x;
    }
    return x_;
}

} // namespace calc_val
