#pragma once
#ifndef BASICS_H
#define BASICS_H

#include <cstdint>

namespace calc_val {

using float_type = long double;
using uint_type = std::uint64_t;
using int_type = std::int64_t;

static_assert(uint_type(-1) == ~uint_type(0)); // two's compliment machine
static_assert(int_type(-2) >> int_type(1) == int_type(-1)); // right-shift is arithmetic

// we may want to support float_type = long double someday but there are some
// considerations: calc_outputter uses ieee_double_parts class (assumes IEEE
// double format) and precision of complex_type::lgamma and complex_type::gamma
// is limited to double.

static constexpr auto pi = 3.1415926535897932384626433832795028841971693993751058209749445923L;
static constexpr auto e = 2.7182818284590452353602874713526624977572470936999595749669676277L;

enum number_type_codes : std::uint8_t {complex_code, uint_code, int_code};

enum radices : std::uint8_t {base2 = 2, base8 = 8, base10 = 10, base16 = 16};

enum int_word_sizes : std::uint8_t {
    int_bits_8 = 8, int_bits_16 = 16, int_bits_32 = 32, int_bits_64 = 64};

// note: not using scoped enums because i want enum values to automatically be
// convertible to their underlying types and to be accessable unqualified in
// calc_val namespace

} // namespace calc_val

#endif // BASICS_H
