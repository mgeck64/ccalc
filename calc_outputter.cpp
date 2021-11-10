#include "calc_outputter.hpp"
#include <boost/io/ios_state.hpp>
#include <limits>
#include <cassert>

std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter) {
    boost::io::ios_all_saver guard(out);
    out.unsetf(std::ios::basefield | std::ios::adjustfield | std::ios::floatfield); // put in known default state
    (outputter.*outputter.output_fn)(out);
    std::visit([&](const auto& val) {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT> && std::is_signed_v<VT>)
            out << " (int";
        else if constexpr (std::is_integral_v<VT>)
            out << " (uint";
        else if constexpr (std::is_same_v<VT, calc_val::complex_type>)
            out << " (cplx";
        out << " base" << outputter.out_options.output_radix << ')';
    }, outputter.val);
    return out;
}

auto calc_outputter::output_fn_for(calc_val::radices radix) -> output_fn_type {
    switch (radix) {
        case calc_val::base10:
            return &calc_outputter::output_dec;
        case calc_val::base2:
        case calc_val::base8:
        case calc_val::base16:
            return &calc_outputter::output_pow2;
        default:
            assert(false); // missed one
            return &calc_outputter::output_dec;
    }
}

auto calc_outputter::output_dec(std::ostream& out) const -> std::ostream& {
    if (out_options.output_fixed_fp) {
        out.setf(std::ios_base::fixed);
        out.precision(out_options.precision);
    } else if (out_options.precision == 0
            || out_options.precision > std::numeric_limits<calc_val::float_type>::max_digits10)
        out.precision(std::numeric_limits<calc_val::float_type>::max_digits10);
    else
        out.precision(out_options.precision);

    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT>)
            out << +val; // +val: incase val is char type, will output as int
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            if (val.real() != 0 || val.imag() == 0)
                out << val.real();
            if (val.imag() != 0) {
                if (val.real() != 0 && !signbit(val.imag()))
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

auto calc_outputter::output_pow2(std::ostream& out) const -> std::ostream& {
    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT> && std::is_signed_v<VT>) {
            output_pow2_as_uint(out, [&]() -> std::make_unsigned_t<VT> {
                if (val < 0) {
                    out << '-';
                    return -val;
                }
                return val;
            }());
        } else if constexpr (std::is_integral_v<VT>)
            output_pow2_as_uint(out, val);
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            if (val.real() != 0 || val.imag() == 0)
                output_pow2_as_floating_point(out, val.real());
            if (val.imag() != 0) {
                if (val.real() != 0 && !signbit(val.imag()))
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    output_pow2_as_floating_point(out, val.imag());
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output_pow2_as_uint(std::ostream& out, std::uintmax_t val) const -> std::ostream& {
    unsigned delimit_at;
    decltype(val) digit_mask;
    size_t digit_n_bits;
    if (out_options.output_radix == calc_val::base2) {
        delimit_at = 4;
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (out_options.output_radix == calc_val::base8) {
        delimit_at = 3;
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(out_options.output_radix == calc_val::base16);
        delimit_at = 4;
        digit_mask = 15;
        digit_n_bits = 4;
    }

    decltype(val) reversed = 0;
    unsigned digit_count = 0;
    for (; val >= out_options.output_radix; ++digit_count) { // all digits except leftmost one
        reversed <<= digit_n_bits;
        reversed |= val & digit_mask;
        val >>= digit_n_bits;
    }

    // leftmost digit (or 0) -- leftmost digit may be partial; e.g., for octal,
    // 64%3 == 1: 64 is bit width of val and 3 is bit width of octal digit
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

auto calc_outputter::output_pow2_as_floating_point(std::ostream& out, const calc_val::float_type& val) const -> std::ostream& {
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
    if (out_options.output_radix == calc_val::base2) {
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (out_options.output_radix == calc_val::base8) {
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(out_options.output_radix == calc_val::base16);
        digit_mask = 15;
        digit_n_bits = 4;
    }

    using significand_type = boost::multiprecision::number<calc_val::float_type::backend_type::rep_type>;
    assert(std::numeric_limits<significand_type>::digits >= digit_n_bits);
    auto significand = significand_type();
    assert(significand == 0);
    auto exponent = val.backend().exponent();

    static_assert(!std::numeric_limits<decltype(out_options.precision)>::is_signed);
    if (out_options.precision == 0
            || out_options.precision * digit_n_bits > std::numeric_limits<significand_type>::digits)
        significand = val.backend().bits();
    else { // handle rounding to precision
        auto f = calc_val::float_type();
        f.backend().bits() = val.backend().bits();
        f.backend().exponent() = 0;

        if (out_options.output_fp_normalized)
            f.backend().exponent() -= digit_n_bits;
        else
            f.backend().exponent() = f.backend().exponent() - (digit_n_bits - exponent % digit_n_bits);

        f.backend().exponent() += out_options.precision * digit_n_bits;
        auto e0 = f.backend().exponent();
        f += calc_val::float_type(out_options.output_radix / 2) / calc_val::float_type(unsigned(out_options.output_radix));
        f = trunc(f);
        significand = f.backend().bits();
        assert(!iszero(f) && !isinf(f) && !isnan(f));
        exponent += f.backend().exponent() - e0;
    }

    static_assert(!std::numeric_limits<significand_type>::is_signed);
    using reversed_type = boost::multiprecision::number<calc_val::float_type::backend_type::double_rep_type>;
    reversed_type reversed = 0;
    auto n_bits = std::numeric_limits<significand_type>::digits - 1; // exclude leading bit, which is handled specially

    if (!out_options.output_fp_normalized) {
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
    out << 'p' << std::dec;
    if (exponent >= 0)
        out << '+';
    if (out_options.output_fp_normalized)
        out << exponent;
    else {
        assert(exponent % digit_n_bits == 0);
        out << (exponent / digit_n_bits);
    }

    return out;
}
