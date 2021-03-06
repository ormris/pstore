//*   __                                      _    *
//*  / _|_ __ __ _  __ _ _ __ ___   ___ _ __ | |_  *
//* | |_| '__/ _` |/ _` | '_ ` _ \ / _ \ '_ \| __| *
//* |  _| | | (_| | (_| | | | | | |  __/ | | | |_  *
//* |_| |_|  \__,_|\__, |_| |_| |_|\___|_| |_|\__| *
//*                |___/                           *
//===- include/pstore/mcrepo/fragment.hpp ---------------------------------===//
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
#ifndef PSTORE_MCREPO_FRAGMENT_HPP
#define PSTORE_MCREPO_FRAGMENT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

#include "pstore/config/config.hpp"
#include "pstore/core/address.hpp"
#include "pstore/core/transaction.hpp"
#include "pstore/mcrepo/bss_section.hpp"
#include "pstore/mcrepo/debug_line_section.hpp"
#include "pstore/mcrepo/dependents_section.hpp"
#include "pstore/mcrepo/generic_section.hpp"
#include "pstore/mcrepo/repo_error.hpp"
#include "pstore/mcrepo/sparse_array.hpp"
#include "pstore/support/aligned.hpp"
#include "pstore/support/inherit_const.hpp"
#include "pstore/support/max.hpp"
#include "pstore/support/pointee_adaptor.hpp"
#include "pstore/support/small_vector.hpp"

namespace pstore {
    namespace repo {

        namespace details {

            /// An iterator adaptor which produces a value_type of 'section_kind const' from
            /// values deferenced from the supplied underlying iterator.
            template <typename Iterator>
            class content_type_iterator {
            public:
                using value_type = section_kind const;
                using difference_type = typename std::iterator_traits<Iterator>::difference_type;
                using pointer = value_type *;
                using reference = value_type &;
                using iterator_category = std::input_iterator_tag;

                content_type_iterator () = default;
                explicit content_type_iterator (Iterator it)
                        : it_{it} {}
                content_type_iterator (content_type_iterator const & rhs)
                        : it_{rhs.it_} {}
                content_type_iterator & operator= (content_type_iterator const & rhs) {
                    it_ = rhs.it_;
                    return *this;
                }
                bool operator== (content_type_iterator const & rhs) const { return it_ == rhs.it_; }
                bool operator!= (content_type_iterator const & rhs) const {
                    return !(operator== (rhs));
                }
                content_type_iterator & operator++ () {
                    ++it_;
                    return *this;
                }
                content_type_iterator operator++ (int) {
                    auto const old = *this;
                    it_++;
                    return old;
                }

                reference operator* () const { return (*it_).kind (); }
                pointer operator-> () const { return &(*it_).kind (); }
                reference operator[] (difference_type n) const { return it_[n].kind (); }

            private:
                Iterator it_{};
            };

            template <typename Iterator>
            inline content_type_iterator<Iterator> make_content_type_iterator (Iterator it) {
                return content_type_iterator<Iterator> (it);
            }


        } // end namespace details


        // clang-format off
        /// Maps from the section kind enumeration to the type that is used to represent a section
        /// of that kind.
        template <section_kind T>
        struct enum_to_section { using type = generic_section; };
        template <> struct enum_to_section<section_kind::bss> { using type = bss_section; };
        template <> struct enum_to_section<section_kind::debug_line> { using type = debug_line_section; };
        template <> struct enum_to_section<section_kind::dependent> { using type = dependents; };
        // clang-format on



        //*   __                             _    *
        //*  / _|_ _ __ _ __ _ _ __  ___ _ _| |_  *
        //* |  _| '_/ _` / _` | '  \/ -_) ' \  _| *
        //* |_| |_| \__,_\__, |_|_|_\___|_||_\__| *
        //*              |___/                    *
        class fragment {
        public:
            using member_array = sparse_array<std::uint64_t>;

