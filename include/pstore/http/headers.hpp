//*  _                    _                *
//* | |__   ___  __ _  __| | ___ _ __ ___  *
//* | '_ \ / _ \/ _` |/ _` |/ _ \ '__/ __| *
//* | | | |  __/ (_| | (_| |  __/ |  \__ \ *
//* |_| |_|\___|\__,_|\__,_|\___|_|  |___/ *
//*                                        *
//===- include/pstore/http/headers.hpp ------------------------------------===//
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
#ifndef PSTORE_HTTP_HEADERS_HPP
#define PSTORE_HTTP_HEADERS_HPP

#include <string>

#include "pstore/support/maybe.hpp"

namespace pstore {
    namespace httpd {

        struct header_info {
            bool operator== (header_info const & rhs) const;
            bool operator!= (header_info const & rhs) const { return !operator== (rhs); }

            bool upgrade_to_websocket = false;
            bool connection_upgrade = false;
            pstore::maybe<std::string> websocket_key;
            pstore::maybe<unsigned> websocket_version;

            header_info handler (std::string const & key, std::string const & value);
        };

    } // end namespace httpd
} // end namespace pstore

#endif // PSTORE_HTTP_HEADERS_HPP
