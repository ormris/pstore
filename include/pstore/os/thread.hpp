//*  _   _                        _  *
//* | |_| |__  _ __ ___  __ _  __| | *
//* | __| '_ \| '__/ _ \/ _` |/ _` | *
//* | |_| | | | | |  __/ (_| | (_| | *
//*  \__|_| |_|_|  \___|\__,_|\__,_| *
//*                                  *
//===- include/pstore/os/thread.hpp ---------------------------------------===//
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
/// \file thread.hpp

#ifndef PSTORE_OS_THREAD_HPP
#define PSTORE_OS_THREAD_HPP

#include <cstdint>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#    define NOMINMAX
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    define PSTORE_THREAD_LOCAL __declspec(thread)
#else
#    define PSTORE_THREAD_LOCAL __thread
#endif

#include "pstore/support/gsl.hpp"

namespace pstore {
    namespace threads {

#ifdef _WIN32
        using thread_id_type = DWORD;
#elif defined(__APPLE__)
        using thread_id_type = std::uint64_t;
#elif defined(__linux__)
        using thread_id_type = int;
#elif defined(__FreeBSD__)
        using thread_id_type = int;
#elif defined(__sun__)
        using thread_id_type = std::uint32_t;
        static_assert (sizeof (thread_id_type) == sizeof (pthread_t),
                       "expected pthread_t to be 32-bits");
#else
#    error "Don't know how to represent a thread-id on this OS"
#endif

        constexpr std::size_t name_size = 16;
        void set_name (gsl::not_null<gsl::czstring> name);
        gsl::czstring get_name (gsl::span<char, name_size> name /*out*/);
        std::string get_name ();

        thread_id_type get_id ();

    } // end namespace threads
} // end namespace pstore

#endif // PSTORE_OS_THREAD_HPP