            /// Prepares an instance of a fragment with the collection of sections defined by the
            /// iterator range [first, last).
            ///
            /// \tparam Transaction  The type of the database transaction.
            /// \tparam Iterator  An iterator which will yield values of type
            ///   pstore::repo::section_creation_dispatcher. The values must be sorted in
            ///   pstore::repo::section_content::kind order.
            /// \param transaction  The transaction to which the fragment should be appended.
            /// \param first  The beginning of the range of
            ///   pstore::repo::section_creation_dispatcher values.
            /// \param last  The end of the range of pstore::repo::section_creation_dispatcher
            ///   values.
            template <typename Transaction, typename Iterator>
            static pstore::extent<fragment> alloc (Transaction & transaction, Iterator first,
                                                   Iterator last);

            /// Provides a pointer to an individual fragment instance given a database and a extent
            /// describing its address and size.
            ///
            /// \param db  The database from which the fragment is to be read.
            /// \param location  The address and size of the fragment data.
            /// \returns  A pointer to the fragment instance.
            static std::shared_ptr<fragment const> load (database const & db,
                                                         extent<fragment> const & location);

            /// Provides a pointer to an individual fragment instance given a transaction and an
            /// extent describing its address and size.
            ///
            /// \tparam LockType  The transaction lock type.
            /// \param transaction  The transaction from which the fragment is to be read.
            /// \param location  The address and size of the fragment data.
            /// \returns  A pointer to the fragment instance.
            template <typename LockType>
            static std::shared_ptr<fragment> load (transaction<LockType> & transaction,
                                                   extent<fragment> const & location);

            /// Returns true if the fragment contains a section of the kind given by \p kind, false
            /// otherwise.
            /// \param kind  The section kind to check.
            /// \returns Returns true if the fragment contains a section of the kind given by
            ///   \p kind, false otherwise.
            bool has_section (section_kind const kind) const noexcept {
                return arr_.has_index (
                    static_cast<std::underlying_type<section_kind>::type> (kind));
            }

            /// \name Section Access
            ///@{

            /// Returns a const reference to the section data given the section kind. The section
            /// must exist in the fragment.
            template <section_kind Key>
            auto at () const noexcept -> typename enum_to_section<Key>::type const & {
                return at_impl<Key> (*this);
            }
            /// Returns a reference to the section data given the section kind. The section must
            /// exist in the fragment.
            template <section_kind Key>
            auto at () noexcept -> typename enum_to_section<Key>::type & {
                return at_impl<Key> (*this);
            }

            /// Returns a pointer to the section data given the section kind or nullptr if the
            /// section is not present.
            template <section_kind Key>
            auto atp () const noexcept -> typename enum_to_section<Key>::type const * {
                return atp_impl<Key> (*this);
            }
            /// Returns a pointer to the section data given the section kind or nullptr if the
            /// section is not present.
            template <section_kind Key>
            auto atp () noexcept -> typename enum_to_section<Key>::type * {
                return atp_impl<Key> (*this);
            }
            ///@}

            /// Return the number of sections in the fragment.
            std::size_t size () const noexcept { return arr_.size (); }

            /// Returns the array of section offsets.
            member_array const & members () const noexcept { return arr_; }

            /// An iterator which makes the process of iterating over the sections within
            /// a loaded fragment quite straightforward.
            class const_iterator {
                using wrapped_iterator = member_array::indices::const_iterator;
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = section_kind;
                using difference_type = wrapped_iterator::difference_type;
                using pointer = value_type const *;
                using reference = value_type const &;

                constexpr const_iterator (section_kind const kind,
                                          wrapped_iterator const & it) noexcept
                        : section_kind_{kind}
                        , it_{it} {}
                constexpr explicit const_iterator (wrapped_iterator const & it) noexcept
                        : const_iterator (static_cast<section_kind> (*it), it) {
                    assert (*it < static_cast<std::uint64_t> (section_kind::last));
                }

                bool operator== (const_iterator const & rhs) const noexcept {
                    return it_ == rhs.it_;
                }
                bool operator!= (const_iterator const & rhs) const noexcept {
                    return !operator== (rhs);
                }

                reference operator* () const noexcept { return section_kind_; }
                pointer operator-> () const noexcept { return &section_kind_; }
                const_iterator & operator++ () noexcept;
                const_iterator operator++ (int) noexcept;

            private:
                section_kind section_kind_;
                wrapped_iterator it_;
            };

            const_iterator begin () const noexcept {
                return const_iterator{arr_.get_indices ().begin ()};
            }
            const_iterator end () const noexcept {
                return const_iterator{section_kind::last, arr_.get_indices ().end ()};
            }

