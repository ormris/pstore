//*    _                  *
//*   (_)___  ___  _ __   *
//*   | / __|/ _ \| '_ \  *
//*   | \__ \ (_) | | | | *
//*  _/ |___/\___/|_| |_| *
//* |__/                  *
//===- include/pstore/json/json.hpp ---------------------------------------===//
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
#ifndef PSTORE_JSON_JSON_HPP
#define PSTORE_JSON_JSON_HPP

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <stack>
#include <system_error>
#include <type_traits>

#include "pstore/json/json_error.hpp"
#include "pstore/support/gsl.hpp"
#include "pstore/support/max.hpp"
#include "pstore/support/maybe.hpp"
#include "pstore/support/portab.hpp"
#include "pstore/support/utf.hpp"

namespace pstore {
    namespace json {

        /// \brief JSON parser implementation details.
        namespace details {

            template <typename Callbacks>
            class matcher;
            template <typename Callbacks>
            class whitespace_matcher;

            template <typename Callbacks>
            struct singleton_storage;

            /// deleter is intended for use as a unique_ptr<> Deleter. It enables unique_ptr<> to be
            /// used with a mixture of heap-allocated and placement-new-allocated objects.
            template <typename T>
            class deleter {
            public:
                /// \param d True if the managed object should be deleted; false, if it only the
                /// detructor should be called.
                constexpr explicit deleter (bool const d = true) noexcept
                        : delete_{d} {}
                void operator() (T * const p) const noexcept {
                    if (delete_) {
                        delete p;
                    } else {
                        // this instance of T is in unowned memory: call the destructor but don't
                        // try to free the storage.
                        if (p) {
                            p->~T ();
                        }
                    }
                }

            private:
                bool delete_;
            };

        } // namespace details

        // clang-format off
        /// \tparam Callbacks  Should be a type containing the following members:
        ///     Signature | Description
        ///     ----------|------------
        ///     result_type | The type returned by the Callbacks::result() member function. This will be the type returned by parser<>::eof(). Should be default-constructible.
        ///     void string_value (std::string const & s) | Called when a JSON string has been parsed.
        ///     void integer_value (long v) | Called when an integer value has been parsed.
        ///     void float_value (double v) | Called when a floating-point value has been parsed.
        ///     void boolean_value (bool v) | Called when a boolean value has been parsed.
        ///     void null_value () | Called when a null value has been parsed.
        ///     void begin_array () | Called to notify the start of an array. Subsequent event notifications are for members of this array until a matching call to Callbacks::end_array().
        ///     void end_array () | Called indicate that an array has been completely parsed. This will always follow an earlier call to begin_array().
        ///     void begin_object () | Called to notify the start of an object. Subsequent event notifications are for members of this object until a matching call to Callbacks::end_object().
        ///     void end_object () | Called to indicate that an object has been completely parsed. This will always follow an earlier call to begin_object().
        ///     result_type result () const | Returns the result of the parse. If the parse was successful, this function is called by parser<>::eof() which will return its result.
        // clang-format on
        template <typename Callbacks>
        class parser {
            friend class details::matcher<Callbacks>;
            friend class details::whitespace_matcher<Callbacks>;

        public:
            using result_type = typename Callbacks::result_type;

            explicit parser (Callbacks callbacks = Callbacks ());

            ///@{
            /// Parses a chunk of JSON input. This function may be called repeatedly with portions
            /// of the source data (for example, as the data is received from an external source).
            /// Once all of the data has been received, call the parser::eof() method.

            /// A convenience function which is equivalent to calling:
            ///     input (gsl::make_span (src))
            /// \param src The data to be parsed.
            parser & input (std::string const & src) { return this->input (gsl::make_span (src)); }

            /// \param span The span of UTF-8 code units to be parsed.
            template <typename SpanType>
            parser & input (SpanType const & span);

            /// \param first The element in the half-open range of UTF-8 code-units to be parsed.
            /// \param last The end of the range of UTF-8 code-units to be parsed.
            template <typename InputIterator>
            parser & input (InputIterator first, InputIterator last);

            ///@}

            /// Informs the parser that the complete input stream has been passed by calls to
            /// parser<>::input().
            ///
            /// \returns If the parse completes successfully, Callbacks::result()
            /// is called and its result returned. If the parse failed, then a default-constructed
            /// instance of result_type is returned.
            result_type eof ();

            ///@{

            /// \returns True if the parser has signalled an error.
            bool has_error () const noexcept { return error_ != error_code::none; }
            /// \returns The error code held by the parser.
            std::error_code last_error () const noexcept { return make_error_code (error_); }

            ///@{
            Callbacks & callbacks () noexcept { return callbacks_; }
            Callbacks const & callbacks () const noexcept { return callbacks_; }
            ///@}

            std::tuple<unsigned, unsigned> coordinate () const noexcept { return coordinate_; }

        private:
            using matcher = details::matcher<Callbacks>;
            using pointer = std::unique_ptr<matcher, details::deleter<matcher>>;

            ///@{
            /// \brief Managing the column and row number (the "coordinate").

            /// Increments the column number.
            void advance_column () noexcept { ++std::get<0> (coordinate_); }

            /// Increments the row number and resets the column.
            void advance_row () noexcept {
                // The column number is set to 0. This is because the outer parse loop automatically
                // advances the column number for each character consumed. This happens after the
                // row is advanced by a matcher's consume() function.
                coordinate_ = std::make_tuple (0U, std::get<1> (coordinate_) + 1U);
            }

            /// Resets the column count but does not affect the row number.
            void reset_column () noexcept {
                coordinate_ = std::make_tuple (0U, std::get<1> (coordinate_));
            }
            ///@}

            /// Records an error for this parse. The parse will stop as soon as a non-zero error
            /// code is recorded. An error may be reported at any time during the parse; all
            /// subsequent text is ignored.
            ///
            /// \param err  The json error code to be stored in the parser.
            void set_error (error_code const err) noexcept {
                assert (error_ == error_code::none || err != error_code::none);
                error_ = err;
            }
            ///@}

            pointer make_root_matcher (bool only_string = false);
            pointer make_whitespace_matcher ();

