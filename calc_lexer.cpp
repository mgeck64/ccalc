#include "calc_lexer.hpp"
#include "calc_args.hpp"

auto calc_lexer::get_token() -> lexer_token {
    while (in_itr && std::isspace(*in_itr)) // eat whitespace
        ++in_itr;

    if (in_itr.at_end()) {
        auto token_offset = static_cast<lexer_token::offset_type>(in_itr - in_begin);
        return lexer_token{lexer_token::end, {in_itr.ptr(), 0}, token_offset};
    }

    auto token_id = lexer_token::unspecified;
    auto token_begin = in_itr;

    switch (*in_itr) {
        case '+':
            ++in_itr;
            token_id = lexer_token::add;
            break;
        case '-':
            ++in_itr;
            token_id = lexer_token::sub;
            break;
        case '*':
            ++in_itr;
            if (in_itr && *in_itr == '*') {
                token_id = lexer_token::pow;
                ++in_itr;
            } else
                token_id = lexer_token::mul;
            break;
        case '/':
            ++in_itr;
            token_id = lexer_token::div;
            break;
        case '%':
            ++in_itr;
            token_id = lexer_token::mod;
            break;
        case '(':
            ++in_itr;
            token_id = lexer_token::lparen;
            break;
        case ')':
            ++in_itr;
            token_id = lexer_token::rparen;
            break;
        case '!':
            do ++in_itr;
                while (in_itr && *in_itr == '!');
            if (auto n = in_itr - token_begin; n == 1) // factorial
                token_id = lexer_token::fac;
            else if (n == 2) // double factorial
                token_id = lexer_token::dfac;
            else // multifactorial
                token_id = lexer_token::mfac;
            break;
        case '<':
            if (in_itr.length() > 1 && in_itr[1] == '<') {
                in_itr += 2;
                token_id = lexer_token::shiftl;
            }
            break;
        case '>':
            if (in_itr.length() > 1 && in_itr[1] == '>') {
                in_itr += 2;
                token_id = lexer_token::shiftr;
            }
            break;
        case '&': // bitwise and
            ++in_itr;
            token_id = lexer_token::band;
            break;
        case '|': // bitwise or
            ++in_itr;
            token_id = lexer_token::bor;
            break;
        case '^': // exponentiation or maybe bitwise xor
            ++in_itr;
            if (in_itr && *in_itr == '|') { // bitwise xor
                ++in_itr;
                token_id = lexer_token::bxor;
            } else
                token_id = lexer_token::pow;
            break;
        case '~': // bitwise not
            ++in_itr;
            token_id = lexer_token::bnot;
            break;
        case '=':
            ++in_itr;
            token_id = lexer_token::eq;
            break;
        case expression_option_code:
            do ++in_itr;
                while (in_itr && *in_itr == expression_option_code);
            while (in_itr && std::isalnum(*in_itr))
                ++in_itr;
            token_id = lexer_token::option;
            break;
        default:
            if (std::isalpha(*in_itr) || *in_itr == '_') {
                do ++in_itr;
                    while (in_itr && (std::isalnum(*in_itr) || *in_itr == '_'));
                auto id = std::string_view(token_begin.ptr(), in_itr - token_begin);
                if (id == "help")
                    token_id = lexer_token::help;
                else if (id == "delete")
                    token_id = lexer_token::del;
                else
                    token_id = lexer_token::identifier;
            } else {
                scan_as_number();
                if (in_itr != token_begin)
                    token_id = lexer_token::number;
            }
    }

    auto token_size = static_cast<std::string_view::size_type>(in_itr - token_begin);
    auto token_offset = static_cast<std::string_view::size_type>(token_begin - in_begin);
    return lexer_token{token_id, {token_begin.ptr(), token_size}, token_offset};
}
