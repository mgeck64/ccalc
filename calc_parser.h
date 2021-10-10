#pragma once
#ifndef CALC_PARSER_H
#define CALC_PARSER_H

#include "variant.h"
#include "lookahead_calc_lexer.h"
#include <vector>
#include <functional>

class calc_parser {
public:
    calc_parser(calc_val::number_type_codes default_number_type_code_,
        calc_val::radices default_number_radix_, calc_val::int_word_sizes int_word_size_);

    using help_callback = std::function<void()>;
    struct no_mathematical_expression {}; // exception

    auto evaluate(std::string_view input, help_callback help, calc_val::radices& output_radix) -> calc_val::variant_type;
    // evaluates the input string; throws parse_error on parsing error. throws
    // no_mathematical_expression if no mathematical expression was evaluated.
    // input is as specified for lookahead_calc_lexer. may update output_radix

private:
    calc_val::number_type_codes default_number_type_code = calc_val::complex_code;
    calc_val::radices default_number_radix = calc_val::base10;
    calc_val::int_word_sizes int_word_size = calc_val::int_bits_64;
    calc_val::variant_type last_val = calc_val::complex_type(std::numeric_limits<calc_val::float_type>::quiet_NaN());

    // parser productions
    auto math_expr(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto bxor_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto band_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto shift_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto additive_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto term(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto factor(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto base(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto assumed_identifier_expr(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto assumed_group(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    std::string number_buf;
    auto assumed_number(lookahead_calc_lexer& lexer, bool is_negative) -> calc_val::variant_type;

    auto trim_if_int(calc_val::complex_type x) const -> calc_val::complex_type;
    auto trim_if_int(calc_val::float_type x) const -> calc_val::float_type;
    auto trim_if_int(calc_val::uint_type x) const -> calc_val::uint_type;
    auto trim_if_int(calc_val::int_type x) const -> calc_val::int_type;
    auto trim_int(calc_val::variant_type& val) const -> void;

    struct identifier_with_val {
        std::string identifier;
        calc_val::variant_type val;
    };

    std::vector<identifier_with_val> variables;
    // variables: simple unordered array; should be small enough that simple
    // linear search will be adequate

    using unary_fn = calc_val::complex_type (*)(const calc_val::complex_type&);
    struct identifier_with_unary_fn {
        const char* identifier;
        unary_fn fn;
    };

    static identifier_with_unary_fn unary_fn_table[];
    // unary_fn_table: simple unordered array; should be small enough that
    // simple linear search will be adequate

    using internal_val_fn = calc_val::variant_type (calc_parser::*)();
    struct identifier_with_internal_val {
        const char* identifier;
        internal_val_fn fn;
    };

    static identifier_with_internal_val internal_vals[];
    // internal_vals: simple unordered array; should be small enough that simple
    // linear search will be adequate

    // internal val functions:
    auto pi() -> calc_val::variant_type {return calc_val::complex_type(calc_val::pi, 0);}
    auto e() -> calc_val::variant_type {return calc_val::complex_type(calc_val::e, 0);}
    auto i() -> calc_val::variant_type {return calc_val::complex_type(0, 1);}
    auto last() -> calc_val::variant_type {return last_val;}

    static bool identifiers_match(std::string_view identifier1, std::string_view identifier2);
    static bool identifiers_match(std::string_view identifier1, const char* identifier2);
};

inline calc_parser::calc_parser(calc_val::number_type_codes default_number_type_code_,
        calc_val::radices default_number_radix_, calc_val::int_word_sizes int_word_size_)
    : default_number_type_code{default_number_type_code_}, default_number_radix{default_number_radix_},
        int_word_size{int_word_size_}
{}

inline bool calc_parser::identifiers_match(std::string_view identifier1, std::string_view identifier2) {
    return identifier1 == identifier2;
    // may want to support case insensitive match in the future--maybe
}

inline bool calc_parser::identifiers_match(std::string_view identifier1, const char* identifier2) {
    return identifier1 == identifier2;
    // may want to support case insensitive match in the future--maybe
}

#endif // CALC_PARSER_H
