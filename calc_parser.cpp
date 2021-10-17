#include "calc_parser.hpp"
#include "calc_parse_error.hpp"
#include "calc_args.hpp"
#include <algorithm>
#include <cmath>

calc_parser::identifier_with_unary_fn calc_parser::unary_fn_table[] = {
    {"exp", calc_val::complex_common::exp}, // exp(n) is e raised to the power of n
    {"ln", calc_val::complex_common::log}, // natural (base e) log
    {"log10", calc_val::complex_common::log10}, // base 10 log
    {"log2", calc_val::log2}, // base 2 log
    {"sqrt", calc_val::complex_common::sqrt},
    {"cbrt", calc_val::cbrt}, // cubic root
    {"sin", calc_val::complex_common::sin},
    {"cos", calc_val::complex_common::cos},
    {"tan", calc_val::complex_common::tan},
    {"asin", calc_val::complex_common::asin}, // arc sin
    {"acos", calc_val::complex_common::acos}, // arc cos
    {"atan", calc_val::complex_common::atan}, // arc tan
    {"sinh", calc_val::complex_common::sinh}, // hyperbolic sin
    {"cosh", calc_val::complex_common::cosh}, // hyperbolic cos
    {"tanh", calc_val::complex_common::tanh}, // hyperbolic tan
    {"asinh", calc_val::complex_common::asinh}, // inverse hyperbolic sin
    {"acosh", calc_val::complex_common::acosh}, // inverse hyperbolic cos
    {"atanh", calc_val::complex_common::atanh}, // inverse hyperbolic tan
    {"gamma", calc_val::tgamma},
    {"lgamma", calc_val::lgamma}, // log gamma
    {"arg", calc_val::arg_wrapper}, // phase angle
    {"norm", calc_val::norm_wrapper}, // squared magnitude
    {"conj", calc_val::complex_common::conj}, // conjugate
    {"proj", calc_val::complex_common::proj}, // projection onto the Riemann sphere
};

calc_parser::identifier_with_internal_val calc_parser::internal_vals[] = {
    {"pi", &calc_parser::pi},
    {"e", &calc_parser::e},
    {"i", &calc_parser::i},
    {"last", &calc_parser::last},
};



template <class T>
static inline constexpr auto is_nan(T x) -> bool
{return x != x;} // nan is a special creature that nerver tests equal to itself



inline auto calc_parser::trim_if_int(calc_val::complex_type x) const -> calc_val::complex_type
{return x;}

inline auto calc_parser::trim_if_int(calc_val::float_type x) const -> calc_val::float_type
{return x;}

inline auto calc_parser::trim_if_int(calc_val::uint_type x) const -> calc_val::uint_type {
    static_assert(sizeof(std::int8_t) == 1);
    assert(sizeof(x) * 8 >= int_word_size);
    unsigned shift = sizeof(x) * 8 - int_word_size;
    return x & (~calc_val::uint_type(0) >> shift);
}

inline auto calc_parser::trim_if_int(calc_val::int_type x) const -> calc_val::int_type  {
    static_assert(sizeof(std::int8_t) == 1);
    assert(sizeof(x) * 8 >= int_word_size);
    unsigned shift = sizeof(x) * 8 - int_word_size;
    return (x << shift) >> shift; // preserves sign bits for negative number
}

inline auto calc_parser::trim_int(calc_val::variant_type& val) const -> void {
    std::visit([&](const auto& x) {
        using VT = std::decay_t<decltype(x)>;
        if constexpr (std::is_integral_v<VT>)
            val = trim_if_int(x);
    }, val);
}



