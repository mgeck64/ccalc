#ifndef IS_DIGIT_H
#define IS_DIGIT_H

extern const unsigned char alphanumeric_digits_lut[128];

#ifdef __cplusplus

inline bool is_digit(unsigned char c, unsigned radix) {
    return static_cast<unsigned>(alphanumeric_digits_lut[c & 127]) - 1 < radix;
    // subtract 1 so ordinal value of digit character becomes 0 based (0 to 35),
    // and default 0 value for non-digit becomes unsigned(-1) (all bits are 1),
    // which will test false for < radix
}

inline bool is_digit_older_version(char c, unsigned radix) {
    static_assert( // assert 0-9, a-z and A-Z sets are contiguous
        '1' - '0' == 1  && '2' - '0' == 2  && '3' - '0' == 3  && '4' - '0' == 4  &&
        '5' - '0' == 5  && '6' - '0' == 6  && '7' - '0' == 7  && '8' - '0' == 8  &&
        '9' - '0' == 9  &&
        'b' - 'a' == 1  && 'c' - 'a' == 2  && 'd' - 'a' == 3  && 'e' - 'a' == 4  &&
        'f' - 'a' == 5  && 'g' - 'a' == 6  && 'h' - 'a' == 7  && 'i' - 'a' == 8  &&
        'j' - 'a' == 9  && 'k' - 'a' == 10 && 'l' - 'a' == 11 && 'm' - 'a' == 12 &&
        'n' - 'a' == 13 && 'o' - 'a' == 14 && 'p' - 'a' == 15 && 'q' - 'a' == 16 &&
        'r' - 'a' == 17 && 's' - 'a' == 18 && 't' - 'a' == 19 && 'u' - 'a' == 20 &&
        'v' - 'a' == 21 && 'w' - 'a' == 22 && 'x' - 'a' == 23 && 'y' - 'a' == 24 &&
        'z' - 'a' == 25 &&
        'B' - 'A' == 1  && 'C' - 'A' == 2  && 'D' - 'A' == 3  && 'E' - 'A' == 4  &&
        'F' - 'A' == 5  && 'G' - 'A' == 6  && 'H' - 'A' == 7  && 'I' - 'A' == 8  &&
        'J' - 'A' == 9  && 'K' - 'A' == 10 && 'L' - 'A' == 11 && 'M' - 'A' == 12 &&
        'N' - 'A' == 13 && 'O' - 'A' == 14 && 'P' - 'A' == 15 && 'Q' - 'A' == 16 &&
        'R' - 'A' == 17 && 'S' - 'A' == 18 && 'T' - 'A' == 19 && 'U' - 'A' == 20 &&
        'V' - 'A' == 21 && 'W' - 'A' == 22 && 'X' - 'A' == 23 && 'Y' - 'A' == 24 &&
        'Z' - 'A' == 25);
    if (auto x = static_cast<unsigned>(c) - '0'; x < 10)
        return x < radix;
    if (auto x = static_cast<unsigned>(c) - 'a'; x < 26)
        return x + 10 < radix;
    if (auto x = static_cast<unsigned>(c) - 'A'; x < 26)
        return x + 10 < radix;
    return false;
}

#endif

#endif // IS_DIGIT_H
