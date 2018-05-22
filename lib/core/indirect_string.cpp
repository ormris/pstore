//*  _           _ _               _         _        _              *
//* (_)_ __   __| (_)_ __ ___  ___| |_   ___| |_ _ __(_)_ __   __ _  *
//* | | '_ \ / _` | | '__/ _ \/ __| __| / __| __| '__| | '_ \ / _` | *
//* | | | | | (_| | | | |  __/ (__| |_  \__ \ |_| |  | | | | | (_| | *
//* |_|_| |_|\__,_|_|_|  \___|\___|\__| |___/\__|_|  |_|_| |_|\__, | *
//*                                                           |___/  *
//===- lib/core/indirect_string.cpp ---------------------------------------===//
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
#include "pstore/core/indirect_string.hpp"

namespace pstore {

    shared_sstring_view indirect_string::as_string_view () const {
        if (is_pointer_) {
            return *str_;
        }
        if (address_ & in_heap_mask) {
            return *reinterpret_cast<shared_sstring_view const *> (address_ & ~in_heap_mask);
        }
        using namespace serialize;
        return read<shared_sstring_view> (archive::make_reader (db_, address{address_}));
    }

    bool indirect_string::operator== (indirect_string const & rhs) const {
        assert (&db_ == &rhs.db_);
        return this->as_string_view () == rhs.as_string_view ();
    }

    bool indirect_string::operator< (indirect_string const & rhs) const {
        assert (&db_ == &rhs.db_);
        return this->as_string_view () < rhs.as_string_view ();
    }

    std::ostream & operator<< (std::ostream & os, indirect_string const & ind_str) {
        return os << ind_str.as_string_view ();
    }

    namespace serialize {

        template <typename DBArchive>
        void serializer<indirect_string>::read_string_address (DBArchive && archive,
                                                               value_type & value) {
            database & db = archive.get_db ();
            auto const addr = *db.getro<address> (archive.get_address ());
            new (&value) value_type (db, addr);
        }

        void serializer<indirect_string>::read (archive::database_reader & archive,
                                                value_type & value) {
            read_string_address (archive, value);
        }

        void serializer<indirect_string>::read (archive::database_reader && archive,
                                                value_type & value) {
            read_string_address (std::forward<archive::database_reader> (archive), value);
        }

    } // namespace serialize

} // namespace pstore

// eof: lib/core/indirect_string.cpp
