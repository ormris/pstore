//*  _                    _                *
//* | |__   ___  __ _  __| | ___ _ __ ___  *
//* | '_ \ / _ \/ _` |/ _` |/ _ \ '__/ __| *
//* | | | |  __/ (_| | (_| |  __/ |  \__ \ *
//* |_| |_|\___|\__,_|\__,_|\___|_|  |___/ *
//*                                        *
//===- lib/http/headers.cpp -----------------------------------------------===//
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
#include "pstore/http/headers.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <limits>
#include <unordered_map>
#include <vector>

#include "pstore/support/ctype.hpp"

using pstore::httpd::header_info;

namespace {

    bool case_insensitive_equal (std::string::const_iterator const lhs_first,
                                 std::string::const_iterator const lhs_last,
                                 std::string::const_iterator const rhs_first,
                                 std::string::const_iterator const rhs_last) noexcept {
        if (std::distance (lhs_first, lhs_last) != std::distance (rhs_first, rhs_last)) {
            return false;
        }
        return std::equal (
            lhs_first, lhs_last, rhs_first, [](char const a, char const b) noexcept {
                auto const lower = [](char const c) noexcept {
                    return static_cast<char> (std::tolower (static_cast<unsigned char> (c)));
                };
                return lower (a) == lower (b);
            });
    }


    bool case_insensitive_equal (std::string const & lhs,
                                 std::string::const_iterator const rhs_first,
                                 std::string::const_iterator const rhs_last) noexcept {
        return case_insensitive_equal (std::begin (lhs), std::end (lhs), rhs_first, rhs_last);
    }

    bool case_insensitive_equal (std::string const & lhs, std::string const & rhs) noexcept {
        return case_insensitive_equal (std::begin (lhs), std::end (lhs), std::begin (rhs),
                                       std::end (rhs));
    }


    template <typename InputIterator, typename OutputIterator>
    OutputIterator split (InputIterator first, InputIterator last, OutputIterator out,
                          typename std::iterator_traits<InputIterator>::value_type separator) {
        for (;;) {
            auto const next = std::find (first, last, separator);
            (*out)++ = std::string{first, next};
            if (next == last) {
                break;
            } else {
                first = std::next (next);
            }
        }
        return out;
    }

    template <typename Container, typename OutputIterator>
    OutputIterator split (Container const & c, OutputIterator out,
                          typename Container::value_type separator) {
        return split (std::begin (c), std::end (c), out, separator);
    }



    header_info upgrade (header_info hi, std::string const & value) {
        if (case_insensitive_equal ("websocket", value)) {
            hi.upgrade_to_websocket = true;
        }
        return hi;
    }

    // The "connection" header is a comma-separated string. We're looking for the "upgrade" request.
    header_info connection (header_info hi, std::string const & value) {
        static std::string const upgrade = "upgrade";

        std::vector<std::string> strings;
        split (value, std::back_inserter (strings), ',');

        for (auto const & str : strings) {
            auto is_ws = [](char const c) { return pstore::isspace (c); };
            // Remove trailing whitespace.
            auto const end = std::find_if_not (str.rbegin (), str.rend (), is_ws).base ();
            // Skip leading whitespace.
            auto const begin = std::find_if_not (str.begin (), end, is_ws);

            if (case_insensitive_equal (upgrade, begin, end)) {
                hi.connection_upgrade = true;
            }
        }

        return hi;
    }

    header_info sec_websocket_key (header_info hi, std::string const & value) {
        hi.websocket_key = value;
        return hi;
    }

    header_info sec_websocket_version (header_info hi, std::string const & value) {
        auto str_to_num = [](std::string const & v) {
            auto pos = std::string::size_type{0};
            auto const last = v.length ();
            auto num = 0U;
            for (; pos < last && std::isdigit (static_cast<unsigned char> (v[pos])); ++pos) {
                if (num > std::numeric_limits<unsigned>::max () / 10U - 10U) {
                    break; // overflow.
                }
                assert (v[pos] >= '0');
                num = num * 10U + static_cast<unsigned> (v[pos] - '0');
            }
            return pos == last ? pstore::just (num) : pstore::nothing<unsigned> ();
        };

        hi.websocket_version = str_to_num (value);
        return hi;
    }

} // end anonymous namespace

bool pstore::httpd::header_info::operator== (header_info const & rhs) const {
    return upgrade_to_websocket == rhs.upgrade_to_websocket &&
           connection_upgrade == rhs.connection_upgrade && websocket_key == rhs.websocket_key &&
           websocket_version == rhs.websocket_version;
}

header_info pstore::httpd::header_info::handler (std::string const & key,
                                                 std::string const & value) {
    static std::unordered_map<
        std::string, std::function<header_info (header_info, std::string const & value)>> const
        handlers = {
            {"connection", connection},
            {"upgrade", upgrade},
            {"sec-websocket-key", sec_websocket_key},
            {"sec-websocket-version", sec_websocket_version},
        };
    auto const pos = handlers.find (key);
    return pos != handlers.end () ? pos->second (*this, value) : *this;
}
