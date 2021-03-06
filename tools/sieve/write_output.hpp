//*                _ _                     _               _    *
//* __      ___ __(_) |_ ___    ___  _   _| |_ _ __  _   _| |_  *
//* \ \ /\ / / '__| | __/ _ \  / _ \| | | | __| '_ \| | | | __| *
//*  \ V  V /| |  | | ||  __/ | (_) | |_| | |_| |_) | |_| | |_  *
//*   \_/\_/ |_|  |_|\__\___|  \___/ \__,_|\__| .__/ \__,_|\__| *
//*                                           |_|               *
//===- tools/sieve/write_output.hpp ---------------------------------------===//
// Copyright (c) 2017-2020 by Sony Interactive Entertainment, Inc.
// All rights reserved.
//
// Developed by:
//   Toolchain Team
//   SN Systems, Ltd.
//   www.snsystems.com
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal with the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimers.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimers in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the names of SN Systems Ltd., Sony Interactive Entertainment,
//   Inc. nor the names of its contributors may be used to endorse or
//   promote products derived from this Software without specific prior
//   written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
//===----------------------------------------------------------------------===//
#ifndef WRITE_OUTPUT_HPP
#define WRITE_OUTPUT_HPP

#include <cstdint>
#include <ostream>
#include "switches.hpp"

template <typename IntType, endian EndianNess>
std::uint8_t * write (IntType v, std::uint8_t * out);

template <>
inline std::uint8_t * write<std::uint16_t, endian::big> (std::uint16_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v) &0xFFU);
    return out;
}
template <>
inline std::uint8_t * write<std::uint16_t, endian::little> (std::uint16_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> (v & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    return out;
}
template <>
inline std::uint8_t * write<std::uint16_t, endian::native> (std::uint16_t v, std::uint8_t * out) {
    auto vptr = reinterpret_cast<std::uint8_t const *> (&v);
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    return out;
}


template <>
inline std::uint8_t * write<std::uint32_t, endian::big> (std::uint32_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> ((v >> 24U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 16U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> (v & 0xFFU);
    return out;
}
template <>
inline std::uint8_t * write<std::uint32_t, endian::little> (std::uint32_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> (v & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 16U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 24U) & 0xFFU);
    return out;
}
template <>
inline std::uint8_t * write<std::uint32_t, endian::native> (std::uint32_t v, std::uint8_t * out) {
    auto vptr = reinterpret_cast<std::uint8_t const *> (&v);
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    return out;
}


template <>
inline std::uint8_t * write<std::uint64_t, endian::big> (std::uint64_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> ((v >> 56U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 48U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 40U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 32U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 24U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 16U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> (v & 0xFF);
    return out;
}
template <>
inline std::uint8_t * write<std::uint64_t, endian::little> (std::uint64_t v, std::uint8_t * out) {
    *(out++) = static_cast<std::uint8_t> (v & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 8U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 16U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 24U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 32U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 40U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 48U) & 0xFFU);
    *(out++) = static_cast<std::uint8_t> ((v >> 56U) & 0xFFU);
    return out;
}
template <>
inline std::uint8_t * write<std::uint64_t, endian::native> (std::uint64_t v, std::uint8_t * out) {
    auto vptr = reinterpret_cast<std::uint8_t const *> (&v);
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    *(out++) = static_cast<std::uint8_t> (*(vptr++));
    return out;
}

template <typename ContainerType, typename GetOutputStreamFn>
void write_output (ContainerType const & primes, endian output_endian, GetOutputStreamFn out) {

    using int_type = typename ContainerType::value_type;
    std::vector<std::uint8_t> bytes;
    bytes.resize (primes.size () * sizeof (int_type));
    auto ptr = bytes.data ();
    switch (output_endian) {
    case endian::big:
        for (auto v : primes) {
            ptr = write<int_type, endian::big> (v, ptr);
        }
        break;
    case endian::little:
        for (auto v : primes) {
            ptr = write<int_type, endian::little> (v, ptr);
        }
        break;
    case endian::native:
        for (auto v : primes) {
            ptr = write<int_type, endian::native> (v, ptr);
        }
        break;
    }
    out ().write (reinterpret_cast<std::ofstream::char_type const *> (bytes.data ()),
                  static_cast<std::streamsize> (sizeof (std::uint8_t) * bytes.size ()));
}

#endif // WRITE_OUTPUT_HPP
