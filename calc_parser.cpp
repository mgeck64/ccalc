#include "calc_parser.hpp"
#include "calc_parse_error.hpp"
#include <algorithm>
#include <cmath>

calc_parser::identifier_with_unary_fn calc_parser::unary_fn_table[] = {
    {"exp", boost::multiprecision::exp}, // exp(n) is e raised to the power of n
    {"ln", boost::multiprecision::log}, // natural (base e) log
    {"log10", boost::multiprecision::log10}, // base 10 log
    {"log2", calc_val::log2}, // base 2 log
    {"sqrt", boost::multiprecision::sqrt},
    {"cbrt", calc_val::cbrt}, // cubic root
    {"sin", boost::multiprecision::sin},
    {"cos", boost::multiprecision::cos},
    {"tan", boost::multiprecision::tan},
    {"asin", boost::multiprecision::asin}, // arc sin
    {"acos", boost::multiprecision::acos}, // arc cos
    {"atan", boost::multiprecision::atan}, // arc tan
    {"sinh", boost::multiprecision::sinh}, // hyperbolic sin
    {"cosh", boost::multiprecision::cosh}, // hyperbolic cos
    {"tanh", boost::multiprecision::tanh}, // hyperbolic tan
    {"asinh", boost::multiprecision::asinh}, // inverse hyperbolic sin
    {"acosh", boost::multiprecision::acosh}, // inverse hyperbolic cos
    {"atanh", boost::multiprecision::atanh}, // inverse hyperbolic tan
    {"gamma", calc_val::tgamma},
    {"lgamma", calc_val::lgamma}, // log gamma
    {"arg", calc_val::arg_wrapper}, // phase angle
    {"norm", calc_val::norm_wrapper}, // squared magnitude
    {"conj", boost::multiprecision::conj}, // conjugate
    {"proj", boost::multiprecision::proj}, // projection onto the Riemann sphere
};



template <class T>
static inline constexpr auto is_nan(T x) -> bool
{return x != x;} // nan is a special creature that nerver tests equal to itself



inline auto calc_parser::trim_if_int(const calc_val::complex_type& x) const -> calc_val::complex_type
{return x;}

inline auto calc_parser::trim_if_int(const calc_val::float_type& x) const -> calc_val::float_type
{return x;}

inline auto calc_parser::trim_if_int(const calc_val::uint_type& x) const -> calc_val::uint_type {
    static_assert(sizeof(std::int8_t) == 1);
    assert(sizeof(x) * 8 >= int_word_size);
    unsigned shift = sizeof(x) * 8 - int_word_size;
    return x & (~calc_val::uint_type(0) >> shift);
}

inline auto calc_parser::trim_if_int(const calc_val::int_type& x) const -> calc_val::int_type  {
    static_assert(sizeof(std::int8_t) == 1);
    assert(sizeof(x) * 8 >= int_word_size);
    unsigned shift = sizeof(x) * 8 - int_word_size;
    return (x << shift) >> shift; // preserves sign bits for negative number
}

inline auto calc_parser::trim_int(calc_val::variant_type& val) const -> void {
    std::visit([&](const auto& x) {
        if constexpr (calc_val::is_int_type<decltype(x)>())
            val = trim_if_int(x);
    }, val);
}



static auto try_to_make_int_if_complex(calc_val::variant_type& val) -> void {
    std::visit([&](const auto& sub_val) {
        if constexpr (calc_val::is_complex_type<decltype(sub_val)>()) {
            if (sub_val.imag() == 0) {
                auto i = static_cast<calc_val::int_type>(sub_val.real());
                if (i == sub_val.real()) // sub_val is a whole number that fits in int
                    val = i; // side effect; assume sub_val becomes invalid
            }
        }
    }, val);
}



calc_parser::calc_parser(
    calc_val::number_type_codes default_number_type_code_,
    calc_val::radices default_number_radix_,
    calc_val::int_word_sizes int_word_size_)
:
    default_number_type_code{default_number_type_code_},
    default_number_radix{default_number_radix_},
    int_word_size{int_word_size_}
{
    for (auto& elem: unary_fn_table)
        variables.emplace(tmp_str = elem.identifier, elem.fn);

    // predefined variables
    variables.emplace(tmp_str = "pi", calc_val::pi);
    variables.emplace(tmp_str = "e", calc_val::e);
    variables.emplace(tmp_str = "i", calc_val::complex_type(0, 1));
}

