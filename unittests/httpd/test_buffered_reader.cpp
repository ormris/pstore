//*  _            __  __                   _                      _            *
//* | |__  _   _ / _|/ _| ___ _ __ ___  __| |  _ __ ___  __ _  __| | ___ _ __  *
//* | '_ \| | | | |_| |_ / _ \ '__/ _ \/ _` | | '__/ _ \/ _` |/ _` |/ _ \ '__| *
//* | |_) | |_| |  _|  _|  __/ | |  __/ (_| | | | |  __/ (_| | (_| |  __/ |    *
//* |_.__/ \__,_|_| |_|  \___|_|  \___|\__,_| |_|  \___|\__,_|\__,_|\___|_|    *
//*                                                                            *
//===- unittests/httpd/test_buffered_reader.cpp ---------------------------===//
// Copyright (c) 2017-2019 by Sony Interactive Entertainment, Inc.
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
#include "pstore/httpd/buffered_reader.hpp"

#include <cassert>
#include <cerrno>
#include <string>

#include "gmock/gmock.h"

using pstore::error_or;
using pstore::in_place;
using pstore::maybe;
using pstore::gsl::span;
using pstore::httpd::make_buffered_reader;
using testing::_;
using testing::Invoke;

namespace {

    using getc_result_type = error_or<std::pair<int, maybe<char>>>;
    using gets_result_type = error_or<std::pair<int, maybe<std::string>>>;

    using refiller_result_type = error_or<std::pair<int, span<char>::iterator>>;
    using refiller_function = std::function<refiller_result_type (int, span<char> const &)>;

    class mock_refiller {
    public:
        virtual ~mock_refiller () = default;
        virtual refiller_result_type fill (int c, span<char> const &) const = 0;
    };

    class refiller : public mock_refiller {
    public:
        MOCK_CONST_METHOD2 (fill, refiller_result_type (int, span<char> const &));

        refiller_function refill_function () const {
            return [this](int io, span<char> const & s) { return this->fill (io, s); };
        }
    };

    /// Returns a function which which simply returns end-of-stream when invoked.
    refiller_function eof () {
        return [](int io, span<char> const & s) {
            return refiller_result_type{in_place, io + 1, s.begin ()};
        };
    }

    /// Returns a function which will yield the string passed as its argument.
    refiller_function yield_string (std::string const & str) {
        return [str](int io, span<char> const & s) {
#ifndef DEBUG
            auto const size = s.size ();
            assert (size > 0 &&
                    str.length () <=
                        static_cast<std::make_unsigned<decltype (size)>::type> (s.size ()));
#endif
            return refiller_result_type{in_place, io + 1,
                                        std::copy (str.begin (), str.end (), s.begin ())};
        };
    }

} // end anonymous namespace

TEST (HttpdBufferedReader, GetcThenEOF) {
    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("a")));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        getc_result_type const c1 = br.getc (io);
        ASSERT_FALSE (c1.has_error ());
        maybe<char> char1;
        std::tie (io, char1) = c1.get_value ();
        ASSERT_TRUE (char1.has_value ());
        EXPECT_EQ (char1.value (), 'a');
    }
    {
        getc_result_type const c2 = br.getc (io);
        ASSERT_FALSE (c2.has_error ());
        maybe<char> char2;
        std::tie (io, char2) = c2.get_value ();
        ASSERT_FALSE (char2.has_value ());
    }
}

TEST (HttpdBufferedReader, GetTwoStringsLFThenEOF) {
    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("abc\ndef")));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        gets_result_type const s1 = br.gets (io);
        ASSERT_FALSE (s1.has_error ());
        maybe<std::string> str1;
        std::tie (io, str1) = s1.get_value ();
        ASSERT_TRUE (str1.has_value ());
        EXPECT_EQ (str1.value (), "abc");
    }
    {
        gets_result_type const s2 = br.gets (io);
        ASSERT_FALSE (s2.has_error ());
        maybe<std::string> str2;
        std::tie (io, str2) = s2.get_value ();
        ASSERT_TRUE (str2.has_value ());
        EXPECT_EQ (str2.value (), "def");
    }
    {
        gets_result_type const s3 = br.gets (io);
        ASSERT_FALSE (s3.has_error ());
        maybe<std::string> str3;
        std::tie (io, str3) = s3.get_value ();
        ASSERT_FALSE (str3.has_value ());
    }
}

