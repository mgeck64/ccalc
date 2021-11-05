#ifndef IEEE_FP_PARTS_HPP
#define IEEE_FP_PARTS_HPP

#include <cstdint>
#include <limits>
#include <cstring>
#include <cassert>
#include <boost/multiprecision/cpp_bin_float.hpp>

template <typename T>
class ieee_fp_parts {
// breaks up IEEE 794 floating point number into component parts
public:
    static constexpr auto is_specialized = false;

    constexpr ieee_fp_parts() noexcept = default;

    constexpr auto is_negative() const noexcept -> bool;
    constexpr auto exponent() const noexcept -> std::uint16_t;

    constexpr auto adjusted_exponent() const noexcept -> std::int16_t;
    // signed (unbiased) exponent(); accounts for special values

    static constexpr auto normalizes() noexcept -> bool;

    constexpr auto has_int_bit() const noexcept -> bool;
    // significand has leading integer 1 bit (may be implied or actual)

    constexpr auto significand() const noexcept -> std::uint64_t;
    // excludes leading integer bit if implied; includes if actual

    static constexpr auto significand_bits() noexcept -> std::size_t;
    // # of bits in significand

    constexpr auto fraction() const noexcept -> std::uint64_t;
    // fraction part (same as significand if leading integer bit is implied)

    static constexpr auto fraction_bits() noexcept -> std::size_t;
    // # of bits in fraction

    constexpr auto is_inf() const noexcept -> bool;
    constexpr auto is_nan() const noexcept -> bool;
    constexpr auto is_zero() const noexcept -> bool;
};

//------------------------------------------------------------------------------

// validate double is IEEE 794 double precision floating point
using ieee_double = double;
static_assert(std::numeric_limits<ieee_double>::is_iec559); // IEEE 794 (IEEE floating point)
static_assert(std::numeric_limits<ieee_double>::digits == 53); // mantissa
static_assert(sizeof(ieee_double) == sizeof(std::uint64_t));

template<>
class ieee_fp_parts<ieee_double> {
// a description of the IEEE 794 double precision floating point format is here:
// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
private:
    std::uint64_t i = 0;
    static constexpr auto sign_mask     = std::uint64_t(0x8000000000000000L);
    static constexpr auto exponent_mask = std::uint64_t(0x7ff0000000000000L);
    static constexpr auto fraction_mask = std::uint64_t(0x000fffffffffffffL);

public:
    static constexpr auto is_specialized = true;

    constexpr explicit ieee_fp_parts(ieee_double d) noexcept
    {std::memcpy(&i, &d, sizeof(i));} // presumably the proper way to do type punning in c++

    constexpr auto is_negative() const noexcept -> bool
    {return i & sign_mask;}

    constexpr auto exponent() const noexcept -> std::uint16_t
    {return (i & exponent_mask) >> 52;}

    constexpr auto adjusted_exponent() const noexcept -> std::int16_t {
        if ((i & exponent_mask) == 0) // 0 or subnormal
            return (i & fraction_mask) == 0 ? 0 : exponent() - 1022;
        return exponent() - 1023;
    }

    static constexpr auto normalizes() noexcept -> bool
    {return true;}

    constexpr auto has_int_bit() const noexcept -> bool
    {return (i & exponent_mask) != 0 && (i & exponent_mask) != exponent_mask;}

    constexpr auto significand() const noexcept -> std::uint64_t
    {return i & fraction_mask;}

    static constexpr auto significand_bits() noexcept -> std::size_t
    {return 52;}

    constexpr auto fraction() const noexcept -> std::uint64_t
    {return i & fraction_mask;}

    static constexpr auto fraction_bits() noexcept -> std::size_t
    {return 52;}

    constexpr auto is_inf() const noexcept -> bool
    {return (i & (exponent_mask | fraction_mask)) == exponent_mask;}

    constexpr auto is_nan() const noexcept -> bool
    {return (i & (exponent_mask | fraction_mask)) > exponent_mask;}

    constexpr auto is_zero() const noexcept -> bool
    {return (i & (exponent_mask | fraction_mask)) == 0;}
};

//------------------------------------------------------------------------------

// validate long double is probably x86 extended precision floating point
using x86_ext_double = long double;
static_assert(std::numeric_limits<x86_ext_double>::is_iec559); // IEEE 794 (IEEE floating point)
static_assert(std::numeric_limits<x86_ext_double>::digits == 64); // mantissa
static_assert(sizeof(x86_ext_double) == sizeof(std::uint64_t) * 2);

template<>
class ieee_fp_parts<x86_ext_double> {
// a description of the x86 extended precision floating point format is here:
// https://docs.oracle.com/cd/E19957-01/806-3568/ncg_math.html
private:
    std::uint16_t hi = 0; // exponent
    std::uint64_t lo = 0; // significand
    static constexpr auto sign_mask     = std::uint16_t(0x8000);
    static constexpr auto exponent_mask = std::uint16_t(0x7fff);
    static constexpr auto fraction_mask = std::uint64_t(0x7fffffffffffffffL);

public:
    static constexpr auto is_specialized = true;

