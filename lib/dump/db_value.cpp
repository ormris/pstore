//*      _ _                  _             *
//*   __| | |__   __   ____ _| |_   _  ___  *
//*  / _` | '_ \  \ \ / / _` | | | | |/ _ \ *
//* | (_| | |_) |  \ V / (_| | | |_| |  __/ *
//*  \__,_|_.__/    \_/ \__,_|_|\__,_|\___| *
//*                                         *
//===- lib/dump/db_value.cpp ----------------------------------------------===//
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
#include "pstore/dump/db_value.hpp"

#include <chrono>
#include <ctime>

#include "pstore/core/generation_iterator.hpp"

namespace {

    pstore::dump::value_ptr
    make_generation (pstore::database const & db,
                     pstore::typed_address<pstore::trailer> const footer_pos, bool const no_times) {
        using namespace pstore::dump;
        auto const trailer = db.getro (footer_pos);
        return make_value (object::container{
            {"footer", make_value (*trailer, no_times)},
            {"content",
             make_blob (db, footer_pos.to_address () - trailer->a.size, trailer->a.size)},
        });
    }

} // end anonymous namespace


namespace pstore {
    namespace dump {

        bool address::default_expanded_ = false;

        // write_impl
        // ~~~~~~~~~~
        std::ostream & address::write_impl (std::ostream & os, indent const & indent) const {
            return this->real_value ()->write_impl (os, indent);
        }
        std::wostream & address::write_impl (std::wostream & os, indent const & indent) const {
            return this->real_value ()->write_impl (os, indent);
        }

        // real_value
        // ~~~~~~~~~~
        value_ptr address::real_value () const {
            if (!value_) {
                if (expanded_) {
                    auto const full = std::make_shared<object> (object::container{
                        {"segment", make_value (addr_.segment ())},
                        {"offset", make_value (addr_.offset ())},
                    });
                    full->compact ();
                    value_ = full;
                } else {
                    value_ = make_value (addr_.absolute ());
                }
            }
            assert (value_.get () != nullptr);
            return value_;
        }


        // *************
        //* make_value *
        // *************
        value_ptr make_value (header const & header) {
            return make_value (object::container{
                {"signature1",
                 make_value (std::begin (header.a.signature1), std::end (header.a.signature1))},
                {"signature2", make_value (header.a.signature2)},
                {"version",
                 make_value (std::begin (header.a.version), std::end (header.a.version))},
                {"uuid", make_value (header.a.uuid.str ())},
                {"crc", make_value (header.crc)},
                {"footer_pos", make_value (header.footer_pos.load ())},
            });
        }

        value_ptr make_value (trailer const & trailer, bool const no_times) {
            return make_value (object::container{
                {"signature1",
                 make_value (std::begin (trailer.a.signature1), std::end (trailer.a.signature1))},
                {"generation", make_value (trailer.a.generation.load ())},
                {"size", make_value (trailer.a.size.load ())},
                {"time", make_time (trailer.a.time, no_times)},
                {"prev_generation", make_value (trailer.a.prev_generation)},
                {"indices", make_value (std::begin (trailer.a.index_records),
                                        std::end (trailer.a.index_records))},
                {"crc", make_value (trailer.crc)},
                {"signature2",
                 make_value (std::begin (trailer.signature2), std::end (trailer.signature2))},
            });
        }

        value_ptr make_value (uuid const & u) { return make_value (u.str ()); }
        value_ptr make_value (index::digest const & d) { return make_value (d.to_hex_string ()); }
        value_ptr make_value (indirect_string const & str) {
            pstore::shared_sstring_view owner;
            return make_value (str.as_db_string_view (&owner));
        }

        value_ptr make_blob (database const & db, pstore::address const begin,
                             std::uint64_t const size) {
            auto const bytes = db.getro (pstore::typed_address<std::uint8_t> (begin), size);
            return make_value (object::container{
                {"size", make_value (size)},
                {"bin", std::make_shared<binary> (bytes.get (), bytes.get () + size)},
            });
        }

        value_ptr make_contents (database const & db, typed_address<trailer> const footer_pos,
                                 bool const no_times) {
            array::container array;
            std::for_each (
                generation_iterator (&db, footer_pos),
                generation_iterator (&db, typed_address<trailer>::null ()),
                [&db, &array, no_times] (pstore::typed_address<pstore::trailer> const fp) {
                    array.emplace_back (make_generation (db, fp, no_times));
                });
            return make_value (std::move (array));
        }

    } // end namespace dump
} // end namespace pstore