            template <typename Matcher, typename... Args>
            pointer make_terminal_matcher (Args &&... args);

            void const * get_terminal_storage () const noexcept;

            /// Preallocated storage for "singleton" matcher. These are the matchers, such as
            /// numbers of strings, which are "terminal" and can't have child objects.
            std::unique_ptr<details::singleton_storage<Callbacks>> singletons_{
                new details::singleton_storage<Callbacks>};
            /// The maximum depth to which we allow the parse stack to grow. This value should be
            /// sufficient for any reasonable input: its intention is to prevent bogus (attack)
            /// inputs from taking the parser down.
            static constexpr std::size_t max_stack_depth_ = 200;
            /// The parse stack.
            std::stack<pointer> stack_;
            error_code error_ = error_code::none;
            Callbacks callbacks_;

            /// The column and row number of the parse within the input stream. Stored as (column,
            /// row) [i.e. (x,y)].
            std::tuple<unsigned, unsigned> coordinate_{1U, 1U};
        };

        template <typename Callbacks>
        inline parser<Callbacks> make_parser (Callbacks const & callbacks) {
            return parser<Callbacks> (callbacks);
        }

        namespace details {
            enum char_set : char { tab = '\x09', lf = '\x0A', cr = '\x0D', space = '\x20' };
            constexpr bool isspace (char const c) noexcept {
                return c == char_set::tab || c == char_set::lf || c == char_set::cr ||
                       c == char_set::space;
            }

            /// The base class for the various state machines ("matchers") which implement the
            /// various productions of the JSON grammar.
            template <typename Callbacks>
            class matcher {
            public:
                using pointer = std::unique_ptr<matcher, deleter<matcher>>;

                virtual ~matcher () = default;

                /// Called for each character as it is consumed from the input.
                ///
                /// \param parser The owning parser instance.
                /// \param ch If true, the character to be consumed. A value of nothing indicates
                /// end-of-file.
                /// \returns A pair consisting of a matcher pointer and a boolean. If non-null, the
                /// matcher is pushed onto the parse stack; if null the same matcher object is used
                /// to process the next character. The boolean value is false if the same character
                /// must be passed to the next consume() call; true indicates that the character was
                /// correctly matched by this consume() call.
                virtual std::pair<pointer, bool> consume (parser<Callbacks> & parser,
                                                          maybe<char> ch) = 0;

                /// \returns True if this matcher has completed (and reached it's "done" state). The
                /// parser will pop this instance from the parse stack before continuing.
                bool is_done () const noexcept { return state_ == done; }

            protected:
                explicit constexpr matcher (int const initial_state) noexcept
                        : state_{initial_state} {}

                constexpr int get_state () const noexcept { return state_; }
                void set_state (int const s) noexcept { state_ = s; }

                ///@{
                /// \brief Errors

                void set_error (parser<Callbacks> & parser, error_code err) noexcept {
                    parser.set_error (err);
                    if (err != error_code::none) {
                        set_state (done);
                    }
                }
                ///@}

                pointer make_root_matcher (parser<Callbacks> & parser, bool only_string = false) {
                    return parser.make_root_matcher (only_string);
                }
                pointer make_whitespace_matcher (parser<Callbacks> & parser) {
                    return parser.make_whitespace_matcher ();
                }

                template <typename Matcher, typename... Args>
                pointer make_terminal_matcher (parser<Callbacks> & parser, Args &&... args) {
                    assert (this != parser.get_terminal_storage ());
                    return parser.template make_terminal_matcher<Matcher, Args...> (
                        std::forward<Args> (args)...);
                }

                /// The value to be used for the "done" state in the each of the matcher state
                /// machines.
                static constexpr auto done = std::uint8_t{1};

            private:
                int state_;
            };

            //*  _       _             *
            //* | |_ ___| |_____ _ _   *
            //* |  _/ _ \ / / -_) ' \  *
            //*  \__\___/_\_\___|_||_| *
            //*                        *
            /// A matcher which checks for a specific keyword such as "true", "false", or "null".
            /// \tparam Callbacks  The parser callback structure.
            /// \tparam DoneFunction  A function matching the signature void(parser<Callbacks>&)
            ///   that will be called when the token is successfully matched.
            template <typename Callbacks, typename DoneFunction>
            class token_matcher : public matcher<Callbacks> {
            public:
                /// \param text  The string to be matched.
                /// \param done_fn  The function called when the source string is matched.
                explicit token_matcher (gsl::czstring const text,
                                        DoneFunction done_fn = DoneFunction ()) noexcept
                        : matcher<Callbacks> (start_state)
                        , text_ (text)
                        , done_ (std::move (done_fn)) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    last_state,
                };

                /// The keyword to be matched. The input sequence must exactly match this string or
                /// an unrecognized token error is raised. Once all of the characters are matched,
                /// complete() is called.
                gsl::czstring text_;

                /// This function is called once the complete token text has been matched.
                DoneFunction done_;
            };

            template <typename Callbacks, typename DoneFunction>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            token_matcher<Callbacks, DoneFunction>::consume (parser<Callbacks> & parser,
                                                             maybe<char> ch) {
                bool match = true;
                switch (this->get_state ()) {
                case start_state:
                    if (!ch || *ch != *text_) {
                        this->set_error (parser, error_code::unrecognized_token);
                    } else {
                        ++text_;
                        if (*text_ == '\0') {
                            // We've run out of input text, so ensure that the next character isn't
                            // alpha-numeric.
                            this->set_state (last_state);
                        }
                    }
                    break;
                case last_state:
                    if (ch) {
                        if (std::isalnum (*ch)) {
                            this->set_error (parser, error_code::unrecognized_token);
                            return {nullptr, true};
                        }
                        match = false;
                    }
                    done_ (parser);
                    this->set_state (done_state);
                    break;
                case done_state: assert (false); break;
                }
                return {nullptr, match};
            }

            //*   __      _           _       _             *
            //*  / _|__ _| |___ ___  | |_ ___| |_____ _ _   *
            //* |  _/ _` | (_-</ -_) |  _/ _ \ / / -_) ' \  *
            //* |_| \__,_|_/__/\___|  \__\___/_\_\___|_||_| *
            //*                                             *

