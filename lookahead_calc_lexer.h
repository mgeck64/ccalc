#pragma once
#ifndef LOOKAHEAD_CALC_LEXER_H
#define LOOKAHEAD_CALC_LEXER_H

#include "calc_lexer.h"

class lookahead_calc_lexer {
// simulates two-token lookhead lexer using calc_lexer. (implemented
// separately from calc_lexer to keep calc_lexer clean and simple, and to
// encapsulate lookahead logic)
public:
    using token_ids = typename calc_token::token_ids;

    lookahead_calc_lexer(std::string_view input, calc_val::radices default_number_radix)
        : lexer{input, default_number_radix} {}
        
    void default_number_radix(calc_val::radices default_number_radix)
    {lexer.default_number_radix(default_number_radix);}

    auto get_token() -> const calc_token&; // consume a token; throws parse_error if token has error
    auto last_token() const -> const calc_token& {return last_token_;}

    auto peek_token() -> const calc_token&; // peek at but don't consume token
    auto peeked_token() const -> const calc_token& {return peeked_token_;}

    auto peek_token2() -> const calc_token&; // peek at second token
    auto peeked_token2() const -> const calc_token& {return peeked_token2_;}

private:
    calc_lexer lexer;
    unsigned peeked = 0;
    calc_token last_token_ = {};
    calc_token peeked_token_ = {};
    calc_token peeked_token2_ = {};
};

#endif // LOOKAHEAD_CALC_LEXER_H