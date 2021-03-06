//*            _        _   _               _              *
//*  _ __ ___ | |_ __ _| |_(_)_ __   __ _  | | ___   __ _  *
//* | '__/ _ \| __/ _` | __| | '_ \ / _` | | |/ _ \ / _` | *
//* | | | (_) | || (_| | |_| | | | | (_| | | | (_) | (_| | *
//* |_|  \___/ \__\__,_|\__|_|_| |_|\__, | |_|\___/ \__, | *
//*                                 |___/           |___/  *
//===- unittests/core/test_rotating_log.cpp -------------------------------===//
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
#include "pstore/os/rotating_log.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "pstore/os/file.hpp"
#include "pstore/support/path.hpp"

namespace {

    struct mock_file_system_traits {
        mock_file_system_traits () {}
        mock_file_system_traits (mock_file_system_traits const &) {}
        mock_file_system_traits & operator= (mock_file_system_traits const &) { return *this; }

        MOCK_METHOD1 (exists, bool(std::string const &));
        MOCK_METHOD1 (unlink, void(std::string const &));
        MOCK_METHOD2 (rename, void(std::string const &, std::string const &));
    };

    struct mock_string_stream_traits {
        using stream_type = std::ostringstream;

        mock_string_stream_traits () {}
        mock_string_stream_traits (mock_string_stream_traits const &) {}
        mock_string_stream_traits & operator= (mock_string_stream_traits const &) { return *this; }

        MOCK_METHOD3 (open, void(stream_type &, std::string const &, std::ios_base::openmode));
        MOCK_METHOD1 (close, void(stream_type &));
        MOCK_METHOD1 (clear, void(stream_type &));
    };

} // end anonymous namespace

TEST (RotatingLog, NothingIsLogged) {
    using log_type =
        pstore::logging::basic_rotating_log<mock_string_stream_traits, mock_file_system_traits>;
    log_type log ("base_name", std::ios_base::streamoff{0} /*max_bytes*/, 0U /*num_backups*/);
    EXPECT_FALSE (log.is_open ()) << "Expected the log file to be initially closed";
    EXPECT_EQ ("", log.stream ().str ());
}


TEST (RotatingLog, OneFile) {
    using ::testing::_;
    using ::testing::Expectation;
    using ::testing::Invoke;
    using ::testing::StrEq;

    using log_type =
        pstore::logging::basic_rotating_log<mock_string_stream_traits, mock_file_system_traits>;
    log_type log ("base_name", std::ios_base::streamoff{0} /*max_bytes*/, 0U /*num_backups*/);

    Expectation open =
        EXPECT_CALL (log.stream_traits (), open (_, StrEq ("base_name"), _)).Times (1);
    EXPECT_CALL (log.stream_traits (), close (_)).Times (1).After (open);

    EXPECT_CALL (log.file_system_traits (), exists (_)).Times (0);
    EXPECT_CALL (log.file_system_traits (), unlink (_)).Times (0);
    EXPECT_CALL (log.file_system_traits (), rename (_, _)).Times (0);

    // Finally, we can log a simple string.
    log.log_impl ("hello world");

    EXPECT_TRUE (log.is_open ())
        << "Expected the log file to open after a message has been written";
    EXPECT_EQ ("hello world", log.stream ().str ());
}


TEST (RotatingLog, TwoRotations) {
    using ::testing::_;
    using ::testing::AnyNumber;
    using ::testing::Invoke;
    using ::testing::Return;
    using ::testing::StrEq;

    using log_type =
        pstore::logging::basic_rotating_log<mock_string_stream_traits, mock_file_system_traits>;

    // We'll contrive two rollovers by generating at least max_size * num_backups
    // worth of output.
    constexpr auto max_size = std::ios_base::streamoff{100};
    constexpr auto num_backups = 2U;
    log_type log ("base_name", max_size, num_backups);

    // Check that the rollover process renames bn1->bn2 and bn->bn1 twice each (once
    // for each rollover. I also check that we open the real output file three times;
    // that is, two rollovers and a partially filled third file.
    {
        auto & string_stream = log.stream_traits ();
        auto & file_system = log.file_system_traits ();

        // I need the clear() method to really clear the stream contents.
        EXPECT_CALL (string_stream, clear (_))
            .Times (AnyNumber ())
            .WillRepeatedly (Invoke ([](std::ostringstream & os) { os.str (""); }));

        EXPECT_CALL (string_stream, open (_, StrEq ("base_name"), _)).Times (3);
        EXPECT_CALL (string_stream, close (_)).Times (3);

        EXPECT_CALL (file_system, exists (_)).WillRepeatedly (Return (true));
        EXPECT_CALL (file_system, unlink (_)).Times (AnyNumber ());
        EXPECT_CALL (file_system, rename (StrEq ("base_name.1"), StrEq ("base_name.2"))).Times (2);
        EXPECT_CALL (file_system, rename (StrEq ("base_name"), StrEq ("base_name.1"))).Times (2);
    }

    // Put together an array of strings which contain "message" followed by the decimal offset
    // of the string. We want one more than will fit in "max_size" bytes (not including line
    // feeds).
    std::vector<std::string> messages;
    {
        auto size = std::ios_base::streamoff{0};
        auto index = 1U;
        while (size < max_size * num_backups) {
            std::ostringstream str;
            str << "message " << index;
            std::string const & message = str.str ();
            messages.push_back (message);

            size += message.size ();
            ++index;
        }
    }
    // Now log each of the strings.
    for (auto const & m : messages) {
        log.log_impl (m);
    }

    // A single copy of the string is left in the active "file".
    EXPECT_EQ ("message 21", log.stream ().str ());


#if 0
    // This is a system test. It shouldn't be here.
    logging::create_log_stream (std::make_unique <logging::rotating_log> ("footle", 500, 2));
    for (auto const & m : messages) {
        logging::log (logging::priority::info, m);
    }
    logging::create_log_stream (std::unique_ptr <logging::rotating_log> ());
#endif
}
