//*  _           _             _                          *
//* (_)_ __   __| | _____  __ | |_ _   _ _ __   ___  ___  *
//* | | '_ \ / _` |/ _ \ \/ / | __| | | | '_ \ / _ \/ __| *
//* | | | | | (_| |  __/>  <  | |_| |_| | |_) |  __/\__ \ *
//* |_|_| |_|\__,_|\___/_/\_\  \__|\__, | .__/ \___||___/ *
//*                                |___/|_|               *
//===- include/pstore/core/index_types.hpp --------------------------------===//
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

#ifndef PSTORE_INDEX_TYPES_HPP
#define PSTORE_INDEX_TYPES_HPP (1)

#include "pstore/core/file_header.hpp"
#include "pstore/core/fnv.hpp"
#include "pstore/core/hamt_map_fwd.hpp"
#include "pstore/core/uint128.hpp"
#include "pstore/core/uuid.hpp"
#include "pstore/support/sstring_view.hpp"

namespace pstore {
    namespace index {

        using digest = uint128;
        struct u128_hash {
            std::uint64_t operator() (digest const & v) const { return v.high (); }
        };

    } // namespace index

    namespace serialize {
        /// \brief A serializer for uint128
        template <>
        struct serializer<uint128> {

            /// \brief Writes an individual uint128 instance to an archive.
            ///
            /// \param archive  The archive to which the span will be written.
            /// \param v        The object value which is to be written.
            template <typename Archive>
            static auto write (Archive & archive, uint128 const & v) ->
                typename Archive::result_type {
                return archive.put (v);
            }

            /// \brief Writes a span of uint128 instances to an archive.
            ///
            /// \param archive  The archive to which the span will be written.
            /// \param span     The span which is to be written.
            template <typename Archive, typename SpanType>
            static auto writen (Archive & archive, SpanType span) -> typename Archive::result_type {
                static_assert (std::is_same<typename SpanType::element_type, uint128>::value,
                               "span type does not match the serializer type");
                return archive.putn (span);
            }

            /// \brief Reads a uint128 value from an archive.
            ///
            /// \param archive  The archive from which the value will be read.
            /// \param out      A reference to uninitialized memory into which a uint128 will be
            /// read.
            template <typename Archive>
            static void read (Archive & archive, uint128 & out) {
                assert (reinterpret_cast<std::uintptr_t> (&out) % alignof (uint128) == 0);
                archive.get (out);
            }

            /// \brief Reads a span of uint128 values from an archive.
            ///
            /// \param archive  The archive from which the value will be read.
            /// \param span     A span pointing to uninitialized memory
            template <typename Archive, typename Span>
            static void readn (Archive & archive, Span span) {
                static_assert (std::is_same<typename Span::element_type, uint128>::value,
                               "span type does not match the serializer type");
                details::getn_helper::getn (archive, span);
            }
        };

    } // namespace serialize

    class database;
    class transaction_base;

    namespace index {

        using write_index = hamt_map<std::string, extent>;
        using digest_index = hamt_map<digest, extent, u128_hash>;
        using ticket_index = hamt_map<digest, extent, u128_hash>;
        using name_index = hamt_set<sstring_view<std::shared_ptr<char const>>, fnv_64a_hash>;


        /// Returns a pointer to the write index, loading it from the store on first access. If
        /// 'create' is false and the index does not already exist then nullptr is returned.
        write_index * get_write_index (database & db, bool create = true);

        /// Returns a pointer to the digest index, loading it from the store on first access. If
        /// 'create' is false and the index does not already exist then nullptr is returned.
        digest_index * get_digest_index (database & db, bool create = true);

        /// Returns a pointer to the ticket index, loading it from the store on first access. If
        /// 'create' is false and the index does not already exist then nullptr is returned.
        ticket_index * get_ticket_index (database & db, bool create = true);

        /// Returns a pointer to the name index, loading it from the store on first access. If
        /// 'create' is false and the index does not already exist then nullptr is returned.
        name_index * get_name_index (database & db, bool create = true);

        /// Write out any indices that have changed. Any that haven't will
        /// continue to point at their previous incarnation. Update the
        /// members of the 'locations' array.
        ///
        /// This happens early in the process of commiting a transaction; we're
        /// allocating and writing space in the store here.
        ///
        /// \param transaction An open transaction to which the index data will be added.
        /// \param locations  An array of the index locations. Any modified indices
        ///                   will be modified to point at the new file address.
        void flush_indices (::pstore::transaction_base & transaction,
                            trailer::index_records_array * const locations);

    } // namespace index
} // namespace pstore
#endif // PSTORE_INDEX_TYPES_HPP
// eof: include/pstore/core/index_types.hpp