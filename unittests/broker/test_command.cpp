//*                                                _  *
//*   ___ ___  _ __ ___  _ __ ___   __ _ _ __   __| | *
//*  / __/ _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | *
//* | (_| (_) | | | | | | | | | | | (_| | | | | (_| | *
//*  \___\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_| *
//*                                                   *
//===- unittests/broker/test_command.cpp ----------------------------------===//
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

#include "pstore/broker/command.hpp"

#include <chrono>
#include <functional>

#include "gmock/gmock.h"

#include "pstore/broker_intf/fifo_path.hpp"
#include "pstore/http/server_status.hpp"

using namespace std::chrono_literals;

namespace {

    class mock_cp : public pstore::broker::command_processor {
    public:
        explicit mock_cp (unsigned const num_read_threads,
                          pstore::httpd::server_status * const status,
                          std::atomic<bool> * const uptime_done)
                : command_processor (num_read_threads, status, uptime_done, 4h) {}

        MOCK_METHOD2 (suicide, void(pstore::broker::fifo_path const &,
                                    pstore::broker::broker_command const &));
        MOCK_METHOD2 (quit, void(pstore::broker::fifo_path const &,
                                 pstore::broker::broker_command const &));
        MOCK_METHOD2 (cquit, void(pstore::broker::fifo_path const &,
                                  pstore::broker::broker_command const &));
        MOCK_METHOD2 (gc, void(pstore::broker::fifo_path const &,
                               pstore::broker::broker_command const &));
        MOCK_METHOD2 (echo, void(pstore::broker::fifo_path const &,
                                 pstore::broker::broker_command const &));
        MOCK_METHOD2 (nop, void(pstore::broker::fifo_path const &,
                                pstore::broker::broker_command const &));
        MOCK_CONST_METHOD1 (unknown, void(pstore::broker::broker_command const &));

        // Replace the log message with an implementation that does nothing at all. We don't really
        // want to be writing logs from the unit tests.
        virtual void log (pstore::broker::broker_command const &) const {}
        virtual void log (pstore::gsl::czstring) const {}
    };

    class Command : public ::testing::Test {
    public:
        Command ()
                : http_status_{8080}
                , uptime_done_{false}
                , cp_{1U, &http_status_, &uptime_done_}
                , fifo_{nullptr} {}

        mock_cp & cp () { return cp_; }
        pstore::broker::fifo_path & fifo () { return fifo_; }

        static constexpr std::uint32_t message_id = 0;
        static constexpr std::uint16_t part_no = 0;
        static constexpr std::uint16_t num_parts = 1;

    private:
        pstore::httpd::server_status http_status_;
        std::atomic<bool> uptime_done_;

        mock_cp cp_;
        pstore::broker::fifo_path fifo_;
    };
} // namespace

// Not a particularly useful test as such, but suppresses warnings from clang about unused member
// functions in mock_cp.
TEST_F (Command, NoCalls) {
    using ::testing::_;
    EXPECT_CALL (cp (), suicide (_, _)).Times (0);
    EXPECT_CALL (cp (), quit (_, _)).Times (0);
    EXPECT_CALL (cp (), cquit (_, _)).Times (0);
    EXPECT_CALL (cp (), gc (_, _)).Times (0);
    EXPECT_CALL (cp (), echo (_, _)).Times (0);
    EXPECT_CALL (cp (), nop (_, _)).Times (0);
}

TEST_F (Command, Nop) {
    using ::testing::_;

    EXPECT_CALL (cp (), nop (_, _)).Times (1);
    pstore::broker::message_type msg{message_id, part_no, num_parts, "NOP"};
    cp ().process_command (fifo (), msg);
}

TEST_F (Command, Bad) {
    EXPECT_CALL (cp (), unknown (pstore::broker::broker_command{"bad", "command"})).Times (1);

    pstore::broker::message_type msg{message_id, part_no, num_parts, "bad command"};
    cp ().process_command (fifo (), msg);
}
