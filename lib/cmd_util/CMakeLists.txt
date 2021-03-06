#*   ____ __  __       _        _     _     _        *
#*  / ___|  \/  | __ _| | _____| |   (_)___| |_ ___  *
#* | |   | |\/| |/ _` | |/ / _ \ |   | / __| __/ __| *
#* | |___| |  | | (_| |   <  __/ |___| \__ \ |_\__ \ *
#*  \____|_|  |_|\__,_|_|\_\___|_____|_|___/\__|___/ *
#*                                                   *
#===- lib/cmd_util/CMakeLists.txt -----------------------------------------===//
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
set (pstore_cmd_util_include_dir "${PSTORE_ROOT_DIR}/include/pstore/cmd_util")

include (add_pstore)
set (pstore_cmd_util_includes "")
set (pstore_cmd_util_src "")

set (pstore_cmd_util_cl_includes
    "${pstore_cmd_util_include_dir}/category.hpp"
    "${pstore_cmd_util_include_dir}/command_line.hpp"
    "${pstore_cmd_util_include_dir}/help.hpp"
    "${pstore_cmd_util_include_dir}/modifiers.hpp"
    "${pstore_cmd_util_include_dir}/option.hpp"
    "${pstore_cmd_util_include_dir}/parser.hpp"
    "${pstore_cmd_util_include_dir}/string_distance.hpp"
    "${pstore_cmd_util_include_dir}/stream_traits.hpp"
    "${pstore_cmd_util_include_dir}/tchar.hpp"
    "${pstore_cmd_util_include_dir}/word_wrapper.hpp"
)
set (PSTORE_CMD_UTIL_CL_SRC
    category.cpp
    command_line.cpp
    help.cpp
    modifiers.cpp
    option.cpp
    parser.cpp
    string_distance.cpp
    tchar.cpp
    word_wrapper.cpp
)
list (APPEND pstore_cmd_util_src ${PSTORE_CMD_UTIL_CL_SRC})
list (APPEND pstore_cmd_util_includes ${pstore_cmd_util_cl_includes})
source_group ("cl" FILES ${PSTORE_CMD_UTIL_CL_SRC} ${pstore_cmd_util_cl_includes})

set (pstore_cmd_util_opt_helpers_includes
    "${pstore_cmd_util_include_dir}/csv.hpp"
    "${pstore_cmd_util_include_dir}/str_to_revision.hpp"
    "${pstore_cmd_util_include_dir}/revision_opt.hpp"
)
set (PSTORE_CMD_UTIL_OPT_HELPERS_SRC
    csv.cpp
    str_to_revision.cpp
    revision_opt.cpp
)
list (APPEND pstore_cmd_util_src ${PSTORE_CMD_UTIL_OPT_HELPERS_SRC})
list (APPEND pstore_cmd_util_includes ${pstore_cmd_util_opt_helpers_includes})
source_group ("opt_helpers" FILES ${PSTORE_CMD_UTIL_OPT_HELPERS_SRC} ${pstore_cmd_util_opt_helpers_includes})


add_pstore_library (TARGET pstore-cmd-util
    NAME cmd_util
    SOURCES ${pstore_cmd_util_src}
    INCLUDES ${pstore_cmd_util_includes}
)
target_link_libraries (pstore-cmd-util PUBLIC pstore-support)
add_clang_tidy_target (pstore-cmd-util)

if (NOT PSTORE_EXCEPTIONS)
    set (LLVM_REQUIRES_EH Yes)
    set (LLVM_REQUIRES_RTTI Yes)
    set (PSTORE_EXCEPTIONS Yes)

    add_pstore_library (TARGET pstore-cmd-util-ex
        NAME pstore_cmd_util_ex
        SOURCES ${pstore_cmd_util_src}
        INCLUDES ${pstore_cmd_util_includes}
    )

    set (LLVM_REQUIRES_EH No)
    set (LLVM_REQUIRES_RTTI No)
    set (PSTORE_EXCEPTIONS No)

    target_link_libraries (pstore-cmd-util-ex pstore-support-ex)
endif (NOT PSTORE_EXCEPTIONS)
