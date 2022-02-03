#include "calc_parser.hpp"
#include "calc_parse_error.hpp"
#include "is_digit.h"
#include "calc_args.hpp"
#include "from_chars.hpp"

// implementation of two very closely related functions regarding scanning and
// converting number tokens

void calc_lexer::scan_as_number() {
// scans for a sequence of characters that resembles a number. the sequence will
// be converted to internal numeric representation (and thus validated) by the
// parser
    auto radix = default_number_radix_;

    auto in_itr2 = in_itr;
    // in_itr2.valid() should be true but to be safe let's not assume that

    auto has_leading_digit = false;
    auto has_alnum = false;

    if (in_itr2 && *in_itr2 == '0') {
        auto prefix_code_1 = null_prefix_code;
        auto prefix_code_2_is_ok = false;
        auto inc_in_itr2 = 0;

        if (in_itr2.length() > 2 && !is_digit_any_decimal(in_itr2[2], radix)) {
            auto prefix_code_2 = std::tolower(in_itr[2]);
            prefix_code_2_is_ok = prefix_code_2 == signed_prefix_code || prefix_code_2 == unsigned_prefix_code || prefix_code_2 == complex_prefix_code;
        }
        if (prefix_code_2_is_ok) {
            prefix_code_1 = std::tolower(in_itr2[1]);
            inc_in_itr2 = 3;
        } else if (in_itr2.length() > 1 && !is_digit_any_decimal(in_itr2[1], radix)) {
            prefix_code_1 = std::tolower(in_itr2[1]);
            prefix_code_2_is_ok = true; // implied code
            inc_in_itr2 = 2;
        }

        switch (prefix_code_1) {
            case base2_prefix_code:
                if (prefix_code_2_is_ok) {
                    radix = calc_val::base2;
                    in_itr2 += inc_in_itr2;
                }
                break;
            case base8_prefix_code:
                if (prefix_code_2_is_ok) {
                    radix = calc_val::base8;
                    in_itr2 += inc_in_itr2;
                }
                break;
            case base10_prefix_code:
                if (prefix_code_2_is_ok) {
                    radix = calc_val::base10;
                    in_itr2 += inc_in_itr2;
                }
                break;
            case base16_prefix_code:
                if (prefix_code_2_is_ok) {
                    radix = calc_val::base16;
                    in_itr2 += inc_in_itr2;
                }
                break;
        }

        has_leading_digit = true;
    } else if (in_itr2 && std::isdigit(*in_itr2)) {
        ++in_itr2;
        has_leading_digit = true;
        has_alnum = true;
    }

    auto exponent_code = radix == calc_val::base10 ? 'e' : 'p';
    auto has_decimal_point = false;

    while (in_itr2) {
        if (*in_itr2 == '.' && !has_decimal_point) {
            ++in_itr2;
            has_decimal_point = true;
        } else if (is_digit_any_decimal(*in_itr2, radix)) {
            ++in_itr2;
            has_alnum = true;
        } else if (std::tolower(*in_itr2) == exponent_code && has_alnum) {
            auto in_itr3 = in_itr2 + 1;
            if (in_itr3 && (*in_itr3 == '+' || *in_itr3 == '-'))
                ++in_itr3;
            if (in_itr3 && std::isdigit(*in_itr3)) {
                do ++in_itr3;
                    while (in_itr3 && std::isdigit(*in_itr3));
                in_itr2 = in_itr3;
            }
            break;
        } else
            break;
    }

    if ((has_leading_digit || has_decimal_point) && has_alnum)
        in_itr = in_itr2;
    else if (has_leading_digit)
        ++in_itr;
}

