//*  _                             _   _              *
//* | |__  ___ ___   ___  ___  ___| |_(_) ___  _ __   *
//* | '_ \/ __/ __| / __|/ _ \/ __| __| |/ _ \| '_ \  *
//* | |_) \__ \__ \ \__ \  __/ (__| |_| | (_) | | | | *
//* |_.__/|___/___/ |___/\___|\___|\__|_|\___/|_| |_| *
//*                                                   *
//===- lib/mcrepo/bss_section.cpp -----------------------------------------===//
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
#include "pstore/mcrepo/bss_section.hpp"

namespace pstore {
    namespace repo {

        //*                  _   _               _ _               _      _             *
        //*  __ _ _ ___ __ _| |_(_)___ _ _    __| (_)____ __  __ _| |_ __| |_  ___ _ _  *
        //* / _| '_/ -_) _` |  _| / _ \ ' \  / _` | (_-< '_ \/ _` |  _/ _| ' \/ -_) '_| *
        //* \__|_| \___\__,_|\__|_\___/_||_| \__,_|_/__/ .__/\__,_|\__\__|_||_\___|_|   *
        //*                                            |_|                              *
        std::size_t bss_section_creation_dispatcher::size_bytes () const {
            return sizeof (bss_section);
        }

        std::uint8_t * bss_section_creation_dispatcher::write (std::uint8_t * const out) const {
            assert (this->aligned (out) == out);
            assert (section_->data.size () <= std::numeric_limits<bss_section::size_type>::max ());
            auto * const scn = new (out) bss_section (
                section_->align, static_cast<bss_section::size_type> (section_->data.size ()));
            return out + scn->size_bytes ();
        }

        std::uintptr_t
        bss_section_creation_dispatcher::aligned_impl (std::uintptr_t const in) const {
            return pstore::aligned<bss_section> (in);
        }


        //*             _   _               _ _               _      _             *
        //*  ___ ___ __| |_(_)___ _ _    __| (_)____ __  __ _| |_ __| |_  ___ _ _  *
        //* (_-</ -_) _|  _| / _ \ ' \  / _` | (_-< '_ \/ _` |  _/ _| ' \/ -_) '_| *
        //* /__/\___\__|\__|_\___/_||_| \__,_|_/__/ .__/\__,_|\__\__|_||_\___|_|   *
        //*                                       |_|                              *
        bss_section_dispatcher::~bss_section_dispatcher () noexcept = default;

    } // end namespace repo
} // end namespace pstore
