//*               _ _       _                *
//*  _____      _(_) |_ ___| |__   ___  ___  *
//* / __\ \ /\ / / | __/ __| '_ \ / _ \/ __| *
//* \__ \\ V  V /| | || (__| | | |  __/\__ \ *
//* |___/ \_/\_/ |_|\__\___|_| |_|\___||___/ *
//*                                          *
//===- tools/index_structure/switches.hpp ---------------------------------===//
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
#ifndef PSTORE_INDEX_STRUCTURE_SWITCHES_HPP
#define PSTORE_INDEX_STRUCTURE_SWITCHES_HPP

#include <bitset>
#include <string>

#include "pstore/cmd_util/tchar.hpp"
#include "pstore/core/database.hpp"
#include "pstore/config/config.hpp"

#include "indices.hpp"

struct switches {
    std::bitset<static_cast<std::underlying_type<pstore::trailer::indices>::type> (
        pstore::trailer::indices::last)>
        selected;
    unsigned revision = pstore::head_revision;
    std::string db_path;

    bool test (pstore::trailer::indices idx) const {
        auto const position =
            static_cast<std::underlying_type<pstore::trailer::indices>::type> (idx);
        assert (idx < pstore::trailer::indices::last);
        return selected.test (position);
    }
};

std::pair<switches, int> get_switches (int argc, pstore::cmd_util::tchar * argv[]);

#endif // PSTORE_INDEX_STRUCTURE_SWITCHES_HPP