            /// Returns the number of bytes of storage that are required for a fragment containing
            /// the sections defined by [first, last).
            ///
            /// \tparam Iterator  An iterator which will yield values of type
            ///   pstore::repo::section_content. The values must be sorted in
            ///   pstore::repo::section_content::kind order.
            /// \param first  The beginning of the range of pstore::repo::section_content values.
            /// \param last  The end of the range of pstore::repo::section_content values.
            template <typename Iterator>
            static std::size_t size_bytes (Iterator first, Iterator last);

            /// Returns the number of bytes of storage that are accupied by this fragment.
            std::size_t size_bytes () const;

        private:
            template <typename IteratorIdx>
            constexpr fragment (IteratorIdx const first_index, IteratorIdx const last_index)
                    : arr_ (first_index, last_index) {

                // Verify that the structure layout is the same regardless of the compiler and
                // target platform.
                PSTORE_STATIC_ASSERT (alignof (fragment) == 8);
                PSTORE_STATIC_ASSERT (sizeof (fragment) == 24);
                PSTORE_STATIC_ASSERT (offsetof (fragment, signature_) == 0);
                PSTORE_STATIC_ASSERT (offsetof (fragment, arr_) == 8);

                static_assert (
                    std::numeric_limits<member_array::bitmap_type>::radix == 2,
                    "expect numeric radix to be 2 (so that 'digits' is the number of bits)");
                using utype = std::underlying_type<section_kind>::type;
                static_assert (static_cast<utype> (section_kind::last) <=
                                   std::numeric_limits<member_array::bitmap_type>::digits,
                               "section_kind does not fit in the member sparse array");
            }

            /// Returns pointer to an individual fragment instance given a function which can yield
            /// it given the object's extent.
            template <typename ReturnType, typename GetOp>
            static ReturnType load_impl (extent<fragment> const & location, GetOp get);

            template <typename Iterator>
            static void check_range_is_sorted (Iterator first, Iterator last);

            ///@{
            /// Yields a reference to section data found at a known offset within the fragment
            /// payload.
            /// \param offset The number of bytes from the start of the fragment at which
            ///   the data lies.
            template <typename InstanceType>
            InstanceType const & offset_to_instance (std::uint64_t offset) const noexcept {
                return offset_to_instance_impl<InstanceType const> (*this, offset);
            }
            template <typename InstanceType>
            InstanceType & offset_to_instance (std::uint64_t offset) noexcept {
                return offset_to_instance_impl<InstanceType> (*this, offset);
            }
            ///@}


            /// The implementation of offset_to_instance<>() (used by the const and non-const
            /// flavors).
            template <typename InstanceType, typename Fragment>
            static InstanceType & offset_to_instance_impl (Fragment && f,
                                                           std::uint64_t offset) noexcept;

            template <section_kind Key, typename InstanceType = typename enum_to_section<Key>::type>
            static std::uint64_t section_offset_is_valid (fragment const & f,
                                                          extent<fragment> const & fext,
                                                          std::uint64_t min_offset,
                                                          std::uint64_t offset, std::size_t size);


            static bool fragment_appears_valid (fragment const & f, extent<fragment> const & fext);

            /// The implementation of at<>() (used by the const and non-const flavors).
            ///
            /// \tparam Key The section_kind to be accessed.
            /// \tparam Fragment Const- or non-const fragment.
            /// \tparam ResultType The function result type, normally derived automatically from the
            ///   Key and Fragment template arguments.
            /// \param f  The fragment to be accessed.
            /// \returns A constant reference to the instance contained in the Key section if the
            ///   input fragment type is const; non-const otherwise.
            template <section_kind Key, typename Fragment,
                      typename ResultType = typename inherit_const<
                          Fragment, typename enum_to_section<Key>::type>::type>
            static ResultType & at_impl (Fragment && f) noexcept {
                assert (f.has_section (Key));
                using utype = std::underlying_type<section_kind>::type;
                return f.template offset_to_instance<ResultType> (f.arr_[static_cast<utype> (Key)]);
            }

