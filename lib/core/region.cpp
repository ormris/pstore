//*                 _              *
//*  _ __ ___  __ _(_) ___  _ __   *
//* | '__/ _ \/ _` | |/ _ \| '_ \  *
//* | | |  __/ (_| | | (_) | | | | *
//* |_|  \___|\__, |_|\___/|_| |_| *
//*           |___/                *
//===- lib/core/region.cpp ------------------------------------------------===//
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
/// \file region.cpp
/// \brief A factory for memory-mapper objects which is used for both the initial file
///        allocation and to grow files as allocations are performed in a transaction.
/// The "real" library exclusively uses the file-based factory; the memory-based
/// factory is used for unit testing.

#include "pstore/core/region.hpp"

namespace pstore {
    namespace region {

        std::unique_ptr<factory> get_factory (std::shared_ptr<file::file_handle> const & file,
                                              std::uint64_t full_size, std::uint64_t min_size) {
            return std::make_unique<file_based_factory> (file, full_size, min_size);
        }

        std::unique_ptr<factory> get_factory (std::shared_ptr<file::in_memory> const & file,
                                              std::uint64_t full_size, std::uint64_t min_size) {
            return std::make_unique<mem_based_factory> (file, full_size, min_size);
        }


        //*   __         _                 *
        //*  / _|__ _ __| |_ ___ _ _ _  _  *
        //* |  _/ _` / _|  _/ _ \ '_| || | *
        //* |_| \__,_\__|\__\___/_|  \_, | *
        //*                          |__/  *
        factory::~factory () noexcept = default;

        //*   __ _ _       _                     _    __         _                 *
        //*  / _(_) |___  | |__  __ _ ___ ___ __| |  / _|__ _ __| |_ ___ _ _ _  _  *
        //* |  _| | / -_) | '_ \/ _` (_-</ -_) _` | |  _/ _` / _|  _/ _ \ '_| || | *
        //* |_| |_|_\___| |_.__/\__,_/__/\___\__,_| |_| \__,_\__|\__\___/_|  \_, | *
        //*                                                                  |__/  *
        // (ctor)
        // ~~~~~~
        file_based_factory::file_based_factory (std::shared_ptr<file::file_handle> file,
                                                std::uint64_t const full_size,
                                                std::uint64_t const min_size)
                : factory{full_size, min_size}
                , file_{std::move (file)} {}

        // init
        // ~~~~
        auto file_based_factory::init () -> std::vector<memory_mapper_ptr> {
            return this->create<file::file_handle, memory_mapper> (file_);
        }

        // add
        // ~~~
        void file_based_factory::add (gsl::not_null<std::vector<memory_mapper_ptr> *> const regions,
                                      std::uint64_t const original_size,
                                      std::uint64_t const new_size) {
            this->append<file::file_handle, memory_mapper> (file_, regions, original_size,
                                                            new_size);
        }

        // file
        // ~~~~
        std::shared_ptr<file::file_base> file_based_factory::file () {
            return std::static_pointer_cast<file::file_base> (file_);
        }


        //*                    _                     _    __         _                 *
        //*  _ __  ___ _ __   | |__  __ _ ___ ___ __| |  / _|__ _ __| |_ ___ _ _ _  _  *
        //* | '  \/ -_) '  \  | '_ \/ _` (_-</ -_) _` | |  _/ _` / _|  _/ _ \ '_| || | *
        //* |_|_|_\___|_|_|_| |_.__/\__,_/__/\___\__,_| |_| \__,_\__|\__\___/_|  \_, | *
        //*                                                                      |__/  *
        // (ctor)
        // ~~~~~~
        mem_based_factory::mem_based_factory (std::shared_ptr<file::in_memory> file,
                                              std::uint64_t const full_size,
                                              std::uint64_t const min_size)
                : factory{full_size, min_size}
                , file_{std::move (file)} {}

        // init
        // ~~~~
        auto mem_based_factory::init () -> std::vector<memory_mapper_ptr> {
            return this->create<file::in_memory, in_memory_mapper> (file_);
        }

        // add
        // ~~~
        void mem_based_factory::add (gsl::not_null<std::vector<memory_mapper_ptr> *> const regions,
                                     std::uint64_t const original_size,
                                     std::uint64_t const new_size) {

            this->append<file::in_memory, in_memory_mapper> (file_, regions, original_size,
                                                             new_size);
        }

        // file
        // ~~~~
        std::shared_ptr<file::file_base> mem_based_factory::file () {
            return std::static_pointer_cast<file::file_base> (file_);
        }

    } // end namespace region
} // end namespace pstore
