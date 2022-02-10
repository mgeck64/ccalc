#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "variant_type.hpp"
#include "lookahead_calc_lexer.hpp"
#include "calc_args.hpp"
#include <map>
#include <functional>

class calc_parser {
public:
    calc_parser();
    calc_parser(calc_val::number_type_codes default_number_type_code_,
        calc_val::radices default_number_radix_, calc_val::int_word_sizes int_word_size_);

    using help_callback = std::function<void()>;
    using variables_changed_callback = std::function<void()>;
    struct void_expression {}; // exception

    auto evaluate(
        std::string_view input,
        help_callback help_fn, // assumed to have a valid target
        output_options& out_options,
        variables_changed_callback variables_changed = variables_changed_callback(),
        calc_args* p_args = 0
    ) -> calc_val::variant_type;
    // evaluates the input string; throws parse_error on parsing error; throws
    // void_expression if no mathematical expression was evaluated--this is not
    // an error.
    // input is as specified for lookahead_calc_lexer.
    // side effects:
    // out_options is not used but may be updated.
    // if p_args is not null and any options were set then *p_args will have
    // a counter set to 1 for each option that was set, and all other counters
    // will be 0. (if p_args is not null but no options were set then *p_args
    // will be unchanged)

    auto options() const -> parser_options;
    auto options(const parser_options&) -> void;

    auto last_val() const -> const calc_val::variant_type&
    {return std::get<calc_val::variant_type>(last_val_pos->second);}

private:
    using variables_map = std::map<std::string, calc_val::variant_type>;

public:
    using variables_pair = variables_map::value_type;
    using variables_itr = variables_map::const_iterator;
    auto variables_begin() const -> variables_itr {return variables.begin();}
    auto variables_end() const -> variables_itr {return variables.end();}

private:
    auto finish_construction() -> void;

    calc_val::number_type_codes default_number_type_code = calc_val::complex_code;
    calc_val::radices default_number_radix = calc_val::base10;
    calc_val::int_word_sizes int_word_size = calc_val::int_bits_128;

    // parser productions
    auto assumed_delete_expr(lookahead_calc_lexer& lexer) -> void;
    auto math_expr(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto bxor_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto band_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto shift_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto additive_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    auto term(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto factor(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto base(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto assumed_identifier_expr(lookahead_calc_lexer& lexer)-> calc_val::variant_type;
    auto group(lookahead_calc_lexer& lexer) -> calc_val::variant_type;
    std::string number_buf;
    auto assumed_number(lookahead_calc_lexer& lexer, bool is_negative) -> calc_val::variant_type;

    auto trim_if_int(const calc_val::complex_type& x) const -> calc_val::complex_type;
    auto trim_if_int(const calc_val::float_type& x) const -> calc_val::float_type;
    auto trim_if_int(const calc_val::uint_type& x) const -> calc_val::uint_type;
    auto trim_if_int(const calc_val::int_type& x) const -> calc_val::int_type;
    auto trim_int(calc_val::variant_type& val) const -> void;

    using unary_fn = calc_val::complex_type (*)(const calc_val::complex_type&);
    struct identifier_with_unary_fn {
        const char* identifier;
        unary_fn fn;
    };
    static identifier_with_unary_fn unary_fn_table[];

    using var_poly_type = std::variant<calc_val::variant_type, unary_fn>;
    using internals_map = std::map<std::string, var_poly_type>;
    // an internals_map element may hold a single value (calc_val::variant_type)
    // or a function pointer (unary_fn).

    internals_map internals;
    internals_map::iterator last_val_pos = internals.end();

    variables_map variables;

    variables_changed_callback variables_changed = variables_changed_callback();

    std::string tmp_key; // to mitigate memory allocations when looking up a key in variables or internals
};

#endif // CALC_PARSER_HPP
