#ifndef BASICS_HPP
#define BASICS_HPP

#include <boost/multiprecision/cpp_bin_float.hpp>

namespace calc_val {

using float_type = boost::multiprecision::cpp_bin_float_100;
using uint_type = unsigned __int128; // __int128 is special GCC type

using int_type = __int128;
// note: not using boost multiprecision int type for int_type because the boost
// int type is implemented as signed magnitude but we require two's compliment

using max_uint_type = unsigned __int128;
using max_int_type = __int128;

extern const float_type pi;
extern const float_type two_pi;
extern const float_type e;
extern const float_type nan;

enum number_type_codes {complex_code, uint_code, int_code};

enum radices {base2 = 2, base8 = 8, base10 = 10, base16 = 16};

enum int_word_sizes {
    int_bits_8 = 8, int_bits_16 = 16, int_bits_32 = 32, int_bits_64 = 64,
    int_bits_128 = 128};

// note: not using scoped enums because i want enum values to automatically be
// convertible to their underlying types and to be accessable unqualified in
// calc_val namespace



template <typename T>
constexpr auto is_float_type() -> bool
{return std::is_same_v<std::decay_t<T>, float_type>;}

template <typename T>
constexpr auto is_int_type() -> bool
{return std::is_same_v<std::decay_t<T>, uint_type> || std::is_same_v<std::decay_t<T>, int_type>;}

} // namespace calc_val

#endif // BASICS_HPP
