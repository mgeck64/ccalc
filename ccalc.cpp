#include "calc_args.hpp"
#include "calc_parser.hpp"
#include "calc_parse_error.hpp"
#include "calc_outputter.hpp"
#include "const_string_itr.hpp"
#include <iostream>

static void evaluate(std::string_view expression, calc_parser& parser, calc_val::radices& output_radix);
static void help();

int main(int argc, const char** argv) {
    calc_args args;

    for (int argi = 1; argi < argc; ++argi)
        interpret_arg(argv[argi], '-', args);

    if (!args.n_help_options
            && args.n_default_options < 2
            && args.n_output_options < 2
            && args.n_int_word_size_options < 2
            && args.n_other_args < 2) {
        calc_parser parser(args.default_number_type_code,
            args.default_number_radix, args.int_word_size);
        if (!args.other_arg.empty()) // expression provided as argument
            evaluate(args.other_arg, parser, args.output_radix);
        else { // input expressions from stdin
            std::string expression;
            for (;;) {
                std::getline(std::cin, expression);
                const_string_itr expression_itr = expression;
                while (expression_itr && std::isspace(*expression_itr))
                    ++expression_itr;
                if (expression_itr.at_end()) // done
                    break;
                evaluate(expression_itr, parser, args.output_radix);
            }
        }
    } else {
        if (args.n_default_options + args.n_output_options
                + args.n_int_word_size_options + args.n_other_args)
            std::cout << "Too many or invalid arguments." << '\n';
        help();
    }

    return 0;
}

static void evaluate(std::string_view expression, calc_parser& parser, calc_val::radices& output_radix) {
    try {
        auto result = parser.evaluate(expression, help, output_radix);
        calc_outputter outputter{output_radix};
        std::cout << outputter(result) << std::endl;
    } catch (const calc_parse_error& e) {
        std::cout << expression << '\n';
        for (auto n = e.token().view_offset; n > 0; --n)
            std::cout << ' ';
        for (auto n = e.token().view.size() ? e.token().view.size() : 1; n > 0; --n) // show at least one "hat" incase error is at end of line
            std::cout << '^';
        std::cout << '\n';
        std::cout << e.error_str() << std::endl;
    } catch (const calc_parser::no_mathematical_expression) {
        // do nothing
    }
}

