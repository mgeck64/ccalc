#ifndef FROM_CHARS_HPP
#define FROM_CHARS_HPP

#include "basics.hpp"
#include <charconv>

namespace calc_val {

auto from_chars(const char* begin, const char* end, float_type& num, unsigned radix) -> std::from_chars_result;

}

#endif // FROM_CHARS_HPP