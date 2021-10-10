#pragma once
#ifndef CALC_OUTPUTTER_H
#define CALC_OUTPUTTER_H

#include "variant.h"
#include "ieee_fp_parts.h"
#include <ostream>
#include <array>

class calc_outputter {
public:
    calc_outputter(calc_val::radices radix) : radix_{radix}, output_fn{output_fn_for(radix)} {}
    calc_outputter() = default;
    calc_outputter(const calc_outputter&) = default;

    const calc_outputter& operator()(const calc_val::variant_type& val) {val_ = val; return *this;}
    friend std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter);

private:
    calc_val::variant_type val_ = calc_val::complex_type{};
    calc_val::radices radix_ = calc_val::base10;

    static auto output_bin(std::ostream& out, const calc_val::variant_type& val) -> std::ostream&;
    static auto output_oct(std::ostream& out, const calc_val::variant_type& val) -> std::ostream&;
    static auto output_dec(std::ostream& out, const calc_val::variant_type& val) -> std::ostream&;
    static auto output_hex(std::ostream& out, const calc_val::variant_type& val) -> std::ostream&;

    using output_fn_type = auto (*)(std::ostream& out, const calc_val::variant_type& val) -> std::ostream&;
    output_fn_type output_fn = output_dec; // (note: auto not allowed for non-static members!)
    static auto output_fn_for(calc_val::radices radix) -> output_fn_type;

    static auto output(std::ostream& out, const calc_val::variant_type& val, calc_val::radices radix) -> std::ostream&;
    static auto output_as_uint(std::ostream& out, std::uintmax_t val, calc_val::radices radix) -> std::ostream&;
    static auto output_as_ieee_fp(std::ostream& out, calc_val::float_type val, calc_val::radices radix) -> std::ostream&;

    static constexpr auto digits = std::array{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
};

#endif // CALC_OUTPUTTER_H