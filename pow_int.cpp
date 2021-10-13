#include "pow_int.hpp"

namespace calc_val {

auto helper::pow_uint(std::uintmax_t x, std::uintmax_t e) -> std::uintmax_t {
    auto x_ = (e & 1) ? x : 1;
    while (e >>= 1) {
        x *= x;
        if (e & 1)
            x_ *= x;
    }
    return x_;
}

} // namespace calc_val
