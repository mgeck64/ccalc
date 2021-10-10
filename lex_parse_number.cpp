#include "calc_parser.h"
#include "calc_parse_error.h"
#include "is_digit.h"
#include "calc_args.h"

// implementation of two very closely related functions regarding scanning and
// converting number tokens

void calc_lexer::scan_as_number() {
// scans for a sequence of characters that resembles a number. the sequence will
// be converted to internal numeric representation (and thus validated) by the
// parser
    auto radix = default_number_radix_;
    auto number_indicated = false;
    auto has_alnum = false;

    auto in_itr2 = in_itr;
    // in_itr2.valid() should be true but to be safe let's not assume that

    // check for 0<prefix code><char> or 0<prefix code><base16_int_code><char> else check for
    // suffix code later
    if (in_itr.length() > 2 && *in_itr == '0') {
        auto prefix_code = std::tolower(in_itr[1]);
        number_indicated = true;
        auto prefix_len = [&] {
            if (in_itr.length() > 3
                    && !is_digit(in_itr[2], radix) // future proofing incase we decide to support, say, base36
                    && in_itr[2] == base16_prefix_code
                    && (prefix_code == base2_prefix_code || prefix_code == base10_prefix_code))
                return 3;
            else if (!is_digit(prefix_code, radix))
                return 2;
            else { // have leading digits
                has_alnum = true;
                in_itr2 += 2;
                return 0;
            }
        }();

        if (prefix_len) {
            switch (tolower(prefix_code)) {
                case base2_prefix_code:
                    radix = calc_val::base2;
                    in_itr2 += prefix_len;
                    break;
                case base8_prefix_code:
                    radix = calc_val::base8;
                    in_itr2 += prefix_len;
                    break;
                case base10_prefix_code:
                    radix = calc_val::base10;
                    in_itr2 += prefix_len;
                    break;
                case base16_prefix_code:
                    radix = calc_val::base16;
                    in_itr2 += prefix_len;
                    break;
                default: // have leading 0
                    has_alnum = true;
                    ++in_itr2;
            }
        }
    } else if (in_itr2 && std::isdigit(*in_itr2)) { // have leading digit
        ++in_itr2;
        has_alnum = true;
        number_indicated = true;
    }

    auto has_decimal_point = false;

    auto exponent_code = [&] {
        if (radix == calc_val::base10)
            return 'e';
        if (radix == calc_val::base16)
            return 'p';
        return '\0';
    }();

    // assume these codes are alnum and so will be accepted in alnum case below:
    assert(!exponent_code || std::isalnum(exponent_code)); // 0 or alnum
    assert(std::isalnum(unsigned_suffix_code));
    assert(std::isalnum(signed_suffix_code));
    assert(std::isalnum(complex_suffix_code));

    while (in_itr2) {
        if (*in_itr2 == '.') {
            ++in_itr2;
            has_decimal_point = true;
        } else if (exponent_code && std::tolower(*in_itr2) == exponent_code) {
            ++in_itr2;
            if (in_itr2 && (*in_itr2 == '+' || *in_itr2 == '-') && has_alnum) {
                auto in_itr3 = in_itr2;
                do ++in_itr3;
                    while (in_itr3 && std::isdigit(*in_itr3));
                if (in_itr3 - in_itr2 > 1 // at least 1 digit
                        && (in_itr3.at_end() || (!std::isalpha(*in_itr3) && *in_itr3 != '.')))
                    in_itr2 = in_itr3;
                break;
            }
            has_alnum = true;
            exponent_code = 0;
        } else if (std::isalnum(*in_itr2)) {
            ++in_itr2;
            has_alnum = true;
        } else
            break;
    }

    if (number_indicated || (has_alnum && has_decimal_point))
        in_itr = in_itr2;
}

