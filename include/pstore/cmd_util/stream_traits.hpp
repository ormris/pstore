//*      _                              _             _ _        *
//*  ___| |_ _ __ ___  __ _ _ __ ___   | |_ _ __ __ _(_) |_ ___  *
//* / __| __| '__/ _ \/ _` | '_ ` _ \  | __| '__/ _` | | __/ __| *
//* \__ \ |_| | |  __/ (_| | | | | | | | |_| | | (_| | | |_\__ \ *
//* |___/\__|_|  \___|\__,_|_| |_| |_|  \__|_|  \__,_|_|\__|___/ *
//*                                                              *
//===- include/pstore/cmd_util/stream_traits.hpp --------------------------===//
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
#ifndef PSTORE_CMD_UTIL_STREAM_TRAITS_HPP
#define PSTORE_CMD_UTIL_STREAM_TRAITS_HPP

#include <string>

#include "pstore/support/gsl.hpp"
#include "pstore/support/utf.hpp"

namespace pstore {
    namespace cmd_util {

        template <typename T>
        struct stream_trait {};

        template <>
        struct stream_trait<char> {
            static constexpr std::string const & out_string (std::string const & str) noexcept {
                return str;
            }
            static constexpr gsl::czstring out_text (gsl::czstring const str) noexcept {
                return str;
            }
        };

#ifdef _WIN32
        template <>
        struct stream_trait<wchar_t> {
            static std::wstring out_string (std::string const & str) {
                return utf::to_native_string (str);
            }
            static std::wstring out_text (gsl::czstring const str) {
                return utf::to_native_string (str);
            }
        };
#endif // _WIN32

    } // end namespace cmd_util
} // end namespace pstore

#endif // PSTORE_CMD_UTIL_STREAM_TRAITS_HPP
