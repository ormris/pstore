//*                _ _             *
//* __      ___ __(_) |_ ___ _ __  *
//* \ \ /\ / / '__| | __/ _ \ '__| *
//*  \ V  V /| |  | | ||  __/ |    *
//*   \_/\_/ |_|  |_|\__\___|_|    *
//*                                *
//===- include/pstore/broker_intf/writer.hpp ------------------------------===//
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
/// \file writer.hpp
/// \brief Provides a simple interface to enable a client to send messages to the pstore broker.

#ifndef PSTORE_BROKER_INTF_WRITER_HPP
#define PSTORE_BROKER_INTF_WRITER_HPP

#include <chrono>
#include <cstdlib>
#include <limits>
#include <functional>

// pstore broker-interface
#include "pstore/broker_intf/descriptor.hpp"
#include "pstore/broker_intf/fifo_path.hpp"
// pstore support
#include "pstore/support/portab.hpp"

namespace pstore {
    namespace broker {

        class message_type;

        class writer {
        public:
            using duration_type = std::chrono::milliseconds;
            static constexpr unsigned infinite_retries = std::numeric_limits<unsigned>::max ();

            using update_callback = std::function<void()>;
            /// The default function called by write() to as the write() function retries the
            /// the write operation. A simple no-op.
            static void default_callback ();

            writer (fifo_path::client_pipe && pipe, duration_type retry_timeout,
                    unsigned max_retries, update_callback cb = default_callback);
            writer (fifo_path::client_pipe && pipe, update_callback cb = default_callback);

            writer (fifo_path const & fifo, duration_type retry_timeout, unsigned max_retries,
                    update_callback cb = default_callback);
            writer (fifo_path const & fifo, update_callback cb = default_callback);
            writer (writer const &) = delete;
            writer (writer && rhs) noexcept = default;

            virtual ~writer () = default;

            writer & operator= (writer const &) = delete;
            writer & operator= (writer && rhs) noexcept = default;

            void write (message_type const & msg, bool error_on_timeout);

        private:
            // (virtual for unit testing)
            virtual bool write_impl (message_type const & msg);

            /// The pipe to which the write() function will write data.
            fifo_path::client_pipe fd_;
            /// The time delay between retries.
            duration_type retry_timeout_;
            /// The number of retries that will be attempted before write() will give up.
            unsigned max_retries_;
            /// A function which is called by write() before each retry.
            update_callback update_cb_;
        };

    } // namespace broker
} // namespace pstore

#endif // PSTORE_BROKER_INTF_WRITER_HPP
