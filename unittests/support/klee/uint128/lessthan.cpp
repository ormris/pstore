//*  _               _   _                  *
//* | | ___  ___ ___| |_| |__   __ _ _ __   *
//* | |/ _ \/ __/ __| __| '_ \ / _` | '_ \  *
//* | |  __/\__ \__ \ |_| | | | (_| | | | | *
//* |_|\___||___/___/\__|_| |_|\__,_|_| |_| *
//*                                         *
//===- unittests/support/klee/uint128/lessthan.cpp ------------------------===//
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
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <klee/klee.h>
#include "pstore/support/uint128.hpp"
#include "./common.hpp"

int main (int argc, char * argv[]) {
    pstore::uint128 lhs;
    pstore::uint128 rhs;
    klee_make_symbolic (&lhs, sizeof (lhs), "lhs");
    klee_make_symbolic (&rhs, sizeof (rhs), "rhs");

    bool const comp = lhs < rhs;
    bool const compx = to_native (lhs) < to_native (rhs);

    // std::printf ("value:0x%" PRIx64 ",0x%" PRIx64 " dist:0x%x\n", value.high(), value.low (),
    // dist);
    assert (comp == compx);
}
