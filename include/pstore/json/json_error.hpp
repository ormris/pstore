//*    _                                             *
//*   (_)___  ___  _ __     ___ _ __ _ __ ___  _ __  *
//*   | / __|/ _ \| '_ \   / _ \ '__| '__/ _ \| '__| *
//*   | \__ \ (_) | | | | |  __/ |  | | | (_) | |    *
//*  _/ |___/\___/|_| |_|  \___|_|  |_|  \___/|_|    *
//* |__/                                             *
//===- include/pstore/json/json_error.hpp ---------------------------------===//
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
#ifndef PSTORE_JSON_JSON_ERROR_HPP
#define PSTORE_JSON_JSON_ERROR_HPP

#include <string>
#include <system_error>

namespace pstore {
    namespace json {

        enum class error_code : int {
            none,
            expected_array_member,
            expected_close_quote,
            expected_colon,
            expected_digits,
            expected_string,
            number_out_of_range,
            expected_object_member,
            expected_token,
            invalid_escape_char,
            invalid_hex_char,
            unrecognized_token,
            unexpected_extra_input,
            bad_unicode_code_point,
            nesting_too_deep,
        };

        // ******************
        // * error category *
        // ******************
        class error_category : public std::error_category {
        public:
            error_category () noexcept;
            char const * name () const noexcept override;
            std::string message (int error) const override;
        };

        std::error_category const & get_error_category () noexcept;

        inline std::error_code make_error_code (error_code const e) noexcept {
            return {static_cast<int> (e), get_error_category ()};
        }

    } // end namespace json
} // end namespace pstore

namespace std {

    template <>
    struct is_error_code_enum<::pstore::json::error_code> : std::true_type {};

} // end namespace std

#endif // PSTORE_JSON_JSON_ERROR_HPP