auto calc_parser::assumed_number(lookahead_calc_lexer& lexer, bool is_negative) -> calc_val::variant_type {
// gets the next token, which is assumed to have been coded as a number.
// converts the character sequence to internal numeric representation (and thus
// final-validates it)
    auto token = lexer.get_token(); // assume next token is number (caller should assure this)
    assert(token.id == lexer_token::number);

    auto num_itr = const_string_itr(token.view);
    auto type_code = default_number_type_code;
    auto radix = default_number_radix;

    // see calc_lexer::scan_as_number above
    if (num_itr && *num_itr == '0') {
        auto prefix_code_1 = null_prefix_code;
        auto prefix_code_2 = null_prefix_code;
        auto inc_num_itr = 0;

        if (num_itr.length() > 2 && !is_digit_any_decimal(num_itr[2], radix)) // 0<prefix code 1><prefix code 2><char>
            prefix_code_2 = std::tolower(num_itr[2]);
        if (prefix_code_2 == signed_prefix_code || prefix_code_2 == unsigned_prefix_code || prefix_code_2 == complex_prefix_code) {
            prefix_code_1 = std::tolower(num_itr[1]);
            inc_num_itr = 3;
        } else if (num_itr.length() > 1 && !is_digit_any_decimal(num_itr[1], radix)) { // 0<prefix code 1><char>
            prefix_code_1 = std::tolower(num_itr[1]);
            prefix_code_2 = signed_prefix_code;
            inc_num_itr = 2;
        }

        switch (prefix_code_1) {
            case base2_prefix_code:
                if (prefix_code_2 == signed_prefix_code) {
                    radix = calc_val::base2;
                    type_code = calc_val::int_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == unsigned_prefix_code) {
                    radix = calc_val::base2;
                    type_code = calc_val::uint_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == complex_prefix_code) {
                    radix = calc_val::base2;
                    type_code = calc_val::complex_code;
                    num_itr += inc_num_itr;
                }
                break;
            case base8_prefix_code:
                if (prefix_code_2 == signed_prefix_code) {
                    radix = calc_val::base8;
                    type_code = calc_val::int_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == unsigned_prefix_code) {
                    radix = calc_val::base8;
                    type_code = calc_val::uint_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == complex_prefix_code) {
                    radix = calc_val::base8;
                    type_code = calc_val::complex_code;
                    num_itr += inc_num_itr;
                }
                break;
            case base10_prefix_code:
                if (prefix_code_2 == signed_prefix_code) {
                    radix = calc_val::base10;
                    type_code = calc_val::int_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == unsigned_prefix_code) {
                    radix = calc_val::base10;
                    type_code = calc_val::uint_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == complex_prefix_code) {
                    radix = calc_val::base10;
                    type_code = calc_val::complex_code;
                    num_itr += inc_num_itr;
                }
                break;
            case base16_prefix_code:
                if (prefix_code_2 == signed_prefix_code) {
                    radix = calc_val::base16;
                    type_code = calc_val::int_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == unsigned_prefix_code) {
                    radix = calc_val::base16;
                    type_code = calc_val::uint_code;
                    num_itr += inc_num_itr;
                } else if (prefix_code_2 == complex_prefix_code) {
                    radix = calc_val::base16;
                    type_code = calc_val::complex_code;
                    num_itr += inc_num_itr;
                }
                break;
        }
    }

    auto exponent_code = radix == calc_val::base10 ? 'e' : 'p';

    for (auto c : num_itr) { // check for decimal point or exponent code
        if (c == '.' || tolower(c) == exponent_code) {
            type_code = calc_val::complex_code;
            break;
        }
    }

    calc_val::float_type float_val = 0;
    calc_val::max_uint_type uint_val = 0;

    std::from_chars_result from_char_result;
    if (type_code == calc_val::complex_code)
        from_char_result = calc_val::from_chars(num_itr.begin(), num_itr.end(), float_val, radix);
    else {
        assert(type_code == calc_val::uint_code || type_code == calc_val::int_code);
        from_char_result = std::from_chars(num_itr.begin(), num_itr.end(), uint_val, radix);
    }
    if (from_char_result.ec == std::errc::result_out_of_range)
        throw calc_parse_error(calc_parse_error::out_of_range, token);
    else if (from_char_result.ec != std::errc() || from_char_result.ptr != num_itr.end())
        throw calc_parse_error(calc_parse_error::invalid_number, token);

    calc_val::variant_type val;
    bool out_of_range = false;

    if (type_code == calc_val::uint_code) {
        auto val_for = [](auto tag, auto uint_val, bool is_negative, bool& out_of_range) -> calc_val::uint_type {
            using uint_t = std::decay_t<decltype(tag)>;
            static_assert(std::is_integral_v<uint_t>);
            static_assert(std::is_unsigned_v<uint_t>);
            if (uint_t(uint_val) != uint_val)
                out_of_range = true;
            else
                tag = is_negative ? -static_cast<uint_t>(uint_val) : static_cast<uint_t>(uint_val);
            static_assert(sizeof(tag) <= sizeof(calc_val::uint_type));
            return tag;
        };

        switch (int_word_size) {
            case calc_val::int_bits_8:
                val = val_for(std::uint8_t{}, uint_val, is_negative, out_of_range);
                break;
            case calc_val::int_bits_16:
                val = val_for(std::uint16_t{}, uint_val, is_negative, out_of_range);
                break;
            case calc_val::int_bits_32:
                val = val_for(std::uint32_t{}, uint_val, is_negative, out_of_range);
                break;
            case calc_val::int_bits_64:
                val = val_for(std::uint64_t{}, uint_val, is_negative, out_of_range);
                break;
            default:
                assert(int_word_size == calc_val::int_bits_128);
                val = val_for((unsigned __int128){}, uint_val, is_negative, out_of_range);
        }
    } else if (type_code == calc_val::int_code) {
        auto val_for = [](auto tag, auto uint_val, bool is_negative, calc_val::radices radix, bool& out_of_range) -> calc_val::int_type {
            using int_t = std::decay_t<decltype(tag)>;
            using uint_t = std::make_unsigned_t<int_t>;
            static_assert(std::is_integral_v<int_t>);
            static_assert(std::is_signed_v<int_t>);
            static_assert(sizeof(int_t) == sizeof(uint_t));
            // calc_val::base10 checks: for base 10, perform normal range
            // checking; for other bases, allow e.g. 0xffff to convert to -1 for
            // 16 bit integer and not be an out of range error
            if (is_negative) {
                if ((radix == calc_val::base10 && uint_val > -static_cast<decltype(uint_val)>(std::numeric_limits<int_t>::min()))
                        || (uint_val > std::numeric_limits<uint_t>::max()))
                    out_of_range = true;
                else
                    tag = -static_cast<int_t>(uint_val);
            } else {
                if ((radix == calc_val::base10 && uint_val > static_cast<uint_t>(std::numeric_limits<int_t>::max()))
                        || (uint_val > std::numeric_limits<uint_t>::max()))
                    out_of_range = true;
                else
                    tag = uint_val;
            }
            static_assert(sizeof(tag) <= sizeof(calc_val::int_type));
            return tag;
        };

        switch (int_word_size) {
            case calc_val::int_bits_8:
                val = val_for(std::int8_t{}, uint_val, is_negative, radix, out_of_range);
                break;
            case calc_val::int_bits_16:
                val = val_for(std::int16_t{}, uint_val, is_negative, radix, out_of_range);
                break;
            case calc_val::int_bits_32:
                val = val_for(std::int32_t{}, uint_val, is_negative, radix, out_of_range);
                break;
            case calc_val::int_bits_64:
                val = val_for(std::int64_t{}, uint_val, is_negative, radix, out_of_range);
                break;
            default:
                assert(int_word_size == calc_val::int_bits_128);
                val = val_for(__int128{}, uint_val, is_negative, radix, out_of_range);
        }
    } else {
        assert(type_code == calc_val::complex_code);
        if (is_negative)
            float_val = -float_val;
        val = calc_val::complex_type(float_val, 0);
    }

    if (out_of_range)
        throw calc_parse_error(calc_parse_error::out_of_range, token);

    return val;
}
