//*                  _        *
//*  _ __ ___   __ _(_)_ __   *
//* | '_ ` _ \ / _` | | '_ \  *
//* | | | | | | (_| | | | | | *
//* |_| |_| |_|\__,_|_|_| |_| *
//*                           *
//===- tools/diff/main.cpp ------------------------------------------------===//
// Copyright (c) 2017-2018 by Sony Interactive Entertainment, Inc.
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
/// \file main.cpp

#include <iostream>

#include "pstore_support/utf.hpp"

#include "./switches.hpp"

namespace {

    auto & out_stream =
#if defined(_WIN32) && defined(_UNICODE)
        std::wcout;
#else
        std::cout;
#endif

#ifdef PSTORE_CPP_EXCEPTIONS
    auto & error_stream =
#if defined(_WIN32) && defined(_UNICODE)
        std::wcerr;
#else
        std::cerr;
#endif
#endif // PSTORE_CPP_EXCEPTIONS

} // namespace

#ifdef PSTORE_CPP_EXCEPTIONS
#define TRY try
#define CATCH(ex, proc) catch (ex) proc
#else
#define TRY
#define CATCH(ex, proc)
#endif


#if defined(_WIN32) && !defined(PSTORE_IS_INSIDE_LLVM)
int _tmain (int argc, TCHAR * argv[]) {
#else
int main (int argc, char * argv[]) {
#endif
    int exit_code = EXIT_SUCCESS;

    TRY {
        switches opt;
        std::tie (opt, exit_code) = get_switches (argc, argv);
        if (exit_code != EXIT_SUCCESS) {
            return exit_code;
        }

        if (opt.hex) {
            pstore::dump::number_base::hex ();
        } else {
            pstore::dump::number_base::dec ();
        }

        pstore::database db (opt.db_path, pstore::database::access_mode::read_only);
        std::tie (opt.first_revision, opt.second_revision) = pstore::diff::update_revisions (
            std::make_pair (opt.first_revision, opt.second_revision), db.get_current_revision ());

        pstore::dump::object::container file;
        file.emplace_back ("indices", pstore::diff::make_indices_diff (db, opt.first_revision,
                                                                       *opt.second_revision));

        auto output = pstore::dump::make_value (file);
        out_stream << NATIVE_TEXT ("---\n") << *output << NATIVE_TEXT ("\n...\n");
    }
    CATCH (std::exception const & ex, {
        error_stream << NATIVE_TEXT ("Error: ") << pstore::utf::to_native_string (ex.what ())
                     << std::endl;
        exit_code = EXIT_FAILURE;
    })
    CATCH (..., {
        error_stream << NATIVE_TEXT ("Unknown error.") << std::endl;
        exit_code = EXIT_FAILURE;
    })
    return exit_code;
}

// eof: tools/diff/main.cpp