//*              *
//*   __ _  ___  *
//*  / _` |/ __| *
//* | (_| | (__  *
//*  \__, |\___| *
//*  |___/       *
//===- include/pstore/broker/gc.hpp ---------------------------------------===//
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
/// \file gc.hpp

#ifndef PSTORE_BROKER_GC_HPP
#define PSTORE_BROKER_GC_HPP

#include <string>

#include "pstore/broker/bimap.hpp"
#include "pstore/broker/pointer_compare.hpp"
#include "pstore/broker/spawn.hpp"
#include "pstore/broker_intf/signal_cv.hpp"
#include "pstore/config/config.hpp"

namespace pstore {
    namespace broker {

        class gc_watch_thread {
            friend gc_watch_thread & getgc ();

        public:
            // The gc-watch thread is normally managed by makind calls to
            // start_vacuum(). It is exposed here for unit testing.
            gc_watch_thread () = default;
            virtual ~gc_watch_thread () noexcept;
            void watcher ();

            void start_vacuum (std::string const & db_path);

            /// Called when a shutdown request is received. This method wakes the watcher
            /// thread and asks all child processes to exit.
            void stop (int signum = -1);

            static std::string vacuumd_path ();

#ifdef _WIN32
            static constexpr DWORD max_gc_processes = MAXIMUM_WAIT_OBJECTS - 1U;
#else
            static constexpr int max_gc_processes = 50;
#endif

        private:
            virtual process_identifier spawn (std::initializer_list<gsl::czstring> argv);
            virtual void kill (process_identifier const & pid);

#ifdef _WIN32
            static constexpr auto vacuumd_name = PSTORE_VACUUM_TOOL_NAME ".exe";
            using process_bimap = bimap<std::string, broker::process_identifier,
                                        std::less<std::string>, broker::pointer_compare<HANDLE>>;
#else
            static constexpr auto vacuumd_name = PSTORE_VACUUM_TOOL_NAME;
            using process_bimap = bimap<std::string, pid_t>;

            /// POSIX signal handler.
            static void child_signal (int sig);
#endif

            std::mutex mut_;
            signal_cv cv_;
            process_bimap processes_;
            bool done_ = false;
        };

        gc_watch_thread & getgc ();

        void start_vacuum (std::string path);
        void gc_sigint (int sig);

        void gc_process_watch_thread ();

    } // namespace broker
} // namespace pstore

#endif // PSTORE_BROKER_GC_HPP
