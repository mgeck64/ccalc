#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "variant_type.hpp"
#include "lookahead_calc_lexer.hpp"
#include "calc_args.hpp"
#include <map>
#include <functional>

class calc_parser {
public:
    calc_parser(calc_val::number_type_codes default_number_type_code_,
        calc_val::radices default_number_radix_, calc_val::int_word_sizes int_word_size_);

    using help_callback = std::function<void()>;
    struct void_expression {}; // exception

    auto evaluate(std::string_view input, help_callback help, output_options& out_options) -> calc_val::variant_type;
    // evaluates the input string; throws parse_error on parsing error. throws
    // void_expression if no mathematical expression was evaluated.
    // input is as specified for lookahead_calc_lexer. may update output_options

    auto options() const -> parser_options;
    auto options(const parser_options&) -> void;

private:
    calc_val::number_type_codes default_number_type_code = calc_val::complex_code;
    calc_val::radices default_number_radix = calc_val::base10;
    calc_val::int_word_sizes int_word_size = calc_val::int_bits_128;
    calc_val::variant_type last_val = calc_val::complex_type(std::numeric_limits<calc_val::float_type>::quiet_NaN());

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
    using variables_map = std::unordered_map<std::string, var_poly_type>;
    // a variables_map element may hold a single value (calc_val::variant_type)
    // or a function pointer (unary_fn). key is 'y' or 'n' (depending on whether
    // the variable is internal or user-defined) + <identifier>. use var_key or
    // var_key_ref (below) to make a key

    static auto var_key(std::string_view identifier, bool internal) -> std::string
    {std::string key = internal ? "y" : "n"; key += identifier; return key;}

    std::string var_key_; // member string to mitigate string memory allocations for lookups
    auto var_key_ref(std::string_view identifier, bool internal) -> const std::string&
    {var_key_ = internal ? 'y' : 'n'; var_key_ += identifier; return var_key_;}

    variables_map variables;
    variables_map::iterator last_val_pos = variables.end();
};

#endif // CALC_PARSER_HPP