            struct false_complete {
                template <typename Callbacks>
                void operator() (parser<Callbacks> & parser) const {
                    parser.callbacks ().boolean_value (false);
                }
            };

            template <typename Callbacks>
            using false_token_matcher = token_matcher<Callbacks, false_complete>;

            //*  _                  _       _             *
            //* | |_ _ _ _  _ ___  | |_ ___| |_____ _ _   *
            //* |  _| '_| || / -_) |  _/ _ \ / / -_) ' \  *
            //*  \__|_|  \_,_\___|  \__\___/_\_\___|_||_| *
            //*                                           *

            struct true_complete {
                template <typename Callbacks>
                void operator() (parser<Callbacks> & parser) const {
                    parser.callbacks ().boolean_value (true);
                }
            };

            template <typename Callbacks>
            using true_token_matcher = token_matcher<Callbacks, true_complete>;

            //*           _ _   _       _             *
            //*  _ _ _  _| | | | |_ ___| |_____ _ _   *
            //* | ' \ || | | | |  _/ _ \ / / -_) ' \  *
            //* |_||_\_,_|_|_|  \__\___/_\_\___|_||_| *
            //*                                       *

            struct null_complete {
                template <typename Callbacks>
                void operator() (parser<Callbacks> & parser) const {
                    parser.callbacks ().null_value ();
                }
            };

            template <typename Callbacks>
            using null_token_matcher = token_matcher<Callbacks, null_complete>;

            //*                 _              *
            //*  _ _ _  _ _ __ | |__  ___ _ _  *
            //* | ' \ || | '  \| '_ \/ -_) '_| *
            //* |_||_\_,_|_|_|_|_.__/\___|_|   *
            //*                                *
            // Grammar (from RFC 7159, March 2014)
            //     number = [ minus ] int [ frac ] [ exp ]
            //     decimal-point = %x2E       ; .
            //     digit1-9 = %x31-39         ; 1-9
            //     e = %x65 / %x45            ; e E
            //     exp = e [ minus / plus ] 1*DIGIT
            //     frac = decimal-point 1*DIGIT
            //     int = zero / ( digit1-9 *DIGIT )
            //     minus = %x2D               ; -
            //     plus = %x2B                ; +
            //     zero = %x30                ; 0
            template <typename Callbacks>
            class number_matcher final : public matcher<Callbacks> {
            public:
                number_matcher () noexcept
                        : matcher<Callbacks> (leading_minus_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                bool in_terminal_state () const;

                bool do_leading_minus_state (parser<Callbacks> & parser, char c);
                /// Implements the first character of the 'int' production.
                bool do_integer_initial_digit_state (parser<Callbacks> & parser, char c);
                bool do_integer_digit_state (parser<Callbacks> & parser, char c);
                bool do_frac_state (parser<Callbacks> & parser, char c);
                bool do_frac_digit_state (parser<Callbacks> & parser, char c);
                bool do_exponent_sign_state (parser<Callbacks> & parser, char c);
                bool do_exponent_digit_state (parser<Callbacks> & parser, char c);

                void complete (parser<Callbacks> & parser);
                void number_is_float ();

                void make_result (parser<Callbacks> & parser);

                enum state {
                    done_state = matcher<Callbacks>::done,
                    leading_minus_state,
                    integer_initial_digit_state,
                    integer_digit_state,
                    frac_state,
                    frac_initial_digit_state,
                    frac_digit_state,
                    exponent_sign_state,
                    exponent_initial_digit_state,
                    exponent_digit_state,
                };

                bool is_neg_ = false;
                bool is_integer_ = true;
                unsigned long int_acc_ = 0UL;

                struct {
                    double frac_part = 0.0;
                    double frac_scale = 1.0;
                    double whole_part = 0.0;

                    bool exp_is_negative = false;
                    unsigned exponent = 0;
                } fp_acc_;
            };

            // number_is_float
            // ~~~~~~~~~~~~~~~
            template <typename Callbacks>
            void number_matcher<Callbacks>::number_is_float () {
                if (is_integer_) {
                    fp_acc_.whole_part = int_acc_;
                    is_integer_ = false;
                }
            }

            // in_terminal_state
            // ~~~~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::in_terminal_state () const {
                switch (this->get_state ()) {
                case integer_digit_state:
                case frac_state:
                case frac_digit_state:
                case exponent_digit_state:
                case done_state: return true;
                default: return false;
                }
            }

            // leading_minus_state
            // ~~~~~~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_leading_minus_state (parser<Callbacks> & parser,
                                                                    char c) {
                bool match = true;
                if (c == '-') {
                    this->set_state (integer_initial_digit_state);
                    is_neg_ = true;
                } else if (c >= '0' && c <= '9') {
                    this->set_state (integer_initial_digit_state);
                    match = do_integer_initial_digit_state (parser, c);
                } else {
                    // minus MUST be followed by the 'int' production.
                    this->set_error (parser, error_code::number_out_of_range);
                }
                return match;
            }