auto calc_parser::evaluate(std::string_view input, help_callback help, calc_val::radices& output_radix)
        -> calc_val::variant_type {
    auto lexer = lookahead_calc_lexer(input, default_number_radix);

    // <input> ::= "help"
    //           | [ <option> ]... [ <math_expr> ]

    if (lexer.peek_token().id == calc_token::help && lexer.peek_token2().id == calc_token::end) {
        help();
        throw no_mathematical_expression();
    }

    if (lexer.peek_token().id == calc_token::option) {
        calc_args args;
        do {
            lexer.get_token();
            interpret_arg(lexer.last_token().view, expression_option_code, args);
            if (args.n_other_args)
                throw calc_parse_error(calc_parse_error::invalid_option, lexer.last_token());
            if (args.n_default_options > 1 || args.n_output_options > 1
                    || args.n_int_word_size_options > 1)
                throw calc_parse_error(calc_parse_error::too_many_options, lexer.last_token());
        } while (lexer.peek_token().id == calc_token::option);

        if (args.n_help_options)
            help();
        if (args.n_default_options) {
            default_number_type_code = args.default_number_type_code;
            default_number_radix = args.default_number_radix;
            lexer.default_number_radix(args.default_number_radix);
        }
        if (args.n_output_options)
            output_radix = args.output_radix;
        if (args.n_int_word_size_options)
            int_word_size = args.int_word_size;
    }

    if (lexer.peek_token().id == calc_token::end)
        throw no_mathematical_expression();

    last_val = math_expr(lexer);

    if (lexer.peek_token().id == calc_token::option)
        throw calc_parse_error(calc_parse_error::option_must_preface_math_expr, lexer.peeked_token());
    if (lexer.get_token().id != calc_token::end)
        throw calc_parse_error(calc_parse_error::syntax_error, lexer.last_token());

    return last_val;
}

auto calc_parser::math_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <math_expr> ::= <bxor_expr> [ "|" <bxor_expr> ]...
    auto lval = bxor_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::bor) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>)
                    return lval | rval;
                else if constexpr (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, bxor_expr(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::bxor_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <bxor_expr> ::= <band_expr> [ "^" <band_expr> ]...
    auto lval = band_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::bxor) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>)
                    return lval ^ rval;
                else if constexpr (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, band_expr(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::band_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <band_expr> ::= <shift_expr> [ "&" <shift_expr> ]...
    auto lval = shift_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::band) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>)
                    return lval & rval;
                else if constexpr (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, shift_expr(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::shift_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <shift_expr> ::= <additive_expr> [ ( "<<" | ">>" ) <additive_expr> ]...
    auto shift_arg_in_range = [&](const auto& shift_arg, const calc_token& op_token) -> auto {
    // assume shift_arg is valid only if positive and less than int_word_size.
    // if shift_arg is negative then it's unusable; parse_error will be thrown
    // in that case. if shift_arg is >= int_word_size then we will simulate
    // shifting beyond that limit
        using ShiftT = std::decay_t<decltype(shift_arg)>;
        if constexpr (std::is_integral_v<ShiftT> && std::is_signed_v<ShiftT>) {
            if (shift_arg < 0)
                throw calc_parse_error(calc_parse_error::negative_shift_invalid, op_token);
            return shift_arg < int_word_size;
        } else if constexpr (std::is_integral_v<ShiftT>)
            return shift_arg < int_word_size;
    };

    auto lval = additive_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::shiftl) {
            calc_token op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>) {
                    if (shift_arg_in_range(rval, op_token))
                        return trim_if_int(lval << rval);
                    return static_cast<LVT>(0);
                } else if (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, additive_expr(lexer));
        } else if (lexer.peeked_token().id == calc_token::shiftr) {
            calc_token op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_signed_v<LVT> && std::is_integral_v<RVT>) {
                    if (shift_arg_in_range(rval, op_token))
                        return lval >> rval;
                    else if (lval < 0)
                        return static_cast<LVT>(-1); // -1 doesn't need to be trimmed -- sign extended value
                    else
                        return static_cast<LVT>(0);
                } else if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>) {
                    calc_val::uint_type shift_arg = 0;
                    if (shift_arg_in_range(rval, op_token))
                        return lval >> shift_arg;
                    return static_cast<LVT>(0);
                } else if (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, additive_expr(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::additive_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <additive_expr> ::= <term> [ ( "+" | "-" ) <term> ]...
    auto lval = term(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::add) {
            lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                return trim_if_int(lval + rval); // trim incase of overflow
            }, lval, term(lexer));
        } else if (lexer.peeked_token().id == calc_token::sub) {
            lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                return trim_if_int(lval - rval); // trim incase of underflow
            }, lval, term(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::term(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <term> ::= <factor> [ ( "*" | "/" | "%" ) <factor> ]...
    auto lval = factor(lexer);
    for (;;) {
        if (lexer.peek_token().id == calc_token::mul) {
            lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                return trim_if_int(lval * rval); // trim incase of overflow
            }, lval, factor(lexer));
        } else if (lexer.peeked_token().id == calc_token::div) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>) {
                    if (rval == 0)
                        throw calc_parse_error(calc_parse_error::integer_division_by_0, op_token);
                }
                return trim_if_int(lval / rval); // note: −32768 / −1 overflows 16 bit int, thus need to trim
            }, lval, factor(lexer));
        } else if (lexer.peeked_token().id == calc_token::mod) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (std::is_integral_v<LVT> && std::is_integral_v<RVT>) {
                    if (rval == 0)
                        throw calc_parse_error(calc_parse_error::integer_division_by_0, op_token);
                    return trim_if_int(lval % rval); // trim for good measure
                } else if constexpr (!std::is_integral_v<LVT>)
                    throw calc_parse_error(calc_parse_error::left_operand_must_be_int_type, op_token);
                else
                    throw calc_parse_error(calc_parse_error::right_operand_must_be_int_type, op_token);
                    // we don't simply convert floating point to int because the
                    // number may be imprecise rounded version of the original
            }, lval, factor(lexer));
        } else
            break;
    }
    return lval;
}

