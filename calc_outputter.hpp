#ifndef CALC_OUTPUTTER_HPP
#define CALC_OUTPUTTER_HPP

#include "variant.hpp"
#include "calc_args.hpp"
#include <ostream>
#include <array>

class calc_outputter {
public:
    calc_outputter(calc_val::radices radix_, decltype(calc_args::precision) precision10_, decltype(calc_args::output_fp_normalized) output_fp_normalized_)
        : radix{radix_}, precision{precision10_}, output_fp_normalized{output_fp_normalized_}, output_fn{output_fn_for(radix)} {}
    calc_outputter() = default;
    calc_outputter(const calc_outputter&) = default;

    const calc_outputter& operator()(const calc_val::variant_type& val_) {val = val_; return *this;}
    friend std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter);

private:
    calc_val::variant_type val = calc_val::complex_type{};
    calc_val::radices radix = calc_val::base10;
    decltype(calc_args::precision) precision = std::numeric_limits<calc_val::float_type>::digits10;
    bool output_fp_normalized = true;

    auto output_bin(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream&;
    auto output_oct(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream&;
    auto output_dec(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream&;
    auto output_hex(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream&;

    using output_fn_type = auto (calc_outputter::*)(std::ostream& out, const calc_val::variant_type& val) const -> std::ostream&;
    output_fn_type output_fn = &calc_outputter::output_dec; // (note: auto not allowed for non-static members!)
    static auto output_fn_for(calc_val::radices radix) -> output_fn_type;

    auto output(std::ostream& out, const calc_val::variant_type& val, calc_val::radices radix) const -> std::ostream&;
    auto output_as_uint(std::ostream& out, std::uintmax_t val, calc_val::radices radix) const -> std::ostream&;
    auto output_as_floating_point(std::ostream& out, const calc_val::float_type& val, calc_val::radices radix) const -> std::ostream&;

    static constexpr auto digits = std::array{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
};

#endif // CALC_OUTPUTTER_HPP
