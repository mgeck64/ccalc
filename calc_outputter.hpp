#ifndef CALC_OUTPUTTER_HPP
#define CALC_OUTPUTTER_HPP

#include "variant.hpp"
#include "calc_args.hpp"
#include <ostream>
#include <array>

class calc_outputter {
public:
    calc_outputter(const output_options& out_options_)
        : out_options{out_options_}, output_fn{output_fn_for(out_options.output_radix)} {}
    calc_outputter() = default;
    calc_outputter(const calc_outputter&) = default;

    const calc_outputter& operator()(const calc_val::variant_type& val_) {val = val_; return *this;}
    friend std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter);

private:
    calc_val::variant_type val = calc_val::complex_type{};
    output_options out_options{};

    using output_fn_type = auto (calc_outputter::*)(std::ostream& out) const -> std::ostream&;
    output_fn_type output_fn = &calc_outputter::output_dec; // (note: auto not allowed for non-static members!)
    static auto output_fn_for(calc_val::radices radix) -> output_fn_type;

    auto output_dec(std::ostream& out) const -> std::ostream&;
    auto output_pow2(std::ostream& out) const -> std::ostream&;
    auto output_pow2_as_uint(std::ostream& out, std::uintmax_t val) const -> std::ostream&;
    auto output_pow2_as_floating_point(std::ostream& out, const calc_val::float_type& val) const -> std::ostream&;

    static constexpr auto digits = std::array{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
};

#endif // CALC_OUTPUTTER_HPP
