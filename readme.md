# ccalc
## Synopsis
An advanced text-based command-line calculator app.
- Supports complex arithmetic, integer arithmetic and bitwise operations
- Supports 8, 16, 32, 64 and 128 bit integer types
- Supports binary, octal, decimal and hexadecimal numbers, both integer and
floating point (and complex)
- Supports floating point numbers up to 50 decimal significant digits (+ guard
digits)
## Dependencies
- This project depends on Boost; this was built using Boost version 1.74. Note:
the newer versions 1.75 and 1.77 produce a slightly incorrect result for the
expression exp(pi*i) where i is the imaginary unit; thus version 1.74 is
statically asserted for in the code
- This project uses the GNU __int128 type with GNU extensions enabled