auto calc_parser::evaluate(std::string_view input, help_callback help, output_options& out_options)
        -> calc_val::variant_type {
    auto lexer = lookahead_calc_lexer(input, default_number_radix);

    // <input> ::= "help"
    //           | [ <option> ]... [ <math_expr> ]

    if (lexer.peek_token().id == lexer_token::help && lexer.peek_token2().id == lexer_token::end) {
        help();
        throw no_mathematical_expression();
    }

    if (lexer.peek_token().id == lexer_token::option) {
        calc_args args;
        do {
            lexer.get_token();
            interpret_arg(lexer.last_token().view, expression_option_code, args);
            if (args.other_args)
                throw calc_parse_error(calc_parse_error::invalid_option, lexer.last_token());
            if (   args.n_default_options > 1
                || args.n_output_options > 1
                || args.n_int_word_size_options > 1
                || args.n_precision_options > 1
                || args.n_output_fp_normalized_options > 1
            )
                throw calc_parse_error(calc_parse_error::too_many_options, lexer.last_token());
        } while (lexer.peek_token().id == lexer_token::option);

        if (args.n_help_options)
            help();
        if (args.n_default_options) {
            default_number_type_code = args.default_number_type_code;
            default_number_radix = args.default_number_radix;
            lexer.default_number_radix(args.default_number_radix);
        }
        if (args.n_output_options)
            out_options.output_radix = args.output_radix;
        if (args.n_int_word_size_options)
            int_word_size = args.int_word_size;
        if (args.n_precision_options)
            out_options.precision = args.precision;
        if (args.n_output_fp_normalized_options)
            out_options.output_fp_normalized = args.output_fp_normalized;
    }

    if (lexer.peek_token().id == lexer_token::end)
        throw no_mathematical_expression();

    auto val = math_expr(lexer);

    if (lexer.peek_token().id == lexer_token::option)
        throw calc_parse_error(calc_parse_error::option_must_preface_math_expr, lexer.peeked_token());
    if (lexer.get_token().id != lexer_token::end)
        throw calc_parse_error(calc_parse_error::syntax_error, lexer.last_token());

    variables.insert_or_assign(tmp_str = "last", val);
    return val;
}

