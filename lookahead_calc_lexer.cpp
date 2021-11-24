#include "lookahead_calc_lexer.hpp"

auto lookahead_calc_lexer::peek_token() -> const lexer_token& {
    if (!peeked) {
        peeked_token_ = lexer.get_token();
        peeked = 1;
    }
    return peeked_token_;
}

auto lookahead_calc_lexer::peek_token2() -> const lexer_token& {
    if (peeked != 2) {
        peek_token();
        peeked_token2_ = lexer.get_token();
        peeked = 2;
    }
    return peeked_token2_;
}

auto lookahead_calc_lexer::get_token() -> const lexer_token& {
    if (!peeked)
        last_token_ = lexer.get_token();
    else {
        last_token_ = peeked_token_;
        if (--peeked) {
            peeked_token_ = peeked_token2_;
            assert(peeked == 1);
        }
    }
    return last_token_;
}