TEST (HttpdBufferedReader, StringCRLF) {
    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("abc\r\ndef")));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        gets_result_type const s1 = br.gets (io);
        ASSERT_FALSE (s1.has_error ());
        maybe<std::string> str1;
        std::tie (io, str1) = s1.get_value ();
        ASSERT_TRUE (str1.has_value ());
        EXPECT_EQ (str1.value (), "abc");
    }
    {
        gets_result_type const s2 = br.gets (io);
        ASSERT_FALSE (s2.has_error ());
        maybe<std::string> str2;
        std::tie (io, str2) = s2.get_value ();
        ASSERT_TRUE (str2.has_value ());
        EXPECT_EQ (str2.value (), "def");
    }
    {
        gets_result_type const s3 = br.gets (io);
        ASSERT_FALSE (s3.has_error ());
        maybe<std::string> str3;
        std::tie (io, str3) = s3.get_value ();
        ASSERT_FALSE (str3.has_value ());
    }
}

TEST (HttpdBufferedReader, StringCRNoLFThenEOF) {
    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("abc\r")));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        gets_result_type const s1 = br.gets (io);
        ASSERT_FALSE (s1.has_error ());
        maybe<std::string> str1;
        std::tie (io, str1) = s1.get_value ();
        ASSERT_TRUE (str1.has_value ());
        EXPECT_EQ (str1.value (), "abc");
    }
    {
        gets_result_type const s2 = br.gets (io);
        ASSERT_FALSE (s2.has_error ());
        maybe<std::string> str2;
        std::tie (io, str2) = s2.get_value ();
        ASSERT_FALSE (str2.has_value ());
    }
}

TEST (HttpdBufferedReader, StringCRNoLFChars) {
    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("abc\rdef")));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        gets_result_type const s1 = br.gets (io);
        ASSERT_FALSE (s1.has_error ());
        maybe<std::string> str1;
        std::tie (io, str1) = s1.get_value ();
        ASSERT_TRUE (str1.has_value ());
        EXPECT_EQ (str1.value (), "abc");
    }
    {
        gets_result_type const s2 = br.gets (io);
        ASSERT_FALSE (s2.has_error ());
        maybe<std::string> str2;
        std::tie (io, str2) = s2.get_value ();
        ASSERT_TRUE (str2.has_value ());
        EXPECT_EQ (str2.value (), "def");
    }
}

TEST (HttpdBufferedReader, SomeCharactersThenAnError) {
    refiller r;
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string ("abc\nd")));
    EXPECT_CALL (r, fill (1, _)).WillOnce (Invoke ([](int, span<char> const &) {
        return refiller_result_type (std::make_error_code (std::errc::operation_not_permitted));
    }));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function ());
    {
        gets_result_type const s1 = br.gets (io);
        ASSERT_FALSE (s1.has_error ()) << "Error: " << s1.get_error ();
        maybe<std::string> str1;
        std::tie (io, str1) = s1.get_value ();
        ASSERT_TRUE (str1.has_value ());
        EXPECT_EQ (str1.value (), "abc");
    }
    {
        gets_result_type const s2 = br.gets (io);
        ASSERT_TRUE (s2.has_error ());
        EXPECT_EQ (s2.get_error (), (std::make_error_code (std::errc::operation_not_permitted)));
    }
}

TEST (HttpdBufferedReader, MaxLengthString) {
    using pstore::httpd::max_string_length;
    std::string const max_length_string (max_string_length, 'a');

    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _)).WillOnce (Invoke (yield_string (max_length_string)));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function (), max_string_length);
    error_or<std::pair<int, maybe<std::string>>> const s1 = br.gets (io);
    ASSERT_FALSE (s1.has_error ()) << "Error: " << s1.get_error ();
    maybe<std::string> str1;
    std::tie (io, str1) = s1.get_value ();
    ASSERT_TRUE (str1.has_value ());
    EXPECT_EQ (str1.value (), max_length_string);
}

TEST (HttpdBufferedReader, StringTooLong) {
    using pstore::httpd::max_string_length;

    refiller r;
    EXPECT_CALL (r, fill (_, _)).WillRepeatedly (Invoke (eof ()));
    EXPECT_CALL (r, fill (0, _))
        .WillOnce (Invoke (yield_string (std::string (max_string_length + 1U, 'a'))));

    auto io = 0;
    auto br = make_buffered_reader<int> (r.refill_function (), max_string_length + 1U);
    error_or<std::pair<int, maybe<std::string>>> const s2 = br.gets (io);
    ASSERT_TRUE (s2.has_error ());
    EXPECT_EQ (s2.get_error (), std::make_error_code (pstore::httpd::error_code::string_too_long));
}