auto calc_parser::factor(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <factor> ::= "-" <number> ( <any_token> - ( <factorial op> | "**" ) )
//            | ( "-" | "+" | "~" ) <factor>
//            | <base> [ <factorial operator> ]... [ "**" <factor> ]
// <factorial op> ::= "!" | "!!" | <mfac>
// note: exponentiation is evaluated right-to-left
    if (lexer.peek_token().id == calc_token::sub) { // "-'
        lexer.get_token();

        // special case: "-" <number> ( <any_token> - ( <factorial op> | "**" ) )
        // this is needed to properly negate and range check the number
        if (lexer.peek_token().id == calc_token::number &&
                lexer.peek_token2().id != calc_token::fac &&
                lexer.peek_token2().id != calc_token::dfac &&
                lexer.peek_token2().id != calc_token::mfac &&
                lexer.peek_token2().id != calc_token::pow)
            return assumed_number(lexer, true);

        return std::visit([&](const auto& val) -> calc_val::variant_type {
            assert(is_nan(val) || val == trim_if_int(val));
            return trim_if_int(-val);
        }, factor(lexer));
    }

    if (lexer.peek_token().id == calc_token::add) { // "+" -- just return <factor>
        lexer.get_token();
        return factor(lexer);
    }

    if (lexer.peek_token().id == calc_token::bnot) { // "~"
        auto op_token = lexer.get_token();
        return std::visit([&](const auto& val) -> calc_val::variant_type {
            using VT = std::decay_t<decltype(val)>;
            assert(is_nan(val) || val == trim_if_int(val));
            if constexpr (std::is_integral_v<VT>)
                return trim_if_int(~val);
            else
                throw calc_parse_error(calc_parse_error::operand_must_be_int_type, op_token);
                // we don't simply convert floating point to int because the
                // number may be imprecise rounded version of the original
        }, factor(lexer));
    }

    // <base>

    auto lval = base(lexer);

    // [ <factorial op> ]...

    for (;;) {
        if (lexer.peek_token().id == calc_token::fac) {
            lexer.get_token();
            lval = std::visit([](const auto& val) -> calc_val::variant_type {
                return calc_val::tgamma(val + 1);
            }, lval);
        } else if (lexer.peeked_token().id == calc_token::dfac) {
            lexer.get_token();
            lval = std::visit([](const auto& val) -> calc_val::complex_type {
                return calc_val::dfac(val);
            }, lval);
        } else if (lexer.peeked_token().id == calc_token::mfac)
            throw calc_parse_error(calc_parse_error::mfac_unsupported, lexer.get_token());
        else
            break;
    }

    // [ "**" <factor> ]

    if (lexer.peek_token().id == calc_token::pow) {
        lexer.get_token();
        lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
            assert(is_nan(lval) || lval == trim_if_int(lval));
            assert(is_nan(rval) || rval == trim_if_int(rval));
            return trim_if_int(calc_val::pow(lval, rval));
        }, lval, factor(lexer));
    }

    return lval;
}