auto calc_parser::math_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <math_expr> ::= <bxor_expr> [ "|" <bxor_expr> ]...
    auto lval = bxor_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == lexer_token::bor) {
            auto op_token = lexer.get_token();
            auto rval = bxor_expr(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (calc_val::is_int_type<decltype(lval)>() && calc_val::is_int_type<decltype(rval)>())
                    return lval | rval;
                else if constexpr (!calc_val::is_int_type<decltype(lval)>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else
            break;
    }
    return lval;
}

auto calc_parser::bxor_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <bxor_expr> ::= <band_expr> [ "^" <band_expr> ]...
    auto lval = band_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == lexer_token::bxor) {
            auto op_token = lexer.get_token();
            auto rval = band_expr(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (calc_val::is_int_type<decltype(lval)>() && calc_val::is_int_type<decltype(rval)>())
                    return lval ^ rval;
                else if constexpr (!calc_val::is_int_type<decltype(lval)>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else
            break;
    }
    return lval;
}

auto calc_parser::band_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <band_expr> ::= <shift_expr> [ "&" <shift_expr> ]...
    auto lval = shift_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == lexer_token::band) {
            auto op_token = lexer.get_token();
            auto rval = shift_expr(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (calc_val::is_int_type<decltype(lval)>() && calc_val::is_int_type<decltype(rval)>())
                    return lval & rval;
                else if constexpr (!calc_val::is_int_type<decltype(lval)>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else
            break;
    }
    return lval;
}

auto calc_parser::shift_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <shift_expr> ::= <additive_expr> [ ( "<<" | ">>" ) <additive_expr> ]...
    auto shift_arg_in_range = [&](const auto& shift_arg, const lexer_token& op_token) -> auto {
    // assume shift_arg is valid only if positive and less than int_word_size.
    // if shift_arg is negative then it's unusable; parse_error will be thrown
    // in that case. if shift_arg is >= int_word_size then we will simulate
    // shifting beyond that limit
        using ShiftT = std::decay_t<decltype(shift_arg)>;
        if constexpr (calc_val::is_int_type<ShiftT>() && std::is_signed_v<ShiftT>) {
            if (shift_arg < 0)
                throw calc_parse_error(calc_parse_error::negative_shift_invalid, op_token);
            return shift_arg < int_word_size;
        } else if constexpr (calc_val::is_int_type<ShiftT>())
            return shift_arg < int_word_size;
    };

    auto lval = additive_expr(lexer);
    for (;;) {
        if (lexer.peek_token().id == lexer_token::shiftl) {
            lexer_token op_token = lexer.get_token();
            auto rval = additive_expr(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                if constexpr (calc_val::is_int_type<LVT>() && calc_val::is_int_type<RVT>()) {
                    if (shift_arg_in_range(rval, op_token))
                        return trim_if_int(lval << rval);
                    return LVT(0);
                } else if (!calc_val::is_int_type<LVT>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else if (lexer.peeked_token().id == lexer_token::shiftr) {
            lexer_token op_token = lexer.get_token();
            auto rval = additive_expr(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                using LVT = std::decay_t<decltype(lval)>;
                using RVT = std::decay_t<decltype(rval)>;
                if constexpr (calc_val::is_int_type<LVT>() && std::is_signed_v<LVT> && calc_val::is_int_type<RVT>()) {
                    if (shift_arg_in_range(rval, op_token))
                        return lval >> rval;
                    else if (lval < 0)
                        return LVT(-1); // -1 doesn't need to be trimmed -- sign extended value
                    else
                        return LVT(0);
                } else if constexpr (calc_val::is_int_type<LVT>() && calc_val::is_int_type<RVT>()) {
                    calc_val::uint_type shift_arg = 0;
                    if (shift_arg_in_range(rval, op_token))
                        return lval >> shift_arg;
                    return LVT(0);
                } else if (!calc_val::is_int_type<LVT>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else
            break;
    }
    return lval;
}

auto calc_parser::additive_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <additive_expr> ::= <term> [ ( "+" | "-" ) <term> ]...
    auto lval = term(lexer);
    for (;;) {
        if (lexer.peek_token().id == lexer_token::add) {
            lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                return trim_if_int(lval + rval); // trim incase of overflow
            }, lval, term(lexer));
        } else if (lexer.peeked_token().id == lexer_token::sub) {
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
        if (lexer.peek_token().id == lexer_token::mul) {
            lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                return trim_if_int(lval * rval); // trim incase of overflow
            }, lval, factor(lexer));
        } else if (lexer.peeked_token().id == lexer_token::div) {
            auto op_token = lexer.get_token();
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (calc_val::is_int_type<decltype(lval)>() && calc_val::is_int_type<decltype(rval)>()) {
                    if (rval == 0)
                        throw calc_parse_error(calc_parse_error::integer_division_by_0, op_token);
                }
                return trim_if_int(lval / rval); // note: −32768 / −1 overflows 16 bit int, thus need to trim
            }, lval, factor(lexer));
        } else if (lexer.peeked_token().id == lexer_token::mod) {
            auto op_token = lexer.get_token();
            auto rval = factor(lexer);
            try_to_make_int_if_complex(lval);
            try_to_make_int_if_complex(rval);
            lval = std::visit([&](const auto& lval, const auto& rval) -> calc_val::variant_type {
                assert(is_nan(lval) || lval == trim_if_int(lval));
                assert(is_nan(rval) || rval == trim_if_int(rval));
                if constexpr (calc_val::is_int_type<decltype(lval)>() && calc_val::is_int_type<decltype(rval)>()) {
                    if (rval == 0)
                        throw calc_parse_error(calc_parse_error::integer_division_by_0, op_token);
                    return trim_if_int(lval % rval); // trim for good measure
                } else if constexpr (!calc_val::is_int_type<decltype(lval)>())
                    throw calc_parse_error(calc_parse_error::invalid_left_operand, op_token);
                else
                    throw calc_parse_error(calc_parse_error::invalid_right_operand, op_token);
            }, lval, rval);
        } else
            break;
    }
    return lval;
}

