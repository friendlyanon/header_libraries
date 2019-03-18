// The MIT License (MIT)
//
// Copyright (c) 2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <algorithm>
#include <array>
#include <unordered_map>

#include "daw/daw_benchmark.h"
#include "daw/daw_traits.h"
#include "daw/iterator/daw_inserter.h"

static_assert( daw::traits::is_output_iterator_test<
               daw::remove_cvref_t<decltype(
                 daw::inserter( std::declval<std::vector<int> &>( ) ) )>,
               int>( ) );

void vector_test_001( ) {
	std::unordered_map<int, int> v{};
	std::array<int, 5> a = {1, 2, 3, 4, 5};
	std::transform(
	  begin( a ), end( a ), daw::inserter( v ),
	  []( auto const &val ) { return std::pair<int const, int>( val, val ); } );
	daw::expecting( a.size( ), v.size( ) );
}

int main( ) {
	vector_test_001( );
}