            /// The implementation of atp<>() (used by the const and non-const flavors).
            ///
            /// \tparam Key The section_kind to be accessed.
            /// \tparam Fragment Const- or non-const fragment.
            /// \tparam ResultType The function result type, normally derived automatically from the
            ///   Key and Fragment template arguments.
            /// \param f  The fragment to be accessed.
            /// \returns A constant pointer to the instance contained in the Key section if the
            ///   input fragment type is const; non-const otherwise.
            template <section_kind Key, typename Fragment,
                      typename ResultType = typename inherit_const<
                          Fragment, typename enum_to_section<Key>::type>::type>
            static ResultType * atp_impl (Fragment && f) noexcept {
                return f.has_section (Key) ? &f.template at<Key> () : nullptr;
            }

            /// Constructs a fragment into the uninitialized memory referred to by ptr and copies
            /// the section contents [first,last) into it.
            ///
            /// \tparam Iterator  An iterator which will yield values of type
            ///   pstore::repo::section_content. The values must be sorted in
            ///   pstore::repo::section_content::kind order.
            /// \param ptr  A pointer to an uninitialized
            ///   block of memory of at least the size returned by size_bytes(first, last).
            /// \param first  The beginning of the range of pstore::repo::section_content values.
            /// \param last  The end of the range of pstore::repo::section_content values.
            template <typename Iterator>
            static void populate (void * ptr, Iterator first, Iterator last);

            static constexpr std::array<char, 8> fragment_signature_ = {
                {'F', 'r', 'a', 'g', 'm', 'e', 'n', 't'}};

            std::array<char, 8> signature_ = fragment_signature_;

            /// A sparse array of offsets to each of the contained sections. (Must be the struct's
            /// last member.)
            member_array arr_;
        };

        // operator<<
        // ~~~~~~~~~~
        template <typename OStream>
        OStream & operator<< (OStream & os, section_kind const kind) {
            char const * name = "*unknown*";
#define X(k)                                                                                       \
case section_kind::k: name = #k; break;

            switch (kind) {
                PSTORE_MCREPO_SECTION_KINDS
            case section_kind::last: assert (false); break;
            }
#undef X
            return os << name;
        }

        // populate [private, static]
        // ~~~~~~~~
        template <typename Iterator>
        void fragment::populate (void * const ptr, Iterator const first, Iterator const last) {
            // Construct the basic fragment structure into this memory.
            auto * const fragment_ptr =
                new (ptr) fragment (details::make_content_type_iterator (first),
                                    details::make_content_type_iterator (last));
            // Point past the end of the sparse array.
            auto * out = reinterpret_cast<std::uint8_t *> (fragment_ptr) +
                         offsetof (fragment, arr_) + fragment_ptr->arr_.size_bytes ();

            // Copy the contents of each of the segments to the fragment.
            std::for_each (
                first, last, [&out, &fragment_ptr](section_creation_dispatcher const & c) {
                    auto const index = static_cast<unsigned> (c.kind ());
                    out = reinterpret_cast<std::uint8_t *> (c.aligned (out));
                    fragment_ptr->arr_[index] = reinterpret_cast<std::uintptr_t> (out) -
                                                reinterpret_cast<std::uintptr_t> (fragment_ptr);
                    out = c.write (out);
                });
#ifndef NDEBUG
            {
                auto const size = fragment::size_bytes (first, last);
                assert (out >= ptr && static_cast<std::size_t> (
                                          out - reinterpret_cast<std::uint8_t *> (ptr)) == size);
                assert (size == fragment_ptr->size_bytes ());
            }
#endif
        }

        // alloc
        // ~~~~~
        template <typename Transaction, typename Iterator>
        auto fragment::alloc (Transaction & transaction, Iterator const first, Iterator const last)
            -> pstore::extent<fragment> {
            fragment::check_range_is_sorted (first, last);
            // Compute the number of bytes of storage that we'll need for this fragment.
            auto const size = fragment::size_bytes (first, last);

            // Allocate storage for the fragment including its three arrays.
            // We can't use the version of alloc_rw() which returns typed_address<> since we need
            // an explicit number of bytes allocated for the fragment.
            std::pair<std::shared_ptr<void>, pstore::address> const storage =
                transaction.alloc_rw (size, alignof (fragment));
            fragment::populate (storage.first.get (), first, last);
            return {typed_address<fragment> (storage.second), size};
        }