static void help() {
    std::cout <<
"\
Basic guide:\n\
ccalc [<input defaults>] [<output base>] [<mode>] [<int word size>] [-h]\n\
[--help] [<expression>]\n\
\n\
<expression>: A mathematical expression, e.g.: 2+3*6. If omitted then\n\
expressions will continuously be input from stdin. Exception: if <expression> is\n\
\"help\" then this content will be printed.\n\
\n\
<input defaults>: Specifies the default representation type and default numeric\n\
base for numbers:\n\
    -0b  - signed integer type, binary base; e.g.: 1010\n\
    -0o  - signed integer type, octal base; e.g.: 12\n\
    -0d  - signed integer type, decimal base; e.g.: 10\n\
    -0x  - signed integer type, hexadecimal base; e.g.: 0a (prepend a number\n\
           with 0 if it consists only of letter digits)\n\
    -0bu - unsigned integer type, binary base\n\
    -0ou - unsigned integer type, octal base\n\
    -0du - unsigned integer type, decimal base\n\
    -0xu - unsigned integer type, hexadecimal base\n\
    -0dn - complex type, decimal base -- the default; e.g.: 10, 10+2*i\n\
    -0xn - complex type, hexadecimal base (hexadecimal floating point)\n\
Complex type: represents a complex number composed of a real and imaginary part,\n\
both of which are of extended precision floating point type. The full form of a\n\
complex number is given here as a+b*i (and not a+bi; the calculator doesn't\n\
support implied multiplication). Examples: 10+2*i (real part is 10, imaginary\n\
part is 2*i), 10 (real number; imaginary part is 0), 2*i (imaginary number; real\n\
part is 0).\n\
Exception for -0d, -0du, -0x and -0xu:  if a number is specified with a decimal\n\
point or exponent then it will be represented as complex type; e.g., for -0x and\n\
-0xu, the numbers a.1 and 0a1p-4 will both be represented as complex type and\n\
interpreted in hexadecimal base.\n\
Note: Hexadecimal floating point is described here:\n\
https://www.exploringbinary.com/hexadecimal-floating-point-constants/\n\
\n\
<output base>: Specifies the numeric base of the output:\n\
    -ob - binary\n\
    -oo - octal\n\
    -od - decimal -- the default\n\
    -ox - hexadecimal\n\
Note: <output base> does not affect the output's representation type. Binary,\n\
octal and hexadecimal output of complex numbers is consistent with hexadecimal\n\
floating point (see above).\n\
\n\
<mode>: Combines <input defaults> and <output base>: -mb (-0b -ob), -mo (-0o\n\
-oo), -md (-0d -od), -mx (-0x -ox), -mbu (-0bu -ob), -mou (-0ou -oo), -mdu\n\
(-0du -od), -mxu (-0xu -ox), -mdn (-0dn -od), -mxn (-0xn -ox).\n\
\n\
<int word size>: Specifies the word size for the integer types:\n\
    -w8  -  8 bits\n\
    -w16 - 16 bits\n\
    -w32 - 32 bits\n\
    -w64 - 64 bits -- the default\n\
Note: this does not affect the complex type.\n\
\n\
Options may also be provided in an expression (e.g., when input from stdin);\n\
options provided this way begin with '@' instead of '-' (because '-' is the\n\
subtraction/negation operator); e.g., @0x @w32\n\
\n\
A number may optionally be given a prefix, suffix or both to specify its numeric\n\
base and representation type, overriding the default ones.\n\
Prefixes:\n\
    0b - binary base; e.g.: 0b1010\n\
    0o - octal base; e.g.: 0o12\n\
    0d - decimal base; e.g.: 0d10\n\
    0x - hexadecimal base; e.g.: 0xa\n\
Suffixes:\n\
    s    - signed integer type; e.g., 0b1010s, 10s\n\
    u    - unsigned integer type; e.g., 0b1010u, 10u\n\
    n    - complex type; e.g., 0x10n; valid only for decimal base and\n\
           hexadecimal base\n\
    none - if the number has a prefix (e.g., 0d10) then signed integer type;\n\
           otherwise (e.g., 10) the default representation type\n\
Exception for decimal base and hexadecimal base: if the number has a decimal\n\
point or exponent then it will be represented as complex type; e.g., 0xa.1 and\n\
0xa1p-4 will both be represented as complex type and interpreted in hexadecimal\n\
base.\n\
Note: 0b and 0d cannot be used when the default numeric base is hexadecimal\n\
because those are valid hexadecimal numbers. For that case, the 0bx and 0dx\n\
prefixes can be used to specify binary base and decimal base respectively.\n\
\n\
Available arithmetic operators:\n\
    + (addition and unary plus) - (subtraction and negation) * (multiplication)\n\
    / (division) % (modulus) ^ ** (exponentiation) ! !! (factorial and double\n\
    factorial) ( ) (grouping)\n\
\n\
Available bitwise operators:\n\
    ~ (not) & (and) | (or) ^| (xor) << >> (shift; algebraic for signed type)\n\
Note: unlike C, C++ and many other programming languages, ^ means exponentiation\n\
here, not bitwise xor; ^| is provided instead for bitwiwise xor.\n\
\n\
Available symbolic values:\n\
    pi, e (Euler's number), i (imaginary unit), last (last result); e.g.,: 2+4*i\n\
\n\
Available functions; e.g.: sin(5):\n\
    exp - exp(n) is e raised to the power of n\n\
    ln - natural (base e) log\n\
    log10 - base 10 log\n\
    log2 - base 2 log\n\
    sqrt - square root\n\
    cbrt - cubic root\n\
    sin\n\
    cos\n\
    tan\n\
    asin - arc sin\n\
    acos - arc cos\n\
    atan - arc tan\n\
    sinh - hyperbolic sin\n\
    cosh - hyperbolic cos\n\
    tanh - hyperbolic tan\n\
    asinh - inverse hyperbolic sin\n\
    acosh - inverse hyperbolic cos\n\
    atanh - inverse hyperbolic tan\n\
    gamma\n\
    lgamma - log gamma\n\
    arg - phase angle\n\
    norm - squared magnitude\n\
    conj - conjugate\n\
    proj - projection onto the Riemann sphere\n\
\n\
Variables can be created and used in expressions, e.g.:\n\
    approx_pi=22/7\n\
    r=5\n\
    approx_pi*r^2\n\
Variable assignments can be chained, e.g.: x=y=2\
" << std::endl;
}
