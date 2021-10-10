#pragma once
#ifndef COMPLEX_TYPE_H
#define COMPLEX_TYPE_H

#include "basics.h"

// std::complex's pow function is giving annoying rouding/precision errors;
// e.g., pow(std::complex<double>(0, 1), 2.0) is yielding
// (-1,1.22464679914735e-16), not (-1,0). an early attempt at a solution was to
// replace std::complex with another complex number class i found but i got the
// same results. i found a solution that seems to be working well, which is to
// use a different algorithm for integer value exponents in pow
//
// addendum: the other complex number class is yielding (-nan, -nan) instead of
// complex infinity (inf, -nan) for 1/0. i am switching back to using
// std::complex because i'm not up for debugging the class and std::complex
// presumably handles such edge cases properly, but i am still using custom pow
// functions to handle integer value exponents



//#define USE_ALTERNATIVE_COMPLEX_TYPE
// define to use the other complex number class. undefine to use std::complex



#ifdef USE_ALTERNATIVE_COMPLEX_TYPE // else use std::complex -------------------

/*
this is a modified version of Michael F. Hutt's complex number class from
https://github.com/huttmf/complexlib

the following notice is provided as required:

The MIT License (MIT)

Copyright (c) 1999 Michael F. Hutt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>

namespace calc_val {

namespace complex_common = calc_val; // = std if using std::complex below

class complex_type {
// this is intended to be a drop-in replacement for std::complex<double> but
// with some exceptions: complex_type has some functionality that
// std::complex doesn't, and not all of std::complex's functionality has been
// implemented

public:
    using value_type = float_type;

    constexpr complex_type(const value_type& real = 0, const value_type& imag = 0);
    complex_type(const complex_type& z) = default;

    auto real() const -> value_type;
    void real(const complex_type& z);
    void real(const value_type& real);

    auto imag() const -> value_type;
    void imag(const complex_type& z);
    void imag(const value_type& imag);

    auto operator=(const complex_type& z) -> complex_type& = default;
    auto operator+=(const complex_type& z) -> complex_type&;
    auto operator-=(const complex_type& z) -> complex_type&;
    auto operator*=(const complex_type& z) -> complex_type&;
    auto operator/=(const complex_type& z) -> complex_type&;

private:
    value_type real_;
    value_type imag_;
};

auto real(const complex_type& z) -> complex_type::value_type;
auto imag(const complex_type& z) -> complex_type::value_type;

auto conj(const complex_type& z) -> complex_type;
auto abs(const complex_type& z) -> complex_type::value_type;
auto arg(const complex_type& z) -> complex_type::value_type;
auto norm(const complex_type& z) -> complex_type::value_type;
auto polar(const complex_type::value_type& r, const complex_type::value_type& t = complex_type::value_type()) -> complex_type;
auto proj(const complex_type& z) -> complex_type;

auto operator==(const complex_type& z1, const complex_type& z2) -> bool;
auto operator!=(const complex_type& z1, const complex_type& z2) -> bool;

auto operator+(const complex_type& z) -> complex_type;
auto operator+(const complex_type& z1, const complex_type& z2) -> complex_type;
auto operator+(const complex_type::value_type& a, const complex_type& z) -> complex_type;
auto operator+(const complex_type& z1, complex_type::value_type& a) -> complex_type;

auto operator-(const complex_type& z) -> complex_type;
auto operator-(const complex_type& z1, const complex_type& z2) -> complex_type;
auto operator-(complex_type::value_type& a, const complex_type& z) -> complex_type;
auto operator-(const complex_type& z, const complex_type::value_type& a) -> complex_type;

auto operator*(const complex_type& z1, const complex_type& z2) -> complex_type;
auto operator*(const complex_type::value_type& a, const complex_type& z) -> complex_type;
auto operator*(const complex_type& z, const complex_type::value_type& a) -> complex_type;

auto operator/(const complex_type& z1, const complex_type& z2) -> complex_type;
auto operator/(const complex_type::value_type& a, const complex_type& z) -> complex_type;
auto operator/(const complex_type& z, const complex_type::value_type& a) -> complex_type;

auto sqrt(const complex_type& z) -> complex_type;
auto log(const complex_type& z) -> complex_type;
auto log10(const complex_type& z) -> complex_type;
auto exp(const complex_type& z) -> complex_type;

auto sin(const complex_type& z) -> complex_type;
auto cos(const complex_type& z) -> complex_type;
auto tan(const complex_type& z) -> complex_type;
auto sec(const complex_type& z) -> complex_type;
auto csc(const complex_type& z) -> complex_type;
auto cot(const complex_type& z) -> complex_type;

auto sinh(const complex_type& z) -> complex_type;
auto cosh(const complex_type& z) -> complex_type;
auto tanh(const complex_type& z) -> complex_type;
auto sech(const complex_type& z) -> complex_type;
auto csch(const complex_type& z) -> complex_type;
auto coth(const complex_type& z) -> complex_type;

auto asin(const complex_type& z) -> complex_type;
auto acos(const complex_type& z) -> complex_type;
auto atan(const complex_type& z) -> complex_type;
auto asinh(const complex_type& z) -> complex_type;
auto acosh(const complex_type& z) -> complex_type;
auto atanh(const complex_type& z) -> complex_type;

std::ostream& operator<<(std::ostream& stream, const complex_type& z);

} // namespace calc_val



#else // use std::complex-------------------------------------------------------



#include <complex>

namespace calc_val {

using complex_type = std::complex<float_type>;
namespace complex_common = std; // = calc_val if using alternative class above

} // namespace calc_val

// std::complex does not support mixed integer/complex arithmetic, thus:

template <typename Int>
inline auto operator+(const calc_val::complex_type& z, const Int &a) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return z + calc_val::complex_type::value_type(a);}

template <typename Int>
inline auto operator+(const Int& a, const calc_val::complex_type& z) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return calc_val::complex_type::value_type(a) + z;}

template <typename Int>
inline auto operator-(const calc_val::complex_type& z, const Int &a) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return z - calc_val::complex_type::value_type(a);}

template <typename Int>
inline auto operator-(const Int& a, const calc_val::complex_type& z) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return calc_val::complex_type::value_type(a) - z;}

template <typename Int>
inline auto operator*(const calc_val::complex_type& z, const Int &a) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return z * calc_val::complex_type::value_type(a);}

template <typename Int>
inline auto operator*(const Int& a, const calc_val::complex_type& z) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return calc_val::complex_type::value_type(a) * z;}

template <typename Int>
inline auto operator/(const calc_val::complex_type& z, const Int &a) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return z / calc_val::complex_type::value_type(a);}

template <typename Int>
inline auto operator/(const Int& a, const calc_val::complex_type& z) ->
std::enable_if_t<std::is_integral_v<Int>, calc_val::complex_type>
{return calc_val::complex_type::value_type(a) / z;}

#endif // USE_ALTERNATIVE_COMPLEX_TYPE------------------------------------------



namespace calc_val {

auto pow(const complex_type& z, const complex_type& e) -> complex_type;

template <typename T>
auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_unsigned_v<T>, complex_type>;

template <typename T>
auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_signed_v<T>, complex_type>;



// need wrappers for the following functions so they match calc_parser::unary_fn:

auto arg_wrapper(const complex_type& z) -> complex_type;
auto norm_wrapper(const complex_type& z) -> complex_type;

} // namespace calc_val



#include "complex_type_defs.h"

#endif // COMPLEX_TYPE_H
