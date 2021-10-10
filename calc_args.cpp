#include "calc_args.h"
#include "const_string_itr.h"
#include <cctype>

static bool single_flag_option(const_string_itr arg_itr, calc_args& args);
static bool double_flag_option(const_string_itr arg_itr, calc_args& args);

void interpret_arg(std::string_view arg_str, char option_code, calc_args& args) {
    auto arg_itr = const_string_itr(arg_str);
    if (arg_itr && *arg_itr == option_code) {
        if (single_flag_option(++arg_itr, args))
            return;
        if (arg_itr && *arg_itr == option_code)
            if (double_flag_option(++arg_itr, args))
                return;
    }
    args.other_arg = arg_str;
    ++args.n_other_args;
}

static bool single_flag_option(const_string_itr arg_itr, calc_args& args) {
    auto arg_view = arg_itr.view();
    if (arg_view == "h" || arg_view == "help") {
        ++args.n_help_options;
        return true;
    }
    if (arg_view == "w8") {
        args.int_word_size = calc_val::int_bits_8;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w16") {
        args.int_word_size = calc_val::int_bits_16;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w32") {
        args.int_word_size = calc_val::int_bits_32;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w64") {
        args.int_word_size = calc_val::int_bits_64;
        ++args.n_int_word_size_options;
        return true;
    }

    // if arg is ( '0' | 'm' ) <prefix code> [ <suffix code> ] <end>
    //     update <input defaults>
    // if arg is ( 'o' | 'm' ) <prefix code> <end>
    //     update <output base>
    // note: update both <input defaults> and <output base> for code 'm'

    auto option_code = arg_itr ? std::tolower(*arg_itr++) : '\0'; // might be '0' | 'o' | 'm'
    auto prefix_code = arg_itr ? std::tolower(*arg_itr++) : '\0'; // might be <prefix code>
    auto updated = false;

    auto default_number_radix = calc_val::base10;
    auto output_radix = calc_val::base10;
    switch (prefix_code) {
        case base2_prefix_code:
            default_number_radix = calc_val::base2;
            output_radix = calc_val::base2;
            break;
        case base8_prefix_code:
            default_number_radix = calc_val::base8;
            output_radix = calc_val::base8;
            break;
        case base10_prefix_code:
            default_number_radix = calc_val::base10;
            output_radix = calc_val::base10;
            break;
        case base16_prefix_code:
            output_radix = calc_val::base16;
            default_number_radix = calc_val::base16;
            break;
        default:
            option_code = '\0';
    }

    if (option_code == '0' || option_code == 'm') {
        auto default_number_type_code = [&] {
            if (arg_itr) {
                auto suffix_code = std::tolower(*arg_itr);
                switch (prefix_code) {
                    case base10_prefix_code:
                    case base16_prefix_code:
                        if (suffix_code == complex_suffix_code) {
                            ++arg_itr;
                            return calc_val::complex_code;
                            break;
                        }
                        [[fallthrough]];
                    case base2_prefix_code:
                    case base8_prefix_code:
                        if (suffix_code == unsigned_suffix_code) {
                            ++arg_itr;
                            return calc_val::uint_code;
                            break;
                        }
                }
            }
            return calc_val::int_code;
        }();

        if (arg_itr.at_end()) {
            args.default_number_radix = default_number_radix;
            args.default_number_type_code = default_number_type_code;
            ++args.n_default_options;
            updated = true;
        }
    }

    if ((option_code == 'o' || option_code == 'm') && arg_itr.at_end()) {
        args.output_radix = output_radix;
        ++args.n_output_options;
        updated = true;
    }

    return updated;
}

static bool double_flag_option(const_string_itr arg_itr, calc_args& args) {
    if (arg_itr.view() == "help") {
        ++args.n_help_options;
        return true;
    }
    return false;
}
