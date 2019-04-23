//*  _     _   _               _       _        *
//* | |__ | |_| |_ _ __     __| | __ _| |_ ___  *
//* | '_ \| __| __| '_ \   / _` |/ _` | __/ _ \ *
//* | | | | |_| |_| |_) | | (_| | (_| | ||  __/ *
//* |_| |_|\__|\__| .__/   \__,_|\__,_|\__\___| *
//*               |_|                           *
//===- lib/httpd/http_date.cpp --------------------------------------------===//
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
#include "pstore/httpd/http_date.hpp"

#include <algorithm>
#include <array>
#include <sstream>
#include <iomanip>

#include "pstore/support/time.hpp"

namespace pstore {
    namespace httpd {

        // Produce a date in http-date format (https://tools.ietf.org/html/rfc7231#page-65)
        std::string http_date (time_t time) {
            std::tm const t = gm_time (time);

            // day-name = %x4D.6F.6E ; "Mon", case-sensitive
            //          / %x54.75.65 ; "Tue", case-sensitive
            //          / %x57.65.64 ; "Wed", case-sensitive
            //          / %x54.68.75 ; "Thu", case-sensitive
            //          / %x46.72.69 ; "Fri", case-sensitive
            //          / %x53.61.74 ; "Sat", case-sensitive
            //          / %x53.75.6E ; "Sun", case-sensitive
            static constexpr std::array<char const *, 7> days{"Sun", "Mon", "Tue", "Wed",
                                                              "Thu", "Fri", "Sat"};
            auto const day_name = days[std::min (static_cast<std::size_t> (std::max (t.tm_wday, 0)),
                                                 days.size () - std::size_t{1})];

            // month = %x4A.61.6E ; "Jan", case-sensitive
            //       / %x46.65.62 ; "Feb", case-sensitive
            //       / %x4D.61.72 ; "Mar", case-sensitive
            //       / %x41.70.72 ; "Apr", case-sensitive
            //       / %x4D.61.79 ; "May", case-sensitive
            //       / %x4A.75.6E ; "Jun", case-sensitive
            //       / %x4A.75.6C ; "Jul", case-sensitive
            //       / %x41.75.67 ; "Aug", case-sensitive
            //       / %x53.65.70 ; "Sep", case-sensitive
            //       / %x4F.63.74 ; "Oct", case-sensitive
            //       / %x4E.6F.76 ; "Nov", case-sensitive
            //       / %x44.65.63 ; "Dec", case-sensitive
            static constexpr std::array<char const *, 12> months{
                "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            auto const month = months[std::min (std::max (t.tm_mon, 0), 11)];

            // hour         = 2DIGIT
            // minute       = 2DIGIT
            // second       = 2DIGIT
            // time-of-day  = hour ":" minute ":" second
            //              ; 00:00:00 - 23:59:60 (leap second)
            // year         = 4DIGIT
            // day          = 2DIGIT
            // date1        = day SP month SP year
            //              ; e.g., 02 Jun 1982
            // IMF-fixdate  = day-name "," SP date1 SP time-of-day SP GMT
            std::ostringstream fixdate;
            fixdate << std::setfill ('0') << day_name << ", " << std::setw (2) << t.tm_mday << ' '
                    << month << ' ' << std::setw (4) << t.tm_year + 1900 << ' ' << std::setw (2)
                    << t.tm_hour << ':' << std::setw (2) << t.tm_min << ':' << std::setw (2)
                    << t.tm_sec << " GMT";
            return fixdate.str ();
        }

        std::string http_date (std::chrono::system_clock::time_point time) {
            return http_date (std::chrono::system_clock::to_time_t (time));
        }

    } // end namespace httpd
} // end namespace pstore