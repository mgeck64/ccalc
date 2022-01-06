#ifndef CALC_PARSE_ERROR_HPP
#define CALC_PARSE_ERROR_HPP

#include "calc_lexer.hpp"
#include <cstring>

class calc_parse_error {
public:
    enum error_codes {
        no_error, syntax_error, number_expected, undefined_identifier,
        token_expected, integer_number_expected, out_of_range,
        invalid_number, invalid_operand, invalid_left_operand,
        invalid_right_operand,
        negative_shift_invalid, integer_division_by_0,
        mfac_unsupported, invalid_option, too_many_options,
        option_must_preface_math_expr,
        function_arg_expected,
        unexpected_end_of_input, invalid_shift_arg,
        op_domain_positive_real_only,
        op_domain_real_only,
        help_invalid_here, internal_error};
    static constexpr auto error_txt = std::array{
        // elements correspond with error_codes enums so enum can be used as index
        "no_error", "syntax error", "number expected", "undefined identifier",
        "was expected", "integer number expected", "number is out of range",
        "invalid number", "invalid operand", "invalid left operand",
        "invalid right operand",
        "negative shift value is invalid", "integer division by 0",
        "multifactorial is unsupported", "invalid option", "too many options",
        "option must preface mathematical expression",
        "function argument enclosed in parentheses was expected",
        "unexpected end of input", "invalid shift argument",
        "operation is valid for number with positive real part only",
        "operation is valid for number with real part only",
        "help is invalid here", "internal error"};

    calc_parse_error(error_codes error, const lexer_token& token_,
        lexer_token::token_ids expected_token_id_ = lexer_token::unspecified);

    auto error_str() const -> std::string;
    auto token() const -> const lexer_token& {return token_;}

private:
    error_codes error_ = no_error;
    lexer_token token_ = lexer_token{};
    lexer_token::token_ids expected_token_id_ = lexer_token::unspecified; // valid for error == tok_expected
};

inline calc_parse_error::calc_parse_error(error_codes error, const lexer_token& token, lexer_token::token_ids expected_token_id)
// expected_token_id != token::unspecified is only valid for error == token_expected
    : error_{error}, token_{token}, expected_token_id_{expected_token_id}
{
    assert(expected_token_id_ == lexer_token::unspecified || error_ == token_expected);
}

#endif // CALC_PARSE_ERROR_HPP