auto calc_parser::assumed_number(lookahead_calc_lexer& lexer, bool is_negative) -> calc_val::variant_type {
// gets the next token, which is assumed to have been scanned as a number;
// converts the character sequence to internal numeric representation (and thus
// validates it)
    auto token = lexer.get_token(); // assume next token is number (caller should assure this)
    assert(token.id == calc_token::number);

    auto num_itr = const_string_itr(token.view);
    auto type_code = default_number_type_code;
    auto radix = default_number_radix;

    // see calc_lexer::scan_as_number above
    if (num_itr.length() > 2 && *num_itr == '0') {
        auto prefix_code = std::tolower(num_itr[1]);
        auto prefix_len = [&] {
            if (num_itr.length() > 3
                    && !is_digit(num_itr[2], radix) // future proofing incase we decide to support, say, base36
                    && num_itr[2] == base16_prefix_code
                    && (prefix_code == base2_prefix_code || prefix_code == base10_prefix_code))
                return 3;
            else if (!is_digit(prefix_code, radix))
                return 2;
            else
                return 0;
        }();

        if (prefix_len) {
            switch (prefix_code) { // set radix and type_code, the latter may be changed for suffix
                case base2_prefix_code:
                    radix = calc_val::base2;
                    type_code = calc_val::int_code;
                    num_itr += prefix_len;
                    break;
                case base8_prefix_code:
                    radix = calc_val::base8;
                    type_code = calc_val::int_code;
                    num_itr += prefix_len;
                    break;
                case base10_prefix_code:
                    radix = calc_val::base10;
                    type_code = calc_val::int_code;
                    num_itr += prefix_len;
                    break;
                case base16_prefix_code:
                    radix = calc_val::base16;
                    type_code = calc_val::int_code;
                    num_itr += prefix_len;
                    break;
            }
        }
    }

    auto exponent_code = [&] {
        if (radix == calc_val::base10)
            return 'e';
        if (radix == calc_val::base16)
            return 'p';
        return '\0';
    }();

    bool is_simple_number = true; // no decimal point or exponent code

    for (auto c : num_itr) { // check for decimal point or exponent code
        if (c == '.' || (exponent_code && tolower(c) == exponent_code)) {
            type_code = calc_val::complex_code;
            is_simple_number = false;
            break;
        }
    }

    if (num_itr) { // check for suffix code
        if (auto c = tolower(num_itr.back()); c == unsigned_suffix_code) {
            if (!is_simple_number)
                throw calc_parse_error(calc_parse_error::this_suffix_invalid_here, token);
            type_code = calc_val::uint_code;
            num_itr.remove_suffix(1);
        } else if (c == signed_suffix_code) {
            if (!is_simple_number)
                throw calc_parse_error(calc_parse_error::this_suffix_invalid_here, token);
            type_code = calc_val::int_code;
            num_itr.remove_suffix(1);
        } else if (c == complex_suffix_code) {
            if (radix != calc_val::base10 && radix != calc_val::base16)
                throw calc_parse_error(calc_parse_error::this_suffix_invalid_here, token);
            type_code = calc_val::complex_code;
            num_itr.remove_suffix(1);
        }
    }

    if (num_itr.at_end()) // prevent empty string from being converted to 0
        throw calc_parse_error(calc_parse_error::invalid_number, token);

    if ((type_code == calc_val::complex_code || radix == calc_val::base16)
            && num_itr.length() > 1
            && *num_itr == '0' && tolower(num_itr[1]) == 'x') // 0x is special prefix code
        throw calc_parse_error(calc_parse_error::invalid_number, token);

    number_buf.clear(); // assume number_buf grows efficiently
    if (type_code == calc_val::complex_code && radix == calc_val::base16)
        // tell conversion function (strtod) to interpret as hex float
        number_buf += "0x";
    number_buf += num_itr.view();

    calc_val::float_type float_val = 0;
    unsigned long long uint_val = 0;
    const char* buf_ptr = number_buf.c_str();
    errno = 0; // global (thread-local) error code set by c conversion function!; must initialize!

    if ((radix == calc_val::base10 && type_code == calc_val::complex_code)
            || (radix == calc_val::base16 && type_code == calc_val::complex_code))
        float_val = std::strtold(buf_ptr, const_cast<char**>(&buf_ptr));
    else if (type_code == calc_val::complex_code)
        throw calc_parse_error(calc_parse_error::integer_number_expected, token);
    else {
        assert(type_code == calc_val::uint_code || type_code == calc_val::int_code);
        uint_val = std::strtoull(buf_ptr, const_cast<char**>(&buf_ptr), radix);
    }

    if (errno == ERANGE)
        throw calc_parse_error(calc_parse_error::out_of_range, token);
    else if (errno) {
        assert(false); // missed a case
        throw calc_parse_error(calc_parse_error::invalid_number, token);
    }

    if (*buf_ptr) // incomplete scan
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
            default:
                val = val_for(std::uint64_t{}, uint_val, is_negative, out_of_range);
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
            // int_bits_16 and not be a out of range error
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
            default:
                assert(int_word_size == calc_val::int_bits_64);
                val = val_for(std::int64_t{}, uint_val, is_negative, radix, out_of_range);
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