auto calc_parser::factor(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <factor> ::= "-" <number> ( <any_token> - ( <factorial op> | "^" | "**" ) )
//            | ( "-" | "+" | "~" ) <factor>
//            | <base> [ <factorial operator> ]... [ "^" | "**" <factor> ]
// <factorial op> ::= "!" | "!!" | <mfac>
// note: exponentiation is evaluated right-to-left
    if (lexer.peek_token().id == lexer_token::sub) { // "-'
        lexer.get_token();

        // special case: "-" <number> ( <any_token> - ( <factorial op> | "^" | "**" ) )
        // this is needed to properly negate and range check the number
        if (lexer.peek_token().id == lexer_token::number &&
                lexer.peek_token2().id != lexer_token::fac &&
                lexer.peek_token2().id != lexer_token::dfac &&
                lexer.peek_token2().id != lexer_token::mfac &&
                lexer.peek_token2().id != lexer_token::pow)
            return assumed_number(lexer, true);

        return std::visit([&](const auto& val) -> calc_val::variant_type {
            assert(is_nan(val) || val == trim_if_int(val));
            return trim_if_int(-val);
        }, factor(lexer));
    }

    if (lexer.peek_token().id == lexer_token::add) { // "+" -- just return <factor>
        lexer.get_token();
        return factor(lexer);
    }

    if (lexer.peek_token().id == lexer_token::bnot) { // "~"
        auto op_token = lexer.get_token();
        auto val = factor(lexer);
        try_to_make_int_if_complex(val);
        return std::visit([&](const auto& val) -> calc_val::variant_type {
            assert(is_nan(val) || val == trim_if_int(val));
            if constexpr (calc_val::is_int_type<decltype(val)>())
                return trim_if_int(~val);
            else
                throw calc_parse_error(calc_parse_error::invalid_operand, op_token);
        }, val);
    }

    // <base>

    auto lval = base(lexer);

    // [ <factorial op> ]...

    for (;;) {
        if (lexer.peek_token().id == lexer_token::fac) {
            lexer.get_token();
            lval = std::visit([](const auto& val) -> calc_val::variant_type {
                return calc_val::tgamma(val + 1);
            }, lval);
        } else if (lexer.peeked_token().id == lexer_token::dfac) {
            lexer.get_token();
            lval = std::visit([](const auto& val) -> calc_val::complex_type {
                return calc_val::dfac(val);
            }, lval);
        } else if (lexer.peeked_token().id == lexer_token::mfac)
            throw calc_parse_error(calc_parse_error::mfac_unsupported, lexer.get_token());
        else
            break;
    }

    // [ "^" | "**" <factor> ]

    if (lexer.peek_token().id == lexer_token::pow) {
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
    if (lexer.peek_token().id == lexer_token::number)
        return assumed_number(lexer, false);
    if (lexer.peeked_token().id == lexer_token::identifier)
        return assumed_identifier_expr(lexer);
    if (lexer.peeked_token().id == lexer_token::lparen)
        return group(lexer);
    if (lexer.peeked_token().id == lexer_token::help)
        throw calc_parse_error(calc_parse_error::help_invalid_here, lexer.peeked_token());
    if (lexer.peeked_token().id == lexer_token::end)
        throw calc_parse_error(calc_parse_error::unexpected_end_of_input, lexer.peeked_token());
    throw calc_parse_error(calc_parse_error::syntax_error, lexer.peeked_token());
}

auto calc_parser::assumed_identifier_expr(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <identifier_expr> ::= <identifier> = <math_expr>
//                     | <value_variable>
//                     | <unary_fn_variable> <group>
//                     | <undefined_identifier>
    auto identifier_token = lexer.get_token(); // assume next token is identifier (caller assures this)
    assert(identifier_token.id == lexer_token::identifier);
    auto identifier = identifier_token.view;

    // <identifier> = <math_expr>

    if (lexer.peek_token().id == lexer_token::eq) {
        lexer.get_token();
        auto val = math_expr(lexer);
        trim_int(val);
        variables.insert_or_assign(tmp_str = identifier, val);
        return val;
    }

    // <undefined_identifier>

    auto itr = variables.find(std::string(identifier));
    if (itr == variables.end()) // <undefined_identifier>
        throw calc_parse_error(calc_parse_error::undefined_identifier, identifier_token);

    // <value_variable> | <unary_fn_variable> <group>

    auto val = std::visit([&](const auto& thing) -> calc_val::variant_type {
        using VT = std::decay_t<decltype(thing)>;
        if constexpr (std::is_same_v<VT, calc_val::variant_type>) // <value_variable>
            return thing;
        else if constexpr (std::is_same_v<VT, unary_fn>) // <unary_fn_variable> <group>
            return std::visit([&](const auto& val) {
                return thing(val);
            }, group(lexer));
    }, itr->second);
    trim_int(val);
    return val;
};

auto calc_parser::group(lookahead_calc_lexer& lexer) -> calc_val::variant_type {
// <group> ::= "(" <math_expr> ")"
    if (lexer.get_token().id != lexer_token::lparen)
        throw calc_parse_error(calc_parse_error::token_expected, lexer.last_token(), lexer_token::lparen);
    auto val = math_expr(lexer);
    if (lexer.get_token().id != lexer_token::rparen)
        throw calc_parse_error(calc_parse_error::token_expected, lexer.last_token(), lexer_token::rparen);
    return val;
}
