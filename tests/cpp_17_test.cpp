// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "boost_test.h"
#include <iostream>

#include "cpp_17.h"

void test( ) {}

BOOST_AUTO_TEST_CASE( cpp_17_test_01 ) {
	constexpr auto const is_func = daw::is_function_v<decltype( test )>;
	constexpr auto const isnt_func = daw::is_function_v<decltype( is_func )>;

	BOOST_REQUIRE( is_func );
	BOOST_REQUIRE( !isnt_func );
}

struct test_binary_pred_t {
	constexpr bool operator( )( int a, int b ) const noexcept {
		return a == b;
	}
};

BOOST_AUTO_TEST_CASE( binary_predicate_002 ) {
	using a_t = daw::is_detected<daw::detectors::callable_with, test_binary_pred_t, int, int>;
	using a_conv_t = std::is_convertible<a_t, bool>;
	constexpr auto const a = a_conv_t::value;
	static_assert( a, "a: Not callable with (int, int )" );

	constexpr auto const b = daw::is_callable_v<test_binary_pred_t, int, int>;
	static_assert( b, "b: Not callable with (int, int )" );

	constexpr auto const c = daw::is_detected_convertible_v<bool, daw::detectors::callable_with, test_binary_pred_t, int, int>;
	static_assert( c, "c: Not callable with (int, int )" );

	static_assert( daw::is_predicate_v<test_binary_pred_t, int, int>, "Not a binary predicate" );
	BOOST_REQUIRE( a );
}

