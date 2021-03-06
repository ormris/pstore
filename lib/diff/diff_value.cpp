//*      _ _  __  __              _             *
//*   __| (_)/ _|/ _| __   ____ _| |_   _  ___  *
//*  / _` | | |_| |_  \ \ / / _` | | | | |/ _ \ *
//* | (_| | |  _|  _|  \ V / (_| | | |_| |  __/ *
//*  \__,_|_|_| |_|     \_/ \__,_|_|\__,_|\___| *
//*                                             *
//===- lib/diff/diff_value.cpp --------------------------------------------===//
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

#include "pstore/diff/diff_value.hpp"

#include "pstore/core/hamt_map.hpp"
#include "pstore/core/hamt_set.hpp"
#include "pstore/core/index_types.hpp"
#include "pstore/core/sstring_view_archive.hpp"


namespace pstore {
    namespace diff {

        dump::value_ptr make_indices_diff (database & db, diff::revision_number const new_revision,
                                           diff::revision_number const old_revision) {
            assert (new_revision >= old_revision);
            return dump::make_value (
                {make_index_diff<index::name_index> ("names", db, new_revision, old_revision,
                                                     index::get_index<trailer::indices::name>),
                 make_index_diff<index::fragment_index> (
                     "fragments", db, new_revision, old_revision,
                     index::get_index<trailer::indices::fragment>),
                 make_index_diff<index::compilation_index> (
                     "compilations", db, new_revision, old_revision,
                     index::get_index<trailer::indices::compilation>),
                 make_index_diff<index::debug_line_header_index> (
                     "debug_line_headers", db, new_revision, old_revision,
                     index::get_index<trailer::indices::debug_line_header>)});
        }

    } // namespace diff
} // namespace pstore
