#ifndef BASICS_HPP
#define BASICS_HPP

#include <cstdint>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/math/constants/constants.hpp>
  
namespace calc_val {

using float_type = boost::multiprecision::cpp_bin_float_50;
using uint_type = std::uint64_t;
using int_type = std::int64_t;

static_assert(uint_type(-1) == ~uint_type(0)); // two's compliment machine
static_assert(int_type(-2) >> int_type(1) == int_type(-1)); // right-shift is arithmetic

static const auto pi     = boost::math::constants::pi<float_type>();
static const auto two_pi = boost::math::constants::two_pi<float_type>();
static const auto e      = boost::math::constants::e<float_type>();

enum number_type_codes {complex_code, uint_code, int_code};

enum radices {base2 = 2, base8 = 8, base10 = 10, base16 = 16};

enum int_word_sizes {
    int_bits_8 = 8, int_bits_16 = 16, int_bits_32 = 32, int_bits_64 = 64};

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
