//*                       _               _                  *
//*   ___ _ __ ___  _ __ | |_ _   _   ___| |_ ___  _ __ ___  *
//*  / _ \ '_ ` _ \| '_ \| __| | | | / __| __/ _ \| '__/ _ \ *
//* |  __/ | | | | | |_) | |_| |_| | \__ \ || (_) | | |  __/ *
//*  \___|_| |_| |_| .__/ \__|\__, | |___/\__\___/|_|  \___| *
//*                |_|        |___/                          *
//===- unittests/common/empty_store.hpp -----------------------------------===//
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
#ifndef EMPTY_STORE_HPP
#define EMPTY_STORE_HPP

#include <cstdint>
#include <cstdlib>
#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "pstore/database.hpp"


/// Allocates memory whose start address is a multiple of 'align'. This function uses the O/S memory
/// allocation API directly and bypasses operator new/malloc(). This is to ensure that the library
/// can safely change the memory permission (with mprotect() or equivalent).
std::shared_ptr<std::uint8_t> aligned_valloc (std::size_t size, unsigned align);


class EmptyStore : public ::testing::Test {
public:
    static std::size_t constexpr file_size = pstore::storage::min_region_size * 2;

    // Build an empty, in-memory database.
    EmptyStore ();

protected:
    std::shared_ptr<std::uint8_t> buffer_;
    std::shared_ptr<pstore::file::in_memory> file_;

private:
    static constexpr std::size_t page_size_ = 4096;
};

#endif // EMPTY_STORE_HPP
// eof: unittests/common/empty_store.hpp