#ifndef BASICS_HPP
#define BASICS_HPP

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/constants/constants.hpp>

namespace calc_val {

using float_type = boost::multiprecision::cpp_bin_float_50;
using uint_type = unsigned __int128; // __int128 is special GCC type
using int_type = __int128;
// note: not using boost multiprecision int type for int_type because the boost
// int type is implemented as signed magnitude but we require two's compliment

// basic assumptions
static_assert(uint_type(-1) == ~uint_type(0)); // two's compliment integer type
static_assert(int_type(-2) >> int_type(1) == int_type(-1)); // right-shift is arithmetic
static_assert(sizeof(uint_type) == sizeof(int_type));

// basic assumptions about library support for __int128; may need to compile
// with std=gnu++XX flag instead of std=c++XX
static_assert(std::is_integral_v<__int128>);
static_assert(std::is_signed_v<__int128>);
static_assert(std::numeric_limits<__int128>::is_integer);
static_assert(std::numeric_limits<__int128>::is_signed);
static_assert(std::is_integral_v<unsigned __int128>);
static_assert(std::is_unsigned_v<unsigned __int128>);
static_assert(std::numeric_limits<unsigned __int128>::is_integer);
static_assert(!std::numeric_limits<unsigned __int128>::is_signed);
static_assert(std::numeric_limits<__int128>::max() < std::numeric_limits<unsigned __int128>::max());
static_assert(std::numeric_limits<std::int64_t>::max() < std::numeric_limits<__int128>::max());
static_assert(std::numeric_limits<unsigned std::int64_t>::max() < std::numeric_limits<unsigned __int128>::max());
static_assert(std::numeric_limits<unsigned __int128>::digits10 > 0);
static_assert(std::numeric_limits<__int128>::digits10 > 0);
// static_assert(sizeof(std::uintmax_t) >= sizeof(unsigned __int128)); // this fails! need to avoid using uintmax_t
// static_assert(sizeof(std::intmax_t) >= sizeof(__int128)); // this fails! need to avoid using intmax_t
using max_uint_type = unsigned __int128;
using max_int_type = __int128;

extern const float_type pi;
extern const float_type two_pi;
extern const float_type e;

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
