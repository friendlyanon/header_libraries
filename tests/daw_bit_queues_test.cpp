// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Darrell Wright
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

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "daw_bit_queues.h"

BOOST_AUTO_TEST_CASE( daw_bit_queues_testing ) {
	using value_type = uint16_t;
	daw::bit_queue_gen<value_type, value_type> test1;
	test1.push_back( 1, 1 );
	BOOST_REQUIRE( test1.value( ) == 1 );
	BOOST_REQUIRE( test1.size( ) == 1 );
	BOOST_REQUIRE( test1.pop_back( 1 ) == 1 );
	test1.clear( );
	BOOST_REQUIRE( test1.value( ) == 0 );
	BOOST_REQUIRE( test1.size( ) == 0 );

	{
		daw::bit_queue_gen<value_type, value_type> test2;
		test2.push_back( 2 );
		BOOST_REQUIRE( test2.value( ) == 2 );
		BOOST_REQUIRE( test2.size( ) == sizeof(value_type)*8 );
		BOOST_REQUIRE( test2.pop_back( 1 ) == 0 );
	}

	uint16_t v = 1;
	test1.push_back( 1, 1 );

	for( size_t n=1; n<sizeof(value_type)*8; ++n ) {
		test1.push_back( 0, 1 );
		v *= 2;
		BOOST_REQUIRE( test1.value( ) == v );
	}

	for( size_t n=1; n<sizeof(value_type)*8; ++n ) {
		int const result = test1.pop_back( 1 );
		BOOST_REQUIRE( result == 0 );
	}
	BOOST_REQUIRE( test1.value( ) == 1 );
	BOOST_REQUIRE( test1.size( ) == 1 );
	BOOST_REQUIRE( test1.pop_back( 1 ) == 1 );
	BOOST_REQUIRE( test1.empty( ) );

	v = 1;
	test1.push_back( 1, 1 );
	for( size_t n=1; n<sizeof(value_type)*8; ++n ) {
		test1.push_back( 0, 1 );
		v *= 2;
		BOOST_REQUIRE( test1.value( ) == v );
	}
	BOOST_REQUIRE( test1.pop_front( 1 ) == 1 );
	for( size_t n=1; n<sizeof(value_type)*8; ++n ) {
		int const result = test1.pop_front( 1 );
		BOOST_REQUIRE( result == 0 );
	}
	BOOST_REQUIRE( test1.size( ) == 0 );
	BOOST_REQUIRE( test1.empty( ) );
}
