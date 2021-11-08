#include "calc_outputter.hpp"
#include "stream_state_restorer.hpp"
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
    if (precision == 0 || precision > std::numeric_limits<calc_val::float_type>::max_digits10)
        out.precision(std::numeric_limits<calc_val::float_type>::max_digits10);
    else
        out.precision(precision);
    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT>)
            out << std::dec << +val; // +val: incase val is char type, will output as int
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
                output_as_floating_point(out, val.real(), radix);
            if (val.imag() != 0) {
                if (val.real() != 0 && !val.imag().backend().sign())
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    output_as_floating_point(out, val.imag(), radix);
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output_as_uint(std::ostream& out, std::uintmax_t val, calc_val::radices radix) const -> std::ostream& {
    unsigned delimit_at;
    decltype(val) digit_mask;
    size_t digit_n_bits;
    if (radix == calc_val::base2) {
        delimit_at = 4;
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (radix == calc_val::base8) {
        delimit_at = 3;
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(radix == calc_val::base16);
        delimit_at = 4;
        digit_mask = 15;
        digit_n_bits = 4;
    }

    decltype(val) reversed = 0;
    unsigned digit_count = 0;
    for (; val >= radix; ++digit_count) { // all digits except leftmost one
        reversed <<= digit_n_bits;
        reversed |= val & digit_mask;
        val >>= digit_n_bits;
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
        reversed >>= digit_n_bits;
    }

    return out;
}

auto calc_outputter::output_as_floating_point(std::ostream& out, const calc_val::float_type& val, calc_val::radices radix) const -> std::ostream& {
    if (signbit(val))
        out << '-';

    if (isinf(val))
        return out << "inf";
    if (isnan(val))
        return out << "nan";
    if (iszero(val)) // can't be handled in general routine
        return out << "0";

    unsigned digit_mask;
    unsigned digit_n_bits;
    if (radix == calc_val::base2) {
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (radix == calc_val::base8) {
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(radix == calc_val::base16);
        digit_mask = 15;
        digit_n_bits = 4;
    }

    using significand_type = boost::multiprecision::number<calc_val::float_type::backend_type::rep_type>;
    assert(std::numeric_limits<significand_type>::digits >= digit_n_bits);
    auto significand = significand_type();
    assert(significand == 0);
    auto exponent = val.backend().exponent();

    static_assert(!std::numeric_limits<decltype(precision)>::is_signed);
    if (precision == 0 || precision * digit_n_bits > std::numeric_limits<significand_type>::digits)
        significand = val.backend().bits();
    else { // handle rounding to precision
        auto f = calc_val::float_type();
        f.backend().bits() = val.backend().bits();
        f.backend().exponent() = 0;

        if (output_fp_normalized)
            f.backend().exponent() -= digit_n_bits;
        else
            f.backend().exponent() = f.backend().exponent() - (digit_n_bits - exponent % digit_n_bits);

        f.backend().exponent() += precision * digit_n_bits;
        auto e0 = f.backend().exponent();
        f += calc_val::float_type(radix / 2) / calc_val::float_type(unsigned(radix));
        f = trunc(f);
        significand = f.backend().bits();
        assert(!iszero(f) && !isinf(f) && !isnan(f));
        exponent += f.backend().exponent() - e0;
    }

    static_assert(!std::numeric_limits<significand_type>::is_signed);
    using reversed_type = boost::multiprecision::number<calc_val::float_type::backend_type::double_rep_type>;
    reversed_type reversed = 0;
    auto n_bits = std::numeric_limits<significand_type>::digits - 1; // exclude leading bit, which is handled specially

    if (!output_fp_normalized) {
        unsigned adjustment = exponent % digit_n_bits;
        n_bits -= adjustment;
        exponent -= adjustment;
    }
    if (auto shift = n_bits % digit_n_bits) { // partial digit
        auto shift2 = digit_n_bits - shift;
        reversed = unsigned((significand & (digit_mask >> shift2)) << shift2);
        significand >>= shift;
    }
    for (auto n = n_bits / digit_n_bits; n; --n) {
        reversed <<= digit_n_bits;
        reversed |= significand & digit_mask;
        significand >>= digit_n_bits;
    }

    out << digits.at(unsigned(significand));
    if (reversed) {
        out << '.';
        do {
            out << digits.at(unsigned(reversed & digit_mask));
            reversed >>= digit_n_bits;
        } while (reversed);
    }
    out << 'p';
    if (exponent >= 0)
        out << '+';
    out << std::dec << exponent;

    return out;
}
