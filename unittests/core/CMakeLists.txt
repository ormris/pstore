#*   ____ __  __       _        _     _     _        *
#*  / ___|  \/  | __ _| | _____| |   (_)___| |_ ___  *
#* | |   | |\/| |/ _` | |/ / _ \ |   | / __| __/ __| *
#* | |___| |  | | (_| |   <  __/ |___| \__ \ |_\__ \ *
#*  \____|_|  |_|\__,_|_|\_\___|_____|_|___/\__|___/ *
#*                                                   *
#===- unittests/core/CMakeLists.txt ---------------------------------------===//
# Copyright (c) 2017-2020 by Sony Interactive Entertainment, Inc.
# All rights reserved.
#
# Developed by:
#   Toolchain Team
#   SN Systems, Ltd.
#   www.snsystems.com
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal with the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimers.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimers in the
#   documentation and/or other materials provided with the distribution.
#
# - Neither the names of SN Systems Ltd., Sony Interactive Entertainment,
#   Inc. nor the names of its contributors may be used to endorse or
#   promote products derived from this Software without specific prior
#   written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#===----------------------------------------------------------------------===//

# The pstore unit test sources
add_pstore_unit_test (pstore-core-unit-tests
    leak_check_fixture.hpp
    test_address.cpp
    test_array_stack.cpp
    test_base32.cpp
    test_basic_logger.cpp
    test_crc32.cpp
    test_database.cpp
    test_db_archive.cpp
    test_generation_iterator.cpp
    test_hamt_map.cpp
    test_hamt_set.cpp
    test_heartbeat.cpp
    test_indirect_string.cpp
    test_protect.cpp
    test_region.cpp
    test_rotating_log.cpp
    test_sstring_view_archive.cpp
    test_storage.cpp
    test_sync.cpp
    test_transaction.cpp
    test_two_connections.cpp
    test_uuid.cpp
)
target_link_libraries (pstore-core-unit-tests PRIVATE pstore-core pstore-common)

# Access the library's private include directory.
target_include_directories (pstore-core-unit-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/core")

add_subdirectory (broker_intf)
