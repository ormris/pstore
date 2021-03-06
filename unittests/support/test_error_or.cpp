//*                                         *
//*   ___ _ __ _ __ ___  _ __    ___  _ __  *
//*  / _ \ '__| '__/ _ \| '__|  / _ \| '__| *
//* |  __/ |  | | | (_) | |    | (_) | |    *
//*  \___|_|  |_|  \___/|_|     \___/|_|    *
//*                                         *
//===- unittests/support/test_error_or.cpp --------------------------------===//
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
#include "pstore/support/error_or.hpp"

#include <stdexcept>
#include <system_error>

#include <gmock/gmock.h>

TEST (ErrorOr, ErrorCodeCtor) {
    std::error_code const err = std::make_error_code (std::errc::address_family_not_supported);
    pstore::error_or<int> const eo{err};
    EXPECT_FALSE (static_cast<bool> (eo));
    EXPECT_EQ (eo.get_error (), err);
}

TEST (ErrorOr, ErrorEnumCtor) {
    constexpr auto err = std::errc::address_family_not_supported;
    pstore::error_or<int> const eo{err};
    EXPECT_FALSE (static_cast<bool> (eo));
    EXPECT_EQ (eo.get_error (), std::make_error_code (err));
}

TEST (ErrorOr, ValueCtor) {
    pstore::error_or<long> const eo{17};
    EXPECT_TRUE (static_cast<bool> (eo));
    EXPECT_EQ (eo.get (), 17L);
    EXPECT_EQ (*eo, 17L);
}

TEST (ErrorOr, InPlaceCtor) {
    pstore::error_or<std::pair<int, int>> eo{pstore::in_place, 17, 23};
    EXPECT_TRUE (static_cast<bool> (eo));
    EXPECT_EQ (*eo, std::make_pair (17, 23));
    EXPECT_EQ (eo->first, 17);

    auto eo2 = eo;
    EXPECT_EQ (eo2, *eo);
}

TEST (ErrorOr, ErrorAssign) {
    constexpr auto err = std::errc::address_family_not_supported;

    pstore::error_or<std::pair<int, int>> eo{pstore::in_place, 17, 23};
    eo = err;

    EXPECT_FALSE (static_cast<bool> (eo));
    EXPECT_EQ (eo.get_error (), std::make_error_code (err));
}

namespace {

    class copy_only {
    public:
        explicit copy_only (int v) noexcept
                : v_{v} {}
        copy_only (copy_only const &) noexcept = default;
        copy_only (copy_only &&) noexcept = delete;
        copy_only & operator= (copy_only const &) = default;
        copy_only & operator= (copy_only &&) = delete;

        int get () noexcept { return v_; }

    private:
        int v_;
    };

} // end anonymous namespace

TEST (ErrorOr, CopyAssign) {
    pstore::error_or<copy_only> eo1{pstore::in_place, 1};
    EXPECT_EQ (eo1->get (), 1);
    pstore::error_or<copy_only> eo2{pstore::in_place, 2};
    eo1 = eo2;
    EXPECT_EQ (eo1->get (), 2);
    EXPECT_EQ (eo2->get (), 2);
}

namespace {

    class move_only {
    public:
        explicit move_only (int v) noexcept
                : v_{v} {}
        move_only (move_only const &) noexcept = delete;
        move_only (move_only &&) noexcept = default;
        move_only & operator= (move_only const &) = delete;
        move_only & operator= (move_only && rhs) = default;

        int get () noexcept { return v_; }

    private:
        int v_;
    };

} // end anonymous namespace

TEST (ErrorOr, MoveAssign) {
    pstore::error_or<move_only> eo1{pstore::in_place, 1};
    EXPECT_EQ (eo1->get (), 1);
    pstore::error_or<move_only> eo2{pstore::in_place, 2};
    eo1 = std::move (eo2);
    EXPECT_EQ (eo1->get (), 2);
}

TEST (ErrorOr, Equal) {
    pstore::error_or<int> eo1{1};
    EXPECT_TRUE (eo1.operator== (1));
    EXPECT_TRUE (eo1.operator==(pstore::error_or<int>{1}));
    EXPECT_FALSE (eo1.operator== (0));
    EXPECT_FALSE (eo1.operator==(pstore::error_or<int>{0}));
    EXPECT_FALSE (eo1.operator== (std::errc::address_family_not_supported));
    EXPECT_FALSE (eo1.operator==(pstore::error_or<int>{std::errc::address_family_not_supported}));

    EXPECT_FALSE (eo1.operator!= (1));
    EXPECT_FALSE (eo1.operator!=(pstore::error_or<int>{1}));
    EXPECT_TRUE (eo1.operator!= (0));
    EXPECT_TRUE (eo1.operator!=(pstore::error_or<int>{0}));
    EXPECT_TRUE (eo1.operator!= (std::errc::address_family_not_supported));
    EXPECT_TRUE (eo1.operator!=(pstore::error_or<int>{std::errc::address_family_not_supported}));
}

TEST (ErrorOrN, StdGet) {
    pstore::error_or_n<int, int> eo{pstore::in_place, 3, 5};
    EXPECT_EQ (std::get<0> (eo), 3);
    EXPECT_EQ (std::get<1> (eo), 5);

    pstore::error_or_n<int, int> const eo_const{pstore::in_place, 7, 11};
    EXPECT_EQ (std::get<0> (eo_const), 7);
    EXPECT_EQ (std::get<1> (eo_const), 11);
}

TEST (ErrorOrN, Bind) {
    pstore::error_or_n<int, int, int> eo{pstore::in_place, 3, 5, 7};
    pstore::error_or<int> y = eo >>= [](int a, int b, int c) {
        return pstore::error_or<int>{pstore::in_place, a + b + c};
    };
    EXPECT_TRUE (y);
    EXPECT_EQ (*y, 15);
}
