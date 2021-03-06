#!/usr/bin/env python
# *                _ _         _                   _                       _  *
# * __      ___ __(_) |_ ___  | |_ _ __ __ ___   _(_)___   _   _ _ __ ___ | | *
# * \ \ /\ / / '__| | __/ _ \ | __| '__/ _` \ \ / / / __| | | | | '_ ` _ \| | *
# *  \ V  V /| |  | | ||  __/ | |_| | | (_| |\ V /| \__ \ | |_| | | | | | | | *
# *   \_/\_/ |_|  |_|\__\___|  \__|_|  \__,_| \_/ |_|___/  \__, |_| |_| |_|_| *
# *                                                        |___/              *
# ===- utils/write_travis_yml.py ------------------------------------------===//
#  Copyright (c) 2017-2020 by Sony Interactive Entertainment, Inc.
#  All rights reserved.
#
#  Developed by:
#    Toolchain Team
#    SN Systems, Ltd.
#    www.snsystems.com
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the
#  "Software"), to deal with the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#
#  - Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimers.
#
#  - Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimers in the
#    documentation and/or other materials provided with the distribution.
#
#  - Neither the names of SN Systems Ltd., Sony Interactive Entertainment,
#    Inc. nor the names of its contributors may be used to endorse or
#    promote products derived from this Software without specific prior
#    written permission.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
#  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#  SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
# ===----------------------------------------------------------------------===//

from __future__ import print_function
import copy
import yaml


def add_build_type(d, build_type):
    d.setdefault('env', []).append('CMAKE_BUILD_TYPE=' + build_type)
    d['env'].append('PSTORE_ALWAYS_SPANNING={0}'.format('Yes' if build_type.lower() == 'debug' else 'No'))
    return d


BUILDS = [
    {
        'os': 'linux',
        'dist': 'xenial',
        'addons': {
            'apt': {
                'sources': ['ubuntu-toolchain-r-test'],
                'packages': ['clang-3.8', 'ninja-build', 'valgrind'],
            }
        },
        'env': [
            'MATRIX_EVAL="CC=clang-3.8 && CXX=clang++-3.8"',
            # Disable valgrind because the standard library is using instructions that
            # Valgrind 3.11.0 does not support.
            'PSTORE_VALGRIND=No',
        ]
    },
    {
        'os': 'linux',
        'dist': 'xenial',
        'addons': {
            'apt': {
                'sources': [
                    'ubuntu-toolchain-r-test',
                    {
                        'sourceline': 'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main',
                        'key_url': 'https://apt.llvm.org/llvm-snapshot.gpg.key',
                    }
                ],
                'packages': ['clang-9', 'ninja-build'],
            },
        },
        'env': [
            'MATRIX_EVAL="CC=clang-9 && CXX=clang++-9"',
        ]
    },
    {
        'os': 'linux',
        'dist': 'trusty',
        'addons': {
            'apt': {
                'sources': ['ubuntu-toolchain-r-test'],
                'packages': ['g++-9', 'ninja-build', 'valgrind'],
            },
        },
        'env': [
            'MATRIX_EVAL="CC=gcc-9 && CXX=g++-9"',
            'PSTORE_VALGRIND=Yes',
        ]
    },
    {
        'os': 'linux',
        'dist': 'trusty',
        'addons': {
            'apt': {
                'sources': ['ubuntu-toolchain-r-test'],
                'packages': ['g++-5', 'ninja-build', 'valgrind'],
            },
        },
        'env': [
            'MATRIX_EVAL="CC=gcc-5 && CXX=g++-5"',
            'PSTORE_VALGRIND=Yes',
        ]
    },
    {
        'os': 'osx',
        'osx_image': 'xcode9.3',
    },
    {
        'os': 'windows',
    }
]


def main():
    travis = {
        'language': 'cpp',
        'jobs': {
            'include': [add_build_type(copy.deepcopy(build), t) for build in BUILDS for t in
                        ['Debug', 'Release']],
        },
        'before_install': ['eval "${MATRIX_EVAL}"'],
        'script': [
            ' '.join([
                './utils/make_build.py',
                '--verbose',
                '-o build',
                '-D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}',
                '-D PSTORE_EXAMPLES=Yes',
                '-D PSTORE_VALGRIND=${PSTORE_VALGRIND}',
                '-D PSTORE_ALWAYS_SPANNING=${PSTORE_ALWAYS_SPANNING}'
            ]),
            'cmake --build build --config ${CMAKE_BUILD_TYPE}',
            'cmake --build build --config ${CMAKE_BUILD_TYPE} --target pstore-system-tests',
        ]
    }

    print('# Auto-generated by {0}. DO NOT EDIT!'.format(__file__))
    print(yaml.dump(travis, indent=4, default_flow_style=False))


if __name__ == '__main__':
    main()
