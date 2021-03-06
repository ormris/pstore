//*  _            _           _               _       __ _      _                   *
//* | | ___  __ _| | __   ___| |__   ___  ___| | __  / _(_)_  _| |_ _   _ _ __ ___  *
//* | |/ _ \/ _` | |/ /  / __| '_ \ / _ \/ __| |/ / | |_| \ \/ / __| | | | '__/ _ \ *
//* | |  __/ (_| |   <  | (__| | | |  __/ (__|   <  |  _| |>  <| |_| |_| | | |  __/ *
//* |_|\___|\__,_|_|\_\  \___|_| |_|\___|\___|_|\_\ |_| |_/_/\_\\__|\__,_|_|  \___| *
//*                                                                                 *
//===- unittests/core/leak_check_fixture.hpp ------------------------------===//
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
#ifndef LEAK_CHECK_FIXTURE_HPP
#define LEAK_CHECK_FIXTURE_HPP

#include <UnitTest++.h>
#include "allocator.h"

#ifndef CL_STANDARD_ONLY
#define CL_STANDARD_ONLY (0)
#endif

#if INSTRUMENT_ALLOCATIONS && !CL_STANDARD_ONLY
class leak_check_fixture {
public:
    leak_check_fixture ()
            : before_ (allocations)
            , before_sequence_ (sequence) {}

    virtual ~leak_check_fixture () {
        dump_allocations_since (NULL, before_sequence_);
        CHECK_EQUAL (before_.number, allocations.number);
        CHECK_EQUAL (before_.total, allocations.total);
    }

private:
    allocation_info before_;
    uint32_t before_sequence_;
};
#else
class leak_check_fixture {
public:
    leak_check_fixture () {}
    virtual ~leak_check_fixture () {}
};
#endif

#endif // LEAK_CHECK_FIXTURE_HPP
