//*      _                   _               *
//*  ___(_) __ _ _ __   __ _| |   _____   __ *
//* / __| |/ _` | '_ \ / _` | |  / __\ \ / / *
//* \__ \ | (_| | | | | (_| | | | (__ \ V /  *
//* |___/_|\__, |_| |_|\__,_|_|  \___| \_/   *
//*        |___/                             *
//===- lib/broker_intf/signal_cv_win32.cpp --------------------------------===//
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
/// \file signal_cv_win32.cpp

#include "pstore/broker_intf/signal_cv.hpp"

#ifdef _WIN32

#    include <cassert>
#    include "pstore/support/error.hpp"
#    include "pstore/support/scope_guard.hpp"

namespace pstore {

    //*     _                _      _              _____   __ *
    //*  __| |___ ___ __ _ _(_)_ __| |_ ___ _ _   / __\ \ / / *
    //* / _` / -_|_-</ _| '_| | '_ \  _/ _ \ '_| | (__ \ V /  *
    //* \__,_\___/__/\__|_| |_| .__/\__\___/_|    \___| \_/   *
    //*                       |_|                             *
    // (ctor)
    // ~~~~~~
    descriptor_condition_variable::descriptor_condition_variable ()
            : event_{::CreateEvent (nullptr /*security attributes*/, FALSE /*manual reset?*/,
                                    FALSE /*initially signalled?*/, nullptr /*name*/)} {
        if (event_.native_handle () == nullptr) {
            raise (win32_erc (::GetLastError ()), "CreateEvent");
        }
    }

    // wait_descriptor
    // ~~~~~~~~~~~~~~~
    pstore::broker::pipe_descriptor const & descriptor_condition_variable::wait_descriptor () const
        noexcept {
        return event_;
    }

    // notify_all
    // ~~~~~~~~~~
    void descriptor_condition_variable::notify_all () {
        if (!::SetEvent (event_.native_handle ())) {
            raise (win32_erc{::GetLastError ()}, "SetEvent");
        }
    }

    // notify_all_no_except
    // ~~~~~~~~~~~~~~~~~~~~
    void descriptor_condition_variable::notify_all_no_except () noexcept {
        ::SetEvent (event_.native_handle ());
    }

    // wait
    // ~~~~
    void descriptor_condition_variable::wait () {
        switch (::WaitForSingleObject (this->wait_descriptor ().native_handle (), INFINITE)) {
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0: return;
        case WAIT_TIMEOUT: assert (0);
        // fallthrough
        case WAIT_FAILED: raise (win32_erc (::GetLastError ()), "WaitForSingleObject");
        }
    }

    void descriptor_condition_variable::wait (std::unique_lock<std::mutex> & lock) {
        lock.unlock ();
        auto _ = make_scope_guard ([&lock]() { lock.lock (); });
        this->wait ();
    }

    // reset
    // ~~~~~
    void descriptor_condition_variable::reset () { ::ResetEvent (event_.native_handle ()); }

} // end namespace pstore

#endif //_WIN32