            // frac_state
            // ~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_frac_state (parser<Callbacks> & parser,
                                                           char const c) {
                bool match = true;
                if (c == '.') {
                    this->set_state (frac_initial_digit_state);
                } else if (c == 'e' || c == 'E') {
                    this->set_state (exponent_sign_state);
                } else if (c >= '0' && c <= '9') {
                    // digits are definitely not part of the next token so we can issue an error
                    // right here.
                    this->set_error (parser, error_code::number_out_of_range);
                } else {
                    // the 'frac' production is optional.
                    match = false;
                    this->complete (parser);
                }
                return match;
            }

            // frac_digit
            // ~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_frac_digit_state (parser<Callbacks> & parser,
                                                                 char const c) {
                assert (this->get_state () == frac_initial_digit_state ||
                        this->get_state () == frac_digit_state);

                bool match = true;
                if (c == 'e' || c == 'E') {
                    this->number_is_float ();
                    if (this->get_state () == frac_initial_digit_state) {
                        this->set_error (parser, error_code::unrecognized_token);
                    } else {
                        this->set_state (exponent_sign_state);
                    }
                } else if (c >= '0' && c <= '9') {
                    this->number_is_float ();
                    fp_acc_.frac_part = fp_acc_.frac_part * 10 + (c - '0');
                    fp_acc_.frac_scale *= 10;

                    this->set_state (frac_digit_state);
                } else {
                    if (this->get_state () == frac_initial_digit_state) {
                        this->set_error (parser, error_code::unrecognized_token);
                    } else {
                        match = false;
                        this->complete (parser);
                    }
                }
                return match;
            }

            // exponent_sign_state
            // ~~~~~~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_exponent_sign_state (parser<Callbacks> & parser,
                                                                    char c) {
                bool match = true;
                this->number_is_float ();
                this->set_state (exponent_initial_digit_state);
                switch (c) {
                case '+': fp_acc_.exp_is_negative = false; break;
                case '-': fp_acc_.exp_is_negative = true; break;
                default: match = this->do_exponent_digit_state (parser, c); break;
                }
                return match;
            }

            // complete
            // ~~~~~~~~
            template <typename Callbacks>
            void number_matcher<Callbacks>::complete (parser<Callbacks> & parser) {
                this->set_state (done_state);
                this->make_result (parser);
            }

            // exponent_digit
            // ~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_exponent_digit_state (parser<Callbacks> & parser,
                                                                     char const c) {
                assert (this->get_state () == exponent_digit_state ||
                        this->get_state () == exponent_initial_digit_state);
                assert (!is_integer_);

                bool match = true;
                if (c >= '0' && c <= '9') {
                    fp_acc_.exponent = fp_acc_.exponent * 10U + static_cast<unsigned> (c - '0');
                    this->set_state (exponent_digit_state);
                } else {
                    if (this->get_state () == exponent_initial_digit_state) {
                        this->set_error (parser, error_code::unrecognized_token);
                    } else {
                        match = false;
                        this->complete (parser);
                    }
                }
                return match;
            }

            // do_integer_initial_digit_state
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool
            number_matcher<Callbacks>::do_integer_initial_digit_state (parser<Callbacks> & parser,
                                                                       char const c) {
                assert (this->get_state () == integer_initial_digit_state);
                assert (is_integer_);
                if (c == '0') {
                    this->set_state (frac_state);
                } else if (c >= '1' && c <= '9') {
                    assert (int_acc_ == 0);
                    int_acc_ = static_cast<unsigned> (c - '0');
                    this->set_state (integer_digit_state);
                } else {
                    this->set_error (parser, error_code::unrecognized_token);
                }
                return true;
            }

            // do_integer_digit_state
            // ~~~~~~~~~~~~~~~~~~~~~~
            template <typename Callbacks>
            bool number_matcher<Callbacks>::do_integer_digit_state (parser<Callbacks> & parser,
                                                                    char const c) {
                assert (this->get_state () == integer_digit_state);
                assert (is_integer_);

                bool match = true;
                if (c == '.') {
                    this->set_state (frac_initial_digit_state);
                    number_is_float ();
                } else if (c == 'e' || c == 'E') {
                    this->set_state (exponent_sign_state);
                    number_is_float ();
                } else if (c >= '0' && c <= '9') {
                    if (int_acc_ > std::numeric_limits<decltype (int_acc_)>::max () / 10U + 10U) {
                        this->set_error (parser, error_code::number_out_of_range);
                    } else {
                        int_acc_ = int_acc_ * 10U + static_cast<unsigned> (c - '0');
                    }
                } else {
                    match = false;
                    this->complete (parser);
                }
                return match;
            }

            // consume
            // ~~~~~~~
            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            number_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                bool match = true;
                if (ch) {
                    char const c = *ch;
                    switch (this->get_state ()) {
                    case leading_minus_state:
                        match = this->do_leading_minus_state (parser, c);
                        break;
                    case integer_initial_digit_state:
                        match = this->do_integer_initial_digit_state (parser, c);
                        break;
                    case integer_digit_state:
                        match = this->do_integer_digit_state (parser, c);
                        break;
                    case frac_state: match = this->do_frac_state (parser, c); break;
                    case frac_initial_digit_state:
                    case frac_digit_state: match = this->do_frac_digit_state (parser, c); break;
                    case exponent_sign_state:
                        match = this->do_exponent_sign_state (parser, c);
                        break;
                    case exponent_initial_digit_state:
                    case exponent_digit_state:
                        match = this->do_exponent_digit_state (parser, c);
                        break;
                    case done_state: assert (false); break;
                    }
                } else {
                    assert (!parser.has_error ());
                    if (!this->in_terminal_state ()) {
                        this->set_error (parser, error_code::expected_digits);
                    }
                    this->complete (parser);
                }
                return {nullptr, match};
            }

            template <typename Callbacks>
            void number_matcher<Callbacks>::make_result (parser<Callbacks> & parser) {
                if (parser.has_error ()) {
                    return;
                }
                assert (this->in_terminal_state ());

                if (is_integer_) {
                    using acc_type = typename std::make_signed<decltype (int_acc_)>::type;
                    using uacc_type = typename std::make_unsigned<decltype (int_acc_)>::type;

                    constexpr auto max = std::numeric_limits<acc_type>::max ();
                    constexpr auto min = std::numeric_limits<acc_type>::min ();
                    constexpr auto umin = static_cast<uacc_type> (min);

                    if (is_neg_ ? int_acc_ > umin : int_acc_ > max) {
                        this->set_error (parser, error_code::number_out_of_range);
                        return;
                    }

                    long lv;
                    if (is_neg_) {
                        lv = (int_acc_ == umin) ? min : -static_cast<acc_type> (int_acc_);
                    } else {
                        lv = static_cast<long> (int_acc_);
                    }
                    parser.callbacks ().integer_value (lv);
                    return;
                }

                auto xf = (fp_acc_.whole_part + fp_acc_.frac_part / fp_acc_.frac_scale);
                auto exp = std::pow (10, fp_acc_.exponent);
                if (std::isinf (exp)) {
                    this->set_error (parser, error_code::number_out_of_range);
                    return;
                }
                if (fp_acc_.exp_is_negative) {
                    exp = 1.0 / exp;
                }

                xf *= exp;
                if (is_neg_) {
                    xf = -xf;
                }

                if (std::isinf (xf) || std::isnan (xf)) {
                    this->set_error (parser, error_code::number_out_of_range);
                    return;
                }
                parser.callbacks ().float_value (xf);
            }


            //*     _       _            *
            //*  __| |_ _ _(_)_ _  __ _  *
            //* (_-<  _| '_| | ' \/ _` | *
            //* /__/\__|_| |_|_||_\__, | *
            //*                   |___/  *
            template <typename Callbacks>
            class string_matcher final : public matcher<Callbacks> {
            public:
                string_matcher () noexcept
                        : matcher<Callbacks> (start_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    normal_char_state,
                    escape_state,
                    hex1_state,
                    hex2_state,
                    hex3_state,
                    hex4_state,
                };

                class appender {
                public:
                    bool append32 (char32_t code_point);
                    bool append16 (char16_t cu);
                    std::string && result () { return std::move (result_); }
                    bool has_high_surrogate () const noexcept { return high_surrogate_ != 0; }

                private:
                    std::string result_;
                    char16_t high_surrogate_ = 0;
                };

                static std::tuple<state, error_code>
                consume_normal_state (parser<Callbacks> & parser, char32_t code_point,
                                      appender & app);

                static maybe<unsigned> hex_value (char32_t c, unsigned value);

                static maybe<std::tuple<unsigned, state>>
                consume_hex_state (unsigned hex, enum state state, char32_t code_point);

                static std::tuple<state, error_code> consume_escape_state (char32_t code_point,
                                                                           appender & app);

                utf::utf8_decoder decoder_;
                appender app_;
                unsigned hex_ = 0U;
            };

            template <typename Callbacks>
            bool string_matcher<Callbacks>::appender::append32 (char32_t code_point) {
                bool ok = true;
                if (this->has_high_surrogate ()) {
                    // A high surrogate followed by something other than a low surrogate.
                    ok = false;
                } else {
                    utf::code_point_to_utf8<char> (code_point, std::back_inserter (result_));
                }
                return ok;
            }

            template <typename Callbacks>
            bool string_matcher<Callbacks>::appender::append16 (char16_t cu) {
                bool ok = true;
                if (utf::is_utf16_high_surrogate (cu)) {
                    if (!this->has_high_surrogate ()) {
                        high_surrogate_ = cu;
                    } else {
                        // A high surrogate following another high surrogate.
                        ok = false;
                    }
                } else if (utf::is_utf16_low_surrogate (cu)) {
                    if (!this->has_high_surrogate ()) {
                        // A low surrogate following by something other than a high surrogate.
                        ok = false;
                    } else {
                        char16_t const surrogates[] = {high_surrogate_, cu};
                        auto first = std::begin (surrogates);
                        auto last = std::end (surrogates);
                        auto code_point = char32_t{0};
                        std::tie (first, code_point) =
                            utf::utf16_to_code_point (first, last, utf::nop_swapper);
                        utf::code_point_to_utf8 (code_point, std::back_inserter (result_));
                        high_surrogate_ = 0;
                    }
                } else {
                    if (this->has_high_surrogate ()) {
                        // A high surrogate followed by something other than a low surrogate.
                        ok = false;
                    } else {
                        auto const code_point = static_cast<char32_t> (cu);
                        utf::code_point_to_utf8 (code_point, std::back_inserter (result_));
                    }
                }
                return ok;
            }

            // [static]
            template <typename Callbacks>
            auto string_matcher<Callbacks>::consume_normal_state (parser<Callbacks> & parser,
                                                                  char32_t code_point,
                                                                  appender & app)
                -> std::tuple<state, error_code> {
                state next_state = normal_char_state;
                error_code error = error_code::none;

                if (code_point == '"') {
                    if (app.has_high_surrogate ()) {
                        error = error_code::bad_unicode_code_point;
                    } else {
                        // Consume the closing quote character.
                        parser.callbacks ().string_value (app.result ());
                    }
                    next_state = done_state;
                } else if (code_point == '\\') {
                    next_state = escape_state;
                } else if (code_point <= 0x1F) {
                    // Control characters U+0000 through U+001F MUST be escaped.
                    error = error_code::bad_unicode_code_point;
                } else {
                    if (!app.append32 (code_point)) {
                        error = error_code::bad_unicode_code_point;
                    }
                }

                return std::make_tuple (next_state, error);
            }

            // [static]
            template <typename Callbacks>
            maybe<unsigned> string_matcher<Callbacks>::hex_value (char32_t const c,
                                                                  unsigned const value) {
                auto digit = 0U;
                if (c >= '0' && c <= '9') {
                    digit = static_cast<unsigned> (c) - '0';
                } else if (c >= 'a' && c <= 'f') {
                    digit = static_cast<unsigned> (c) - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    digit = static_cast<unsigned> (c) - 'A' + 10;
                } else {
                    return nothing<unsigned> ();
                }
                return just (16 * value + digit);
            }

            // [static]
            template <typename Callbacks>
            auto string_matcher<Callbacks>::consume_hex_state (unsigned const hex,
                                                               enum state const state,
                                                               char32_t const code_point)
                -> maybe<std::tuple<unsigned, enum state>> {

                    return hex_value (code_point, hex) >>=
                           [state](unsigned value) {
                               assert (value <= std::numeric_limits<std::uint16_t>::max ());
                               auto next_state = state;
                               switch (state) {
                               case hex1_state: next_state = hex2_state; break;
                               case hex2_state: next_state = hex3_state; break;
                               case hex3_state: next_state = hex4_state; break;
                               case hex4_state: next_state = normal_char_state; break;

                               case start_state:
                               case normal_char_state:
                               case escape_state:
                               case done_state:
                                   assert (false);
                                   return nothing<std::tuple<unsigned, enum state>> ();
                               }

                               return just (std::make_tuple (value, next_state));
                           };
                }

            // [static]
            template <typename Callbacks>
            auto string_matcher<Callbacks>::consume_escape_state (char32_t code_point, appender & app)
                -> std::tuple<state, error_code> {

                auto decode = [](char32_t cp) {
                    state next_state = normal_char_state;
                    switch (cp) {
                    case '"': cp = '"'; break;
                    case '\\': cp = '\\'; break;
                    case '/': cp = '/'; break;
                    case 'b': cp = '\b'; break;
                    case 'f': cp = '\f'; break;
                    case 'n': cp = '\n'; break;
                    case 'r': cp = '\r'; break;
                    case 't': cp = '\t'; break;
                    case 'u': next_state = hex1_state; break;
                    default: return nothing<std::tuple<char32_t, state>> ();
                    }
                    return just (std::make_tuple (cp, next_state));
                };

                auto append = [&app](std::tuple<char32_t, state> const & s) {
                    char32_t const cp = std::get<0> (s);
                    state const next_state = std::get<1> (s);
                    assert (next_state == normal_char_state || next_state == hex1_state);
                    if (next_state == normal_char_state) {
                        if (!app.append32 (cp)) {
                            return nothing<state> ();
                        }
                    }
                    return just (next_state);
                };

                maybe<state> const x = decode (code_point) >>= append;
                return x ? std::make_tuple (*x, error_code::none)
                         : std::make_tuple (normal_char_state, error_code::invalid_escape_char);
            }

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            string_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                if (!ch) {
                    this->set_error (parser, error_code::expected_close_quote);
                    return {nullptr, true};
                }

                if (maybe<char32_t> const code_point =
                        decoder_.get (static_cast<std::uint8_t> (*ch))) {
                    switch (this->get_state ()) {
                    // Matches the opening quote.
                    case start_state:
                        if (*code_point == '"') {
                            assert (!app_.has_high_surrogate ());
                            this->set_state (normal_char_state);
                        } else {
                            this->set_error (parser, error_code::expected_token);
                        }
                        break;
                    case normal_char_state: {
                        auto const normal_resl =
                            string_matcher::consume_normal_state (parser, *code_point, app_);
                        this->set_state (std::get<0> (normal_resl));
                        this->set_error (parser, std::get<1> (normal_resl));
                    } break;

                    case escape_state: {
                        auto const escape_resl =
                            string_matcher::consume_escape_state (*code_point, app_);
                        this->set_state (std::get<0> (escape_resl));
                        this->set_error (parser, std::get<1> (escape_resl));
                    } break;

                    case hex1_state: hex_ = 0; PSTORE_FALLTHROUGH;
                    case hex2_state:
                    case hex3_state:
                    case hex4_state: {
                        maybe<std::tuple<unsigned, state>> const hex_resl =
                            string_matcher::consume_hex_state (
                                hex_, static_cast<state> (this->get_state ()), *code_point);
                        if (!hex_resl) {
                            this->set_error (parser, error_code::invalid_hex_char);
                            break;
                        }
                        hex_ = std::get<0> (*hex_resl);
                        state const next_state = std::get<1> (*hex_resl);
                        this->set_state (next_state);
                        // We're done with the hex characters and are switching back to the "normal"
                        // state. The means that we can add the accumulated code-point (in hex_) to
                        // the string.
                        if (next_state == normal_char_state) {
                            if (!app_.append16 (static_cast<char16_t> (hex_))) {
                                this->set_error (parser, error_code::bad_unicode_code_point);
                            }
                        }
                    } break;

                    case done_state: assert (false); break;
                    }
                }
                return {nullptr, true};
            }


            template <typename Callbacks>
            class root_matcher;

            //*                          *
            //*  __ _ _ _ _ _ __ _ _  _  *
            //* / _` | '_| '_/ _` | || | *
            //* \__,_|_| |_| \__,_|\_, | *
            //*                    |__/  *
            template <typename Callbacks>
            class array_matcher final : public matcher<Callbacks> {
            public:
                array_matcher () noexcept
                        : matcher<Callbacks> (start_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    first_object_state,
                    object_state,
                    comma_state,
                };

                void end_array (parser<Callbacks> & parser);
            };

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            array_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                if (!ch) {
                    this->set_error (parser, error_code::expected_array_member);
                    return {nullptr, true};
                }
                char const c = *ch;
                switch (this->get_state ()) {
                case start_state:
                    assert (c == '[');
                    parser.callbacks ().begin_array ();
                    this->set_state (first_object_state);
                    // Match this character and consume whitespace before the object (or close
                    // bracket).
                    return {this->make_whitespace_matcher (parser), true};

                case first_object_state:
                    if (c == ']') {
                        this->end_array (parser);
                        break;
                    }
                    PSTORE_FALLTHROUGH;
                case object_state:
                    this->set_state (comma_state);
                    return {this->make_root_matcher (parser), false};
                    break;
                case comma_state:
                    if (isspace (c)) {
                        // just consume whitespace before a comma.
                        return {this->make_whitespace_matcher (parser), false};
                    }
                    switch (c) {
                    case ',': this->set_state (object_state); break;
                    case ']': this->end_array (parser); break;
                    default: this->set_error (parser, error_code::expected_array_member); break;
                    }
                    break;
                case done_state: assert (false); break;
                }
                return {nullptr, true};
            }

            template <typename Callbacks>
            void array_matcher<Callbacks>::end_array (parser<Callbacks> & parser) {
                parser.callbacks ().end_array ();
                this->set_state (done_state);
            }

            //*      _     _        _    *
            //*  ___| |__ (_)___ __| |_  *
            //* / _ \ '_ \| / -_) _|  _| *
            //* \___/_.__// \___\__|\__| *
            //*         |__/             *
            template <typename Callbacks>
            class object_matcher final : public matcher<Callbacks> {
            public:
                object_matcher () noexcept
                        : matcher<Callbacks> (start_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    first_key_state,
                    key_state,
                    colon_state,
                    value_state,
                    comma_state,
                };
            };

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            object_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                if (this->get_state () == done_state) {
                    assert (parser.last_error () != make_error_code (error_code::none));
                    return {nullptr, true};
                }
                if (!ch) {
                    this->set_error (parser, error_code::expected_object_member);
                    return {nullptr, true};
                }
                char const c = *ch;
                switch (this->get_state ()) {
                case start_state:
                    assert (c == '{');
                    this->set_state (first_key_state);
                    parser.callbacks ().begin_object ();
                    return {this->make_whitespace_matcher (parser), true};

                case first_key_state:
                    if (c == '}') {
                        parser.callbacks ().end_object ();
                        this->set_state (done_state);
                        break;
                    }
                    PSTORE_FALLTHROUGH;
                case key_state:
                    this->set_state (colon_state);
                    return {this->make_root_matcher (parser, true /*only string allowed*/), false};
                case colon_state:
                    if (isspace (c)) {
                        // just consume whitespace before the colon.
                        return {this->make_whitespace_matcher (parser), false};
                    }
                    if (c == ':') {
                        this->set_state (value_state);
                    } else {
                        this->set_error (parser, error_code::expected_colon);
                    }
                    break;
                case value_state:
                    this->set_state (comma_state);
                    return {this->make_root_matcher (parser), false};
                case comma_state:
                    if (isspace (c)) {
                        // just consume whitespace before the comma.
                        return {this->make_whitespace_matcher (parser), false};
                    }
                    if (c == ',') {
                        this->set_state (key_state);
                    } else if (c == '}') {
                        parser.callbacks ().end_object ();
                        this->set_state (done_state);
                    } else {
                        this->set_error (parser, error_code::expected_object_member);
                    }
                    break;
                case done_state: assert (false); break;
                }
                return {nullptr, true};
            }

            //*             *
            //* __ __ _____ *
            //* \ V  V (_-< *
            //*  \_/\_//__/ *
            //*             *
            /// This matcher consumes whitespace and updates the row number in response to the
            /// various combinations of CR and LF.
            template <typename Callbacks>
            class whitespace_matcher final : public matcher<Callbacks> {
            public:
                whitespace_matcher () noexcept
                        : matcher<Callbacks> (start_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    crlf_state,
                };
            };

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            whitespace_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                if (!ch) {
                    this->set_state (done_state);
                } else {
                    char const c = *ch;
                    switch (this->get_state ()) {
                    case crlf_state:
                        this->set_state (start_state);
                        if (c == details::char_set::lf) {
                            parser.reset_column ();
                            break;
                        }
                        PSTORE_FALLTHROUGH;
                    case start_state:
                        switch (c) {
                        case details::char_set::space: break; // Just consume.
                        case details::char_set::tab:
                            // TODO: tab expansion.
                            break;
                        case details::char_set::cr:
                            parser.advance_row ();
                            this->set_state (crlf_state);
                            break;
                        case details::char_set::lf: parser.advance_row (); break;
                        default:
                            // Stop, pop this matcher, and retry with the same character.
                            this->set_state (done_state);
                            return {nullptr, false};
                        }
                        break;

                    case done_state: assert (false); break;
                    }
                }
                return {nullptr, true};
            }

            //*           __  *
            //*  ___ ___ / _| *
            //* / -_) _ \  _| *
            //* \___\___/_|   *
            //*               *
            template <typename Callbacks>
            class eof_matcher final : public matcher<Callbacks> {
            public:
                eof_matcher () noexcept
                        : matcher<Callbacks> (start_state) {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                };
            };

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            eof_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> const ch) {
                if (ch) {
                    this->set_error (parser, error_code::unexpected_extra_input);
                } else {
                    this->set_state (done_state);
                }
                return {nullptr, true};
            }

            //*               _                _      _             *
            //*  _ _ ___  ___| |_   _ __  __ _| |_ __| |_  ___ _ _  *
            //* | '_/ _ \/ _ \  _| | '  \/ _` |  _/ _| ' \/ -_) '_| *
            //* |_| \___/\___/\__| |_|_|_\__,_|\__\__|_||_\___|_|   *
            //*                                                     *
            template <typename Callbacks>
            class root_matcher final : public matcher<Callbacks> {
            public:
                explicit constexpr root_matcher (bool const only_string = false) noexcept
                        : matcher<Callbacks> (start_state)
                        , only_string_{only_string} {}

                std::pair<typename matcher<Callbacks>::pointer, bool>
                consume (parser<Callbacks> & parser, maybe<char> ch) override;

            private:
                enum state {
                    done_state = matcher<Callbacks>::done,
                    start_state,
                    new_token_state,
                };
                bool const only_string_;
            };

            template <typename Callbacks>
            std::pair<typename matcher<Callbacks>::pointer, bool>
            root_matcher<Callbacks>::consume (parser<Callbacks> & parser, maybe<char> ch) {
                if (!ch) {
                    this->set_error (parser, error_code::expected_token);
                    return {nullptr, true};
                }

                switch (this->get_state ()) {
                case start_state:
                    this->set_state (new_token_state);
                    return {this->make_whitespace_matcher (parser), false};

                case new_token_state: {
                    if (only_string_ && *ch != '"') {
                        this->set_error (parser, error_code::expected_string);
                        // Don't return here in order to allow the switch default to produce a
                        // different error code for a bad token.
                    }
                    this->set_state (done_state);
                    switch (*ch) {
                    case '-':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        return {this->template make_terminal_matcher<number_matcher<Callbacks>> (
                                    parser),
                                false};
                    case '"':
                        return {this->template make_terminal_matcher<string_matcher<Callbacks>> (
                                    parser),
                                false};
                    case 't':
                        return {
                            this->template make_terminal_matcher<true_token_matcher<Callbacks>> (
                                parser, "true"),
                            false};
                    case 'f':
                        return {
                            this->template make_terminal_matcher<false_token_matcher<Callbacks>> (
                                parser, "false"),
                            false};
                    case 'n':
                        return {
                            this->template make_terminal_matcher<null_token_matcher<Callbacks>> (
                                parser, "null"),
                            false};
                    case '[':
                        return {
                            typename matcher<Callbacks>::pointer (new array_matcher<Callbacks> ()),
                            false};
                    case '{':
                        return {
                            typename matcher<Callbacks>::pointer (new object_matcher<Callbacks> ()),
                            false};
                    default:
                        this->set_error (parser, error_code::expected_token);
                        return {nullptr, true};
                    }
                } break;
                case done_state: assert (false); break;
                }
                assert (false); // unreachable.
                return {nullptr, true};
            }


            //*     _           _     _                 _                          *
            //*  __(_)_ _  __ _| |___| |_ ___ _ _    __| |_ ___ _ _ __ _ __ _ ___  *
            //* (_-< | ' \/ _` | / -_)  _/ _ \ ' \  (_-<  _/ _ \ '_/ _` / _` / -_) *
            //* /__/_|_||_\__, |_\___|\__\___/_||_| /__/\__\___/_| \__,_\__, \___| *
            //*           |___/                                         |___/      *
            template <typename Callbacks>
            struct singleton_storage {
                template <typename T>
                struct storage {
                    using type = typename std::aligned_storage<sizeof (T), alignof (T)>::type;
                };

                typename storage<eof_matcher<Callbacks>>::type eof;
                typename storage<whitespace_matcher<Callbacks>>::type trailing_ws;
                typename storage<root_matcher<Callbacks>>::type root;

                using matcher_characteristics =
                    characteristics<number_matcher<Callbacks>, string_matcher<Callbacks>,
                                    true_token_matcher<Callbacks>, false_token_matcher<Callbacks>,
                                    null_token_matcher<Callbacks>, whitespace_matcher<Callbacks>>;

                typename std::aligned_storage<matcher_characteristics::size,
                                              matcher_characteristics::align>::type terminal;
            };

            /// Returns a default-initialized instance of type T.
            template <typename T>
            struct default_return {
                static T get () { return T{}; }
            };
            template <>
            struct default_return<void> {
                static void get () { return; }
            };
        } // namespace details


        // (ctor)
        // ~~~~~~
        template <typename Callbacks>
        parser<Callbacks>::parser (Callbacks callbacks)
                : callbacks_ (std::move (callbacks)) {

            using mpointer = typename matcher::pointer;
            using deleter = typename mpointer::deleter_type;
            // The EOF matcher is placed at the bottom of the stack to ensure that the input JSON
            // ends after a single top-level object.
            stack_.push (mpointer (new (&singletons_->eof) details::eof_matcher<Callbacks>{},
                                   deleter{false}));
            // We permit whitespace after the top-level object.
            stack_.push (mpointer (new (&singletons_->trailing_ws)
                                       details::whitespace_matcher<Callbacks>{},
                                   deleter{false}));
            stack_.push (this->make_root_matcher ());
        }

        // make_root_matcher
        // ~~~~~~~~~~~~~~~~~
        template <typename Callbacks>
        auto parser<Callbacks>::make_root_matcher (bool only_string_allowed) -> pointer {
            using root_matcher = details::root_matcher<Callbacks>;
            return pointer (new (&singletons_->root) root_matcher (only_string_allowed),
                            typename pointer::deleter_type{false});
        }

        // make_whitespace_matcher
        // ~~~~~~~~~~~~~~~~~~~~~~~
        template <typename Callbacks>
        auto parser<Callbacks>::make_whitespace_matcher () -> pointer {
            using whitespace_matcher = details::whitespace_matcher<Callbacks>;
            return this->make_terminal_matcher<whitespace_matcher> ();
        }

        // get_terminal_storage
        // ~~~~~~~~~~~~~~~~~~~~
        template <typename Callbacks>
        void const * parser<Callbacks>::get_terminal_storage () const noexcept {
            return &singletons_->terminal;
        }

        // make_terminal_matcher
        // ~~~~~~~~~~~~~~~~~~~~~
        template <typename Callbacks>
        template <typename Matcher, typename... Args>
        auto parser<Callbacks>::make_terminal_matcher (Args &&... args) -> pointer {
            static_assert (sizeof (Matcher) <= sizeof (decltype (singletons_->terminal)),
                           "terminal storage is not large enough for Matcher type");
            static_assert (alignof (Matcher) <= alignof (decltype (singletons_->terminal)),
                           "terminal storage is not sufficiently aligned for Matcher type");

            return pointer (new (&singletons_->terminal) Matcher (std::forward<Args> (args)...),
                            typename pointer::deleter_type{false});
        }

        // input
        // ~~~~~
        template <typename Callbacks>
        template <typename SpanType>
        auto parser<Callbacks>::input (SpanType const & span) -> parser & {
            static_assert (
                std::is_same<typename std::remove_cv<typename SpanType::element_type>::type,
                             char>::value,
                "span element type must be char");
            return this->input (std::begin (span), std::end (span));
        }

        template <typename Callbacks>
        template <typename InputIterator>
        auto parser<Callbacks>::input (InputIterator first, InputIterator last) -> parser & {
            static_assert (
                std::is_same<typename std::remove_cv<
                                 typename std::iterator_traits<InputIterator>::value_type>::type,
                             char>::value,
                "iterator value_type must be char");
            if (error_ != error_code::none) {
                return *this;
            }
            while (first != last) {
                assert (!stack_.empty ());
                auto & handler = stack_.top ();
                auto res = handler->consume (*this, just (*first));
                if (handler->is_done ()) {
                    if (error_ != error_code::none) {
                        break;
                    }
                    stack_.pop (); // release the topmost matcher object.
                }

                if (res.first != nullptr) {
                    if (stack_.size () > max_stack_depth_) {
                        // We've already hit the maximum allowed parse stack depth. Reject this new
                        // matcher.
                        assert (error_ == error_code::none);
                        error_ = error_code::nesting_too_deep;
                        break;
                    }

                    stack_.push (std::move (res.first));
                }
                // If we're matching this character, advance the column number and increment the
                // iterator.
                if (res.second) {
                    // Increment the column number if this is _not_ a UTF-8 continuation character.
                    if (utf::is_utf_char_start (*first)) {
                        this->advance_column ();
                    }
                    ++first;
                }
            }
            return *this;
        }

        // eof
        // ~~~
        template <typename Callbacks>
        auto parser<Callbacks>::eof () -> result_type {
            while (!stack_.empty () && !has_error ()) {
                auto & handler = stack_.top ();
                auto res = handler->consume (*this, nothing<char> ());
                assert (handler->is_done ());
                assert (res.second);
                stack_.pop (); // release the topmost matcher object.
            }
            return has_error () ? details::default_return<result_type>::get ()
                                : this->callbacks ().result ();
        }

    } // namespace json
} // namespace pstore

#endif // PSTORE_JSON_JSON_HPP
