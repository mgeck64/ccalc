#ifndef CALC_ARGS_HPP
#define CALC_ARGS_HPP

// interprets command line arguments and expression options

#include "basics.hpp"
#include <string_view>

// codes for command line and expression options, and number prefix
static constexpr auto base2_prefix_code      = 'b';
static constexpr auto base8_prefix_code      = 'o';
static constexpr auto base10_prefix_code     = 'd';
static constexpr auto base16_prefix_code     = 'x';
static constexpr auto signed_prefix_code     = 'i';
static constexpr auto unsigned_prefix_code   = 'u';
static constexpr auto complex_prefix_code    = 'n';
static constexpr auto null_prefix_code       = '\0';
static constexpr auto expression_option_code = '@';

constexpr auto default_precision = 25;
static_assert(default_precision < std::numeric_limits<calc_val::float_type>::digits10);

struct calc_args {
    unsigned n_help_options = 0;
    unsigned n_default_options = 0;
    unsigned n_output_options = 0;
    unsigned n_int_word_size_options = 0;
    unsigned n_precision_options = 0;
    unsigned n_output_fp_normalized_options = 0;
    bool other_args = false;

    calc_val::number_type_codes default_number_type_code = calc_val::complex_code;
    calc_val::radices default_number_radix = calc_val::base10;
    calc_val::radices output_radix = calc_val::base10;
    calc_val::int_word_sizes int_word_size = calc_val::int_bits_128;
    unsigned precision = default_precision;
    bool output_fp_normalized = false;

    auto operator ==(const calc_args&) const -> bool = default;
};

void interpret_arg(std::string_view arg_str, char option_code, calc_args& args);
// interprets arg_str and updates args

struct parser_options {
    decltype(calc_args::default_number_type_code) default_number_type_code = calc_val::complex_code;
    decltype(calc_args::default_number_radix) default_number_radix = calc_val::base10;
    decltype(calc_args::int_word_size) int_word_size = calc_val::int_bits_128;

    auto operator ==(const parser_options&) const -> bool = default;
};

struct output_options {
    decltype(calc_args::output_radix) output_radix = calc_val::base10;
    decltype(calc_args::precision) precision = default_precision;
    decltype(calc_args::output_fp_normalized) output_fp_normalized = false;
 
    output_options() = default;
    output_options(const calc_args& args) :
        output_radix{args.output_radix},
        precision{args.precision},
        output_fp_normalized{args.output_fp_normalized}
        {}

    auto operator ==(const output_options&) const -> bool = default;
};

#endif // CALC_ARGS_HPP
