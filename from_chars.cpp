#include "from_chars.hpp"
#include "is_digit.h"
#include <cassert>

namespace calc_val {

static inline auto make_from_chars_result(decltype(std::from_chars_result::ptr) ptr, decltype(std::from_chars_result::ec) ec) -> std::from_chars_result {
    std::from_chars_result r;
    r.ptr = ptr;
    r.ec = ec;
    return r;
}

auto from_chars(const char* begin, const char* end, float_type& out_num, unsigned radix) -> std::from_chars_result {
// specialized variation of std::from_chars for converting a floating point
// number. some differences from std::from_chars:
// - specifically for converting to calc_val::float_type
// - does not recognize leading minus sign
// - does not have std::chars_format parameter, has radix parameter instead
// - if radix != 10 then exponent is specified with 'p'/'P' instead of ''e'/'E',
//   and exponent is a power of 2 expressed in decimal
// - 0x and 0X prefixes are not recognized in any case 
    enum class scanning {whole, fraction, exponent} scan_state = scanning::whole;
    float_type num = 0;
    float_type frac_place = 1;
    float_type exponent = 0; // float_type so overflow results in inf;
                             // float_type is presumably large enough to
                             // handle full range of exponent
    auto digits = false;
    auto exponent_digits = false;
    bool negative_exponent = false;
    auto scan_radix = radix;
    auto pos = begin;

    for (; pos < end; ++pos) {
        auto digit = digit_ord(*pos, scan_radix);
        if (digit != -1) {
            if (scan_state == scanning::whole)
                num = num * radix + digit;
            else if (scan_state == scanning::fraction) {
                frac_place /= radix;
                num += digit * frac_place;
            } else {
                assert(scan_state == scanning::exponent);
                exponent = exponent * 10 + digit;
                exponent_digits = true;
            }
            digits = true;
        } else {
            if (*pos == '.' && scan_state == scanning::whole)
                scan_state = scanning::fraction;
            else if (((*pos == 'e' || *pos == 'E') && radix == 10) ||
                     ((*pos == 'p' || *pos == 'P') && radix != 10)) {
                if (scan_state != scanning::exponent && digits) {
                    scan_state = scanning::exponent;
                    scan_radix = 10;
                    if (pos + 1 < end) {
                        if (pos[1] == '+')
                            ++pos;
                        else if (pos[1] == '-') {
                            ++pos;
                            negative_exponent = true;
                        }
                    }
                } else
                    return make_from_chars_result(pos, std::errc::invalid_argument);
            } else
                return make_from_chars_result(pos, std::errc::invalid_argument);
        }
    } // for...

    if (!digits || (scan_state == scanning::exponent && !exponent_digits))
        return make_from_chars_result(pos, std::errc::invalid_argument);
    if (negative_exponent)
        exponent = -exponent;
    if (exponent != 0) {
        decltype(exponent) base = radix == 10 ? 10 : 2;
        num *= pow(base, exponent);
    }
    out_num = num;
    return make_from_chars_result(pos, std::errc());
}

} // namespace calc_val
