//*      _ _               _                                _               *
//*   __| (_)_ __ ___  ___| |_ ___  _ __ _   _    ___ _ __ | |_ _ __ _   _  *
//*  / _` | | '__/ _ \/ __| __/ _ \| '__| | | |  / _ \ '_ \| __| '__| | | | *
//* | (_| | | | |  __/ (__| || (_) | |  | |_| | |  __/ | | | |_| |  | |_| | *
//*  \__,_|_|_|  \___|\___|\__\___/|_|   \__, |  \___|_| |_|\__|_|   \__, | *
//*                                      |___/                       |___/  *
//===- tools/genromfs/directory_entry.hpp ---------------------------------===//
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
#ifndef PSTORE_GENROMFS_DIRECTORY_ENTRY_HPP
#define PSTORE_GENROMFS_DIRECTORY_ENTRY_HPP

#include <ctime>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct directory_entry;
using directory_container = std::vector<directory_entry>;

struct directory_entry {
    directory_entry (std::string name_, unsigned dirno,
                     std::unique_ptr<directory_container> && children_)
            : name (std::move (name_))
            , contents (dirno)
            , modtime{0}
            , children (std::move (children_)) {}
    directory_entry (std::string name_, unsigned fileno, std::time_t modtime_)
            : name (std::move (name_))
            , contents (fileno)
            , modtime{modtime_} {}

    std::string name;
    unsigned contents;
    std::time_t modtime;
    std::unique_ptr<directory_container> children;
};


#endif // PSTORE_GENROMFS_DIRECTORY_ENTRY_HPP
