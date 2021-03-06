//*  _                     _                              __             _  *
//* | |__   __ _ _ __ ___ | |_   _ __ ___   __ _ _ __    / _|_      ____| | *
//* | '_ \ / _` | '_ ` _ \| __| | '_ ` _ \ / _` | '_ \  | |_\ \ /\ / / _` | *
//* | | | | (_| | | | | | | |_  | | | | | | (_| | |_) | |  _|\ V  V / (_| | *
//* |_| |_|\__,_|_| |_| |_|\__| |_| |_| |_|\__,_| .__/  |_|   \_/\_/ \__,_| *
//*                                             |_|                         *
//===- include/pstore/core/hamt_map_fwd.hpp -------------------------------===//
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
/// \file hamt_map_fwd.hpp

#ifndef PSTORE_CORE_HAMT_MAP_FWD_HPP
#define PSTORE_CORE_HAMT_MAP_FWD_HPP

#include <functional>

namespace pstore {
    namespace index {

        /// This class provides a common base from which each of the real index types derives. This
        /// avoids the lower-level storage code needing to know about the types that these indices
        /// contain.
        class index_base {
        public:
            virtual ~index_base () = 0;
        };

        /// The begin() and end() functions for both hamt_map and hamt_set take an extra parameter
        /// -- the owning database -- which prevents the container's direct use in range-based for
        /// loops. This class can provide the required argument. It is created by calling the
        /// make_range() method of either of those container.

        template <typename Database, typename Container, typename Iterator>
        class range {
        public:
            range (Database & d, Container & c)
                    : db_{d}
                    , c_{c} {}
            /// Returns an iterator to the beginning of the container
            Iterator begin () const { return c_.begin (db_); }
            /// Returns an iterator to the end of the container
            Iterator end () const { return c_.end (db_); }

        private:
            Database & db_;
            Container & c_;
        };

        template <typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>,
                  typename KeyEqual = std::equal_to<KeyType>>
        class hamt_map;

        template <typename KeyType, typename Hash = std::hash<KeyType>,
                  typename KeyEqual = std::equal_to<KeyType>>
        class hamt_set;

    } // namespace index
} // namespace pstore
#endif // PSTORE_CORE_HAMT_MAP_FWD_HPP
