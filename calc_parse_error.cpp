#include "calc_parse_error.h"

auto calc_parse_error::error_str() const -> std::string {
    std::string error_str;

    error_str = "Error: ";

    if (error_ == token_expected) {
        assert(std::strlen(calc_token::token_txt.at(expected_token_id_)));
        error_str += calc_token::token_txt.at(expected_token_id_);
        error_str += " ";
    }

    error_str += error_txt.at(error_);
    error_str += ".";

    return error_str;
}