    constexpr explicit ieee_fp_parts(x86_ext_double xd) noexcept {
        // presumably the proper way to do type punning in c++
        auto p = reinterpret_cast<std::uint64_t*>(&xd);
        memcpy(&lo, p, sizeof(lo));
        memcpy(&hi, p + 1, sizeof(hi));
    }

    constexpr auto is_negative() const noexcept -> bool
    {return hi & sign_mask;}

    constexpr auto exponent() const noexcept -> std::uint16_t // exponent right shifted 52 bits
    {return hi & exponent_mask;}

    constexpr auto adjusted_exponent() const noexcept -> std::int16_t { // signed exponent() adjusted if number is zero or subnormal
        auto exponent = hi & exponent_mask;
        if (exponent == 0) // 0 or subnormal
            return (lo & fraction_mask) == 0 ? 0 : exponent - 16382;
        return exponent - 16383;
    }

    static constexpr auto normalizes() noexcept -> bool
    {return false;}

    constexpr auto has_int_bit() const noexcept -> bool
    {return lo & ~fraction_mask;}

    constexpr auto significand() const noexcept -> std::uint64_t
    {return lo;}

    static constexpr auto significand_bits() noexcept -> std::size_t
    {return 64;}

    constexpr auto fraction() const noexcept -> std::uint64_t
    {return lo & fraction_mask;}

    static constexpr auto fraction_bits() noexcept -> std::size_t
    {return 63;}

    constexpr auto is_inf() const noexcept -> bool
    {return (hi & exponent_mask) == exponent_mask && lo == ~fraction_mask;}

    constexpr auto is_nan() const noexcept -> bool
    {return (hi & exponent_mask) == exponent_mask && lo > ~fraction_mask;}

    constexpr auto is_zero() const noexcept -> bool
    {return (hi & exponent_mask) == 0 && lo == 0;}
};

//------------------------------------------------------------------------------

using pseudo_IEEE_cpp_bin_float = boost::multiprecision::cpp_bin_float_50;

template<>
class ieee_fp_parts<pseudo_IEEE_cpp_bin_float> {
// not officially an IEEE format but is compatable enough that this can be
// adapted: logical model is signed magnitude just like IEEE logical model
    using significand_type = boost::multiprecision::number<pseudo_IEEE_cpp_bin_float::backend_type::rep_type>;
    using exponent_type = pseudo_IEEE_cpp_bin_float::backend_type::exponent_type;

private:
    bool sign;
    significand_type significand_;
    exponent_type exponent_;
    static constexpr auto hi_bit = significand_type(1) << (pseudo_IEEE_cpp_bin_float::backend_type::bit_count - 1);
    static constexpr auto frac_mask = ~hi_bit;
    static constexpr auto significand_0 = significand_type(0);

public:
    static constexpr auto is_specialized = true;

    explicit ieee_fp_parts(const pseudo_IEEE_cpp_bin_float& n) noexcept
        : sign{n.backend().sign()}, significand_{n.backend().bits()},
          exponent_{n.backend().exponent()} {}

    constexpr auto is_negative() const noexcept -> bool
    {return sign;}

    constexpr auto exponent() const noexcept -> const exponent_type&
    {return exponent_;}

    constexpr auto adjusted_exponent() const noexcept -> const exponent_type&
    {assert(!is_inf() && !is_nan() && !is_zero()); return exponent_;}

    static constexpr auto normalizes() noexcept -> bool
    {return false;}

    constexpr auto has_int_bit() const noexcept -> bool
    {return (significand_ & hi_bit) != significand_0;}

    constexpr auto significand() const noexcept -> const significand_type&
    {return significand_;}

    static constexpr auto significand_bits() noexcept -> auto
    {return pseudo_IEEE_cpp_bin_float::backend_type::bit_count;}

    constexpr auto fraction() const noexcept -> significand_type
    {return significand_ & frac_mask;}

    static constexpr auto fraction_bits() noexcept -> auto
    {return significand_bits() - 1;}

    constexpr auto is_inf() const noexcept -> bool
    {return exponent_ == pseudo_IEEE_cpp_bin_float::backend_type::exponent_infinity;}

    constexpr auto is_nan() const noexcept -> bool
    {return exponent_ == pseudo_IEEE_cpp_bin_float::backend_type::exponent_nan;}

    constexpr auto is_zero() const noexcept -> bool
    {return exponent_ == pseudo_IEEE_cpp_bin_float::backend_type::exponent_zero;}
};

#endif // IEEE_FP_PARTS_HPP