        // load_impl
        // ~~~~~~~~~
        template <typename ReturnType, typename GetOp>
        auto fragment::load_impl (extent<fragment> const & fext, GetOp get) -> ReturnType {
            if (fext.size < sizeof (fragment)) {
                raise (error_code::bad_fragment_record);
            }

            ReturnType f = get (fext);
            if (!fragment::fragment_appears_valid (*f, fext)) {
                raise (error_code::bad_fragment_record);
            }
            return f;
        }

        // load
        // ~~~~~
        template <typename LockType>
        auto fragment::load (transaction<LockType> & transaction,
                             pstore::extent<fragment> const & location)
            -> std::shared_ptr<fragment> {
            return load_impl<std::shared_ptr<fragment>> (
                location,
                [&transaction](extent<fragment> const & x) { return transaction.getrw (x); });
        }


        // offset_to_instance
        // ~~~~~~~~~~~~~~~~~~
        // This is the implementation used by both const and non-const flavors of
        // offset_to_instance().
        template <typename InstanceType, typename Fragment>
        inline InstanceType & fragment::offset_to_instance_impl (Fragment && f,
                                                                 std::uint64_t offset) noexcept {
            return *reinterpret_cast<InstanceType *> (
                reinterpret_cast<typename inherit_const<decltype (f), std::uint8_t>::type *> (&f) +
                offset);
        }

        // check_range_is_sorted
        // ~~~~~~~~~~~~~~~~~~~~~
        template <typename Iterator>
        void fragment::check_range_is_sorted (Iterator first, Iterator last) {
            (void) first;
            (void) last;
#ifndef NDEBUG
            using value_type = typename std::iterator_traits<Iterator>::value_type;
            assert (std::is_sorted (first, last, [](value_type const & a, value_type const & b) {
                section_kind const akind = a.kind ();
                section_kind const bkind = b.kind ();
                using utype = std::underlying_type<section_kind>::type;
                return static_cast<utype> (akind) < static_cast<utype> (bkind);
            }));
#endif
        }

        // size_bytes
        // ~~~~~~~~~~
        template <typename Iterator>
        std::size_t fragment::size_bytes (Iterator first, Iterator last) {
            fragment::check_range_is_sorted (first, last);

            auto const num_contents = std::distance (first, last);
            assert (num_contents >= 0);
            auto const unum_contents =
                static_cast<typename std::make_unsigned<decltype (num_contents)>::type> (
                    num_contents);

            // Space needed by the signature and section offset array.
            std::size_t size_bytes =
                offsetof (fragment, arr_) + decltype (fragment::arr_)::size_bytes (unum_contents);
            // Now the storage for each of the contents
            std::for_each (first, last, [&size_bytes](section_creation_dispatcher const & c) {
                size_bytes = c.aligned (size_bytes);
                size_bytes += c.size_bytes ();
            });
            return size_bytes;
        }


        // fragment::const_iterator
        // ~~~~~~~~~~~~~~~~~~~~~~~~
        inline fragment::const_iterator & fragment::const_iterator::operator++ () noexcept {
            ++it_;
            section_kind_ = static_cast<section_kind> (*it_);
            return *this;
        }
        inline fragment::const_iterator fragment::const_iterator::operator++ (int) noexcept {
            auto const prev = *this;
            ++*this;
            return prev;
        }

        /// Returns the alignment of the given section type in the given fragment.
        unsigned section_align (fragment const & fragment, section_kind kind);

        /// Returns the number of bytes of storage that are accupied by the section with the
        /// given type in the given fragment.
        std::size_t section_size (fragment const & fragment, section_kind kind);

        /// Returns the ifixups of the given section type in the given fragment.
        container<internal_fixup> section_ifixups (fragment const & fragment,
                                                            section_kind kind);

        /// Returns the xfixups of the given section type in the given fragment.
        container<external_fixup> section_xfixups (fragment const & fragment,
                                                            section_kind kind);

        /// Returns the section content of the given section type in the given fragment.
        container<std::uint8_t> section_value (fragment const & fragment,
                                                        section_kind kind);
    } // end namespace repo
} // end namespace pstore

#endif // PSTORE_MCREPO_FRAGMENT_HPP
