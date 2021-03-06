//*                _       _      *
//* __      ____ _| |_ ___| |__   *
//* \ \ /\ / / _` | __/ __| '_ \  *
//*  \ V  V / (_| | || (__| | | | *
//*   \_/\_/ \__,_|\__\___|_| |_| *
//*                               *
//===- include/pstore/vacuum/watch.hpp ------------------------------------===//
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
/// \file watch.hpp

#ifndef PSTORE_VACUUM_WATCH_HPP
#define PSTORE_VACUUM_WATCH_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "pstore/core/vacuum_intf.hpp"
#include "pstore/os/file.hpp"
#include "pstore/os/shared_memory.hpp"

namespace pstore {
    class database;
}

namespace vacuum {
    struct watch_state {
        bool start_watch = false;
        std::mutex start_watch_mutex;
        std::condition_variable start_watch_cv;
    };
    // TODO: this shouldn't be a global variable.
    extern watch_state wst;

    struct status;
    void watch (std::shared_ptr<pstore::database> from,
                std::unique_lock<pstore::file::range_lock> & lock, status * const st);
} // namespace vacuum

#endif // PSTORE_VACUUM_WATCH_HPP