auto calc_parser::base(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <base> ::= <number> | <identifier_expr> | <group> | <help>
    if (lexer.peek_token().id == calc_token::number)
        return assumed_number(lexer, false);
    if (lexer.peeked_token().id == calc_token::identifier)
        return assumed_identifier_expr(lexer);
    if (lexer.peeked_token().id == calc_token::lparen)
        return assumed_group(lexer);
    if (lexer.peeked_token().id == calc_token::help)
        throw calc_parse_error(calc_parse_error::help_invalid_here, lexer.peeked_token());
    if (lexer.peeked_token().id == calc_token::end)
        throw calc_parse_error(calc_parse_error::unexpected_end_of_input, lexer.peeked_token());
    throw calc_parse_error(calc_parse_error::syntax_error, lexer.peeked_token());
}

auto calc_parser::assumed_identifier_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <identifier_expr> ::= <variable> [ = <math_expr> ]
//                     | <unary_fn>
//                     | <internal_value>
//                     | <undefined_identifier>
// <unary_fn> ::= <unary_fn_identifier> <group>
// <undefined_identifier> ::= <identifier> - ( <variable> | <unary_fn_identifier> | <internal_value> )
    auto val = [&]() -> calc_val::variant_type {
        auto identifier_token = lexer.get_token(); // assume next token is identifier (caller assures this)
        assert(identifier_token.id = calc_token::identifier);
        auto identifier = identifier_token.view;

        // <variable> [ = <math_expr> ]
        for (auto& variable : variables)
            if (identifiers_match(identifier, variable.identifier)) {
                if (lexer.peek_token().id == calc_token::eq) {
                    lexer.get_token(); // eq
                    variable.val = math_expr(lexer);
                }
                return variable.val;
            }
        if (lexer.peek_token().id == calc_token::eq) {
            lexer.get_token(); // eq
            return variables.emplace_back(identifier_with_val{std::string(identifier),
                math_expr(lexer)}).val;
        }

        // <unary_fn>
        for (auto& unary_fn : unary_fn_table)
            if (identifiers_match(identifier, unary_fn.identifier)) { // <unary_fn_identifier>
                if (lexer.peek_token().id == calc_token::lparen)
                    return std::visit([&](const auto& val) -> calc_val::variant_type {
                        return unary_fn.fn(val);
                    }, assumed_group(lexer));
                throw calc_parse_error(calc_parse_error::function_arg_expected, identifier_token);
            }

        // <internal_value>
        for (auto& internal_val : internal_vals)
            if (identifiers_match(identifier, internal_val.identifier))
                return (this->*internal_val.fn)();

        // <undefined_identifier>
        throw calc_parse_error(calc_parse_error::undefined_identifier, identifier_token);
    }();

    trim_int(val); // incase int_word_size changed
    return val;
};

auto calc_parser::assumed_group(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <group> ::= "(" <math_expr> ")"
    lexer.get_token(); // assume next token is left parenthesis (caller assures this)
    assert(lexer.last_token().id == calc_token::lparen);
    auto val = math_expr(lexer);
    if (lexer.get_token().id != calc_token::rparen)
        throw calc_parse_error(calc_parse_error::token_expected, lexer.last_token(), calc_token::rparen);
    return val;
}
