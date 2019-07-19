//*  _   _                        _  *
//* | |_| |__  _ __ ___  __ _  __| | *
//* | __| '_ \| '__/ _ \/ _` |/ _` | *
//* | |_| | | | | |  __/ (_| | (_| | *
//*  \__|_| |_|_|  \___|\__,_|\__,_| *
//*                                  *
//===- lib/os/thread.cpp --------------------------------------------------===//
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
/// \file thread.cpp

#include "pstore/os/thread.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <system_error>

#ifdef _WIN32

#    include "pstore/config/config.hpp"
#    include "pstore/support/error.hpp"

namespace pstore {
    namespace threads {

        thread_id_type get_id () { return ::GetCurrentThreadId (); }


        static PSTORE_THREAD_LOCAL char thread_name[name_size];

        void set_name (gsl::czstring name) {
            // This code taken from http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
            // Sadly, threads don't actually have names in Win32. The process via
            // RaiseException is just a "Secret Handshake" with the VS Debugger, who
            // actually stores the thread -> name mapping. Windows itself has no
            // notion of a thread "Name".
            if (name == nullptr) {
                throw std::system_error (EINVAL, std::generic_category ());
            } else {
                std::strncpy (thread_name, name, name_size);
                thread_name[name_size - 1] = '\0';
#    ifndef NDEBUG
                DWORD const MS_VC_EXCEPTION = 0x406D1388;

#        pragma pack(push, 8)
                struct THREADNAME_INFO {
                    DWORD dwType;     // Must be 0x1000.
                    LPCSTR szName;    // Pointer to name (in user addr space).
                    DWORD dwThreadID; // Thread ID (-1=caller thread).
                    DWORD dwFlags;    // Reserved for future use, must be zero.
                };
#        pragma pack(pop)

                THREADNAME_INFO info;
                info.dwType = 0x1000;
                info.szName = name;
                info.dwThreadID = GetCurrentThreadId ();
                info.dwFlags = 0;

                __try {
                    RaiseException (MS_VC_EXCEPTION, 0, sizeof (info) / sizeof (ULONG_PTR),
                                    (ULONG_PTR *) &info);
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                }
#    endif // NDEBUG
            }
        }

        char const * get_name (gsl::span<char, name_size> name /*out*/) {
            auto const length = name.size ();
            if (name.data () == nullptr || length < 1) {
                throw std::system_error (EINVAL, std::generic_category ());
            }
            std::strncpy (name.data (), thread_name, length);
            name[length - 1] = '\0';
            return name.data ();
        }

        std::string get_name () {
            std::array<char, name_size> buffer;
            return {get_name (gsl::make_span (buffer))};
        }
    } // namespace threads
} // namespace pstore

#endif //_WIN32
