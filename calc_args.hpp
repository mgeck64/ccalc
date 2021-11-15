#ifndef CALC_ARGS_HPP
#define CALC_ARGS_HPP

// interprets command line arguments and expression options

#include "basics.hpp"
#include <string_view>
#include <limits>

enum : char {
// codes for command line and expression options, number prefix, number suffix
    base2_prefix_code = 'b', base8_prefix_code = 'o', base10_prefix_code = 'd',
    base16_prefix_code = 'x', signed_suffix_code = 's', unsigned_suffix_code = 'u',
    complex_suffix_code = 'n', expression_option_code = '@'};
// considered was having the suffix code 'i' indicate an imaginary number but
// that would cause confusion in expresssions like 2i**3 and 2i!, which would
// look like they have the implied multiplication 2*i improperly being given
// precedence over exponentation and factorial; i.e., the expressions would
// appear to be improperly evaluated as (2*i)**3 and (2*i)! respectively. thus
// complex numbers in the form a+bi are entered as a+b*i.

// note: not using scoped enums because i want these to be easily convertible to
// char or int

struct calc_args {
    unsigned n_help_options = 0;
    unsigned n_default_options = 0;
    unsigned n_output_options = 0;
    unsigned n_int_word_size_options = 0;
    unsigned n_precision_options = 0;
    unsigned n_output_fp_normalized_options = 0;
    bool     other_args = false;

    calc_val::number_type_codes default_number_type_code = calc_val::complex_code;
    calc_val::radices           default_number_radix = calc_val::base10;
    calc_val::radices           output_radix = calc_val::base10;
    calc_val::int_word_sizes    int_word_size = calc_val::int_bits_64;
    unsigned                    precision = std::numeric_limits<calc_val::float_type>::digits10;
    bool                        output_fp_normalized = false;
};

void interpret_arg(std::string_view arg_str, char option_code, calc_args& args);
// interprets arg_str and updates args

struct output_options {
    decltype(calc_args::output_radix)         output_radix = calc_val::base10;
    decltype(calc_args::precision)            precision = std::numeric_limits<calc_val::float_type>::digits10;
    decltype(calc_args::output_fp_normalized) output_fp_normalized = false;
    output_options() = default;
    output_options(const calc_args& args) :
        output_radix{args.output_radix},
        precision{args.precision},
        output_fp_normalized{args.output_fp_normalized}
        {}
};

#endif // CALC_ARGS_HPP
