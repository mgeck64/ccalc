#include "calc_outputter.hpp"
#include "stream_state_restorer.hpp"
#include "ieee_fp_parts.hpp"
#include <limits>
#include <cassert>

std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter) {
    (outputter.*outputter.output_fn)(out, outputter.val);
    std::visit([&](const auto& val) {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT> && std::is_signed_v<VT>)
            out << " (int";
        else if constexpr (std::is_integral_v<VT>)
            out << " (uint";
        else if constexpr (std::is_same_v<VT, calc_val::complex_type>)
            out << " (cplx";
        out << " base" << outputter.radix << ')';
    }, outputter.val);
    return out;
}

auto calc_outputter::output_fn_for(calc_val::radices radix) -> output_fn_type {
    switch (radix) {
        case calc_val::base2:
            return &calc_outputter::output_bin;
        case calc_val::base8:
            return &calc_outputter::output_oct;
        case calc_val::base10:
            return &calc_outputter::output_dec;
        case calc_val::base16:
            return &calc_outputter::output_hex;
        default:
            assert(false); // missed one
            return &calc_outputter::output_dec;
    }
}

auto calc_outputter::output_bin(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream& {
    return output(out, val, calc_val::base2);
}

auto calc_outputter::output_oct(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream& {
    return output(out, val, calc_val::base8);
}

auto calc_outputter::output_hex(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream& {
    return output(out, val, calc_val::base16);
}

auto calc_outputter::output_dec(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream& {
    stream_state_restorer restorer(out);
    out.precision(precision10);
    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT>)
            out << std::dec << +val; // +val; incase val is char type, will output as int
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            out << std::defaultfloat;
            if (val.real() != 0 || val.imag() == 0)
                out << val.real();
            if (val.imag() != 0) {
                if (val.real() != 0 && !val.imag().backend().sign())
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    out << val.imag();
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output(std::ostream& out, const calc_val::variant_type& val, calc_val::radices radix) const -> std::ostream& {
    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT> && std::is_signed_v<VT>) {
            auto val_ = static_cast<std::make_unsigned_t<VT>>(val);
            if (val < 0) {
                val_ = -val_;
                out << '-';
            }
            output_as_uint(out, val_, radix);
        } else if constexpr (std::is_integral_v<VT>)
            output_as_uint(out, val, radix);
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            if (val.real() != 0 || val.imag() == 0)
                output_as_ieee_fp(out, val.real(), radix);
            if (val.imag() != 0) {
                if (val.real() != 0 && !val.imag().backend().sign())
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    output_as_ieee_fp(out, val.imag(), radix);
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output_as_uint(std::ostream& out, std::uintmax_t val, calc_val::radices radix) const -> std::ostream& {
    unsigned delimit_at;
    decltype(val) digit_mask;
    size_t digit_shift;
    if (radix == calc_val::base2) {
        delimit_at = 4;
        digit_mask = 1;
        digit_shift = 1;
    } else if (radix == calc_val::base8) {
        delimit_at = 3;
        digit_mask = 7;
        digit_shift = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(radix == calc_val::base16);
        delimit_at = 4;
        digit_mask = 15;
        digit_shift = 4;
    }

    decltype(val) reversed = 0;
    unsigned digit_count = 0;
    for (; val >= radix; ++digit_count) { // all digits except leftmost one
        reversed <<= digit_shift;
        reversed |= val & digit_mask;
        val >>= digit_shift;
    }

    // leftmost digit (or 0) -- leftmost digit may not be "full"; e.g., for
    // octal, 64%3 == 1: 64 is bit width of val and 3 is bit width of octal
    // digit
    assert(val < digits.size());
    out << digits.at(static_cast<size_t>(val));

    // remaining reversed digits
    assert(digit_mask < digits.size());
    for (; digit_count; --digit_count) {
        if (delimit_at && !(digit_count % delimit_at))
            out << ' ';
        out << digits.at(static_cast<size_t>(reversed & digit_mask));
        reversed >>= digit_shift;
    }

    return out;
}

auto calc_outputter::output_as_ieee_fp(std::ostream& out, const pseudo_IEEE_cpp_bin_float& val, calc_val::radices radix) const -> std::ostream& {
    static_assert(ieee_fp_parts<std::decay_t<decltype(val)>>::is_specialized);
    auto val_parts = ieee_fp_parts<std::decay_t<decltype(val)>>(val);

    if (val_parts.is_negative())
        out << '-';

    if (val_parts.is_inf())
        out << "inf";
    else if (val_parts.is_nan())
        out << "nan";
    else {
        unsigned digit_mask;
        unsigned digit_shift;
        if (radix == calc_val::base2) {
            digit_mask = 1;
            digit_shift = 1;
        } else if (radix == calc_val::base8) {
            digit_mask = 7;
            digit_shift = 3;
        } else { // assume base 16; output in other bases is unsupported
            assert(radix == calc_val::base16);
            digit_mask = 15;
            digit_shift = 4;
        }

#if (1) //----------------------------------------------------------------------
// output the number normalized.
// note: hexadecimal floating point format is described here:
// https://www.exploringbinary.com/hexadecimal-floating-point-constants/
// binary and octal floating point format is by extension
        auto significand = val_parts.significand();
        using reversed_t = decltype(significand);
        static_assert(!std::numeric_limits<reversed_t>::is_signed);
        reversed_t reversed = 0;

        auto n_bits = val_parts.significand_bits();
        if (!val_parts.lead_bit_implied())
            --n_bits;

        if (auto odd = n_bits % digit_shift) {
            reversed |= significand & (digit_mask >> odd);
            significand >>= odd;
        }
        for (auto n = n_bits / digit_shift; n; --n) {
            reversed <<= digit_shift;
            reversed |= significand & digit_mask;
            significand >>= digit_shift;
        }

        out << (val_parts.has_lead_bit() ? '1' : '0');
        if (reversed) {
            out << '.';
            do {
                out << digits.at(unsigned(reversed & digit_mask));
                reversed >>= digit_shift;
            } while (reversed);
        }
        out << 'p';
        auto exponent = val_parts.adjusted_exponent();
        if (exponent >= 0)
            out << '+';
        out << std::dec << exponent;
    }

    return out;
#else //------------------------------------------------------------------------
// this code seems to match what gcc is outputting for hexfloat, but outputs a
// different though equivalent result for long double than for double; plus the
// enabled code above is actually simplier and outputs a normalized value
        auto significand = val_parts.significand();
        decltype(significand) reversed = 0;

        if (auto odd = val_parts.significand_bits() % digit_shift) {
            reversed |= significand & (digit_mask >> odd);
            significand >>= odd;
        }
        for (auto n = val_parts.significand_bits() / digit_shift; n; --n) {
            reversed <<= digit_shift;
            reversed |= significand & digit_mask;
            significand >>= digit_shift;
        }

        std::int64_t exponent = val_parts.adjusted_exponent();
        if (val_parts.lead_bit_implied())
            out << '1';
        else {
            out << digits.at(reversed & digit_mask);
            if (reversed) {
                reversed >>= digit_shift;
                exponent -= digit_shift - 1;
            }
        }
        if (reversed) {
            out << '.';
            do {
                out << digits.at(reversed & digit_mask);
                reversed >>= digit_shift;
            } while (reversed);
        }
        out << 'p';
        if (exponent >= 0)
            out << '+';
        out << std::dec << exponent;
#endif //-----------------------------------------------------------------------
}
