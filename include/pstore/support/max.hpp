//*                        *
//*  _ __ ___   __ ___  __ *
//* | '_ ` _ \ / _` \ \/ / *
//* | | | | | | (_| |>  <  *
//* |_| |_| |_|\__,_/_/\_\ *
//*                        *
//===- include/pstore/support/max.hpp -------------------------------------===//
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
/// \brief A template implementation of std::max() with helper types for determining the maximum
/// size and alignment of a collection of types.
///
/// In C++14, std::max() becomes a constexpr function which enables its use at compile time, and
/// more particularly allows for its use in template arguments. This library uses it as an argument
/// to std::aligned_storage<> so that we can produce a block of storage into which an instance of a
/// number of types could be constructed.
#ifndef PSTORE_SUPPORT_MAX_HPP
#define PSTORE_SUPPORT_MAX_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace pstore {

    // max
    // ~~~
    /// In C++14 std::max() is a constexpr function so we could use it in these templates,
    /// but our code needs to be compatible with C++11.
    template <typename T, T Value1, T Value2>
    struct max : std::integral_constant<T, (Value1 < Value2) ? Value2 : Value1> {};

    // max_sizeof
    // ~~~~~~~~~~
    template <typename... T>
    struct max_sizeof;
    template <>
    struct max_sizeof<> : std::integral_constant<std::size_t, 1U> {};
    template <typename Head, typename... Tail>
    struct max_sizeof<Head, Tail...>
            : std::integral_constant<
                  std::size_t, max<std::size_t, sizeof (Head), max_sizeof<Tail...>::value>::value> {
    };

    // max_alignment
    // ~~~~~~~~~~~~~
    template <typename... Types>
    struct max_alignment;
    template <>
    struct max_alignment<> : std::integral_constant<std::size_t, 1U> {};
    template <typename Head, typename... Tail>
    struct max_alignment<Head, Tail...>
            : std::integral_constant<std::size_t, max<std::size_t, alignof (Head),
                                                      max_alignment<Tail...>::value>::value> {};

    // characteristics
    // ~~~~~~~~~~~~~~~
    /// Given a list of types, find the size of the largest and the alignment of the most aligned.
    template <typename... T>
    struct characteristics {
        static constexpr std::size_t size = max_sizeof<T...>::value;
        static constexpr std::size_t align = max_alignment<T...>::value;
    };

} // end namespace pstore

static_assert (pstore::max<int, 1, 2>::value == 2, "max(1,2)!=2");
static_assert (pstore::max<int, 2, 1>::value == 2, "max(2,1)!=2");
static_assert (pstore::max_sizeof<std::uint_least8_t>::value == sizeof (std::uint_least8_t),
               "max(sizeof(8))!=sizeof(8)");
static_assert (pstore::max_sizeof<std::uint_least8_t, std::uint_least16_t>::value >=
                   sizeof (std::uint_least16_t),
               "max(sizeof(8),sizeof(16)!=4");

#endif // PSTORE_SUPPORT_MAX_HPP
