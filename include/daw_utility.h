// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
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

#pragma once

#ifdef max
#undef max
#endif // max

#include <cassert>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include "daw_string_view.h"

namespace daw {
	namespace impl {
		template<typename ResultType, typename... ArgTypes>
		struct make_function_pointer_impl {
			using type = typename std::add_pointer<ResultType( ArgTypes... )>::type;
		};

		template<typename ResultType, typename ClassType, typename... ArgTypes>
		struct make_pointer_to_member_function_impl {
			using type = ResultType ( ClassType::* )( ArgTypes... );
		};

		template<typename ResultType, typename ClassType, typename... ArgTypes>
		struct make_pointer_to_volatile_member_function_impl {
			using type = ResultType ( ClassType::* )( ArgTypes... ) volatile;
		};

		template<typename ResultType, typename ClassType, typename... ArgTypes>
		struct make_pointer_to_const_member_function_impl {
			using type = ResultType ( ClassType::* )( ArgTypes... ) const;
		};

		template<typename ResultType, typename ClassType, typename... ArgTypes>
		struct make_pointer_to_const_volatile_member_function_impl {
			using type = ResultType ( ClassType::* )( ArgTypes... ) const volatile;
		};
	} // namespace impl

	template<typename ResultType, typename... ArgTypes>
	using function_pointer_t = typename impl::make_function_pointer_impl<ResultType, ArgTypes...>::type;

	template<typename ResultType, typename ClassType, typename... ArgTypes>
	using pointer_to_member_function_t =
	    typename impl::make_pointer_to_member_function_impl<ResultType, ClassType, ArgTypes...>::type;

	template<typename ResultType, typename ClassType, typename... ArgTypes>
	using pointer_to_volatile_member_function_t =
	    typename impl::make_pointer_to_volatile_member_function_impl<ResultType, ClassType, ArgTypes...>::type;

	template<typename ResultType, typename ClassType, typename... ArgTypes>
	using pointer_to_const_member_function_t =
	    typename impl::make_pointer_to_const_member_function_impl<ResultType, ClassType, ArgTypes...>::type;

	template<typename ResultType, typename ClassType, typename... ArgTypes>
	using pointer_to_const_volatile_member_function_t =
	    typename impl::make_pointer_to_const_volatile_member_function_impl<ResultType, ClassType, ArgTypes...>::type;

	namespace impl {
		template<typename T>
		class EqualToImpl final {
			T m_value;

		  public:
			EqualToImpl( T value ) : m_value( value ) {}
			EqualToImpl( ) = delete;
			~EqualToImpl( ) = default;
			EqualToImpl( EqualToImpl const & ) = default;
			EqualToImpl &operator=( EqualToImpl const & ) = default;
			EqualToImpl( EqualToImpl && ) = default;
			EqualToImpl &operator=( EqualToImpl && ) = default;

			bool operator( )( T const &value ) {
				return m_value == value;
			}
		}; // class EqualToImpl
	}      // namespace impl
	template<typename T>
	impl::EqualToImpl<T> equal_to( T value ) {
		return impl::EqualToImpl<T>( std::move( value ) );
	}

	template<typename T>
	class equal_to_last final {
		T *m_value;

	  public:
		equal_to_last( ) : m_value( nullptr ) {}
		~equal_to_last( ) = default;
		equal_to_last( equal_to_last const & ) = default;
		equal_to_last( equal_to_last && ) = default;
		equal_to_last &operator=( equal_to_last const & ) = default;
		equal_to_last &operator=( equal_to_last && ) = default;

		bool operator( )( T const &value ) {
			bool result = false;
			if( m_value ) {
				result = *m_value == value;
			}
			m_value = const_cast<T *>( &value );
			return result;
		}
	}; // class equal_to_last

	namespace impl {
		template<typename Function>
		class NotImpl final {
			Function m_function;

		  public:
			NotImpl( Function func ) : m_function( func ) {}
			~NotImpl( ) = default;
			NotImpl( NotImpl && ) = default;
			NotImpl &operator=( NotImpl const & ) = default;
			NotImpl &operator=( NotImpl && ) = default;

			template<typename... Args>
			bool operator( )( Args &&... args ) {
				return !m_function( std::forward<Args>( args )... );
			}
		}; // class NotImpl
	}      // namespace impl

	template<typename Function>
	impl::NotImpl<Function> Not( Function func ) {
		return impl::NotImpl<Function>( func );
	}

	// For generic types that are functors, delegate to its 'operator()'
	template<typename T>
	struct function_traits : public function_traits<decltype( &T::operator( ) )> {};

	// for pointers to member function(const version)
	template<typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType ( ClassType::* )( Args... ) const> {
		enum { arity = sizeof...( Args ) };
		using type = std::function<ReturnType( Args... )>;
		using result_type = ReturnType;
	};

	// for pointers to member function
	template<typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType ( ClassType::* )( Args... )> {
		enum { arity = sizeof...( Args ) };
		using type = std::function<ReturnType( Args... )>;
		using result_type = ReturnType;
	};

	// for function pointers
	template<typename ReturnType, typename... Args>
	struct function_traits<ReturnType ( * )( Args... )> {
		enum { arity = sizeof...( Args ) };
		using type = std::function<ReturnType( Args... )>;
		using result_type = ReturnType;
	};

	template<typename L>
	static typename function_traits<L>::type make_function( L l ) {
		return static_cast<typename function_traits<L>::type>( l );
	}

	// handles bind & multiple function call operator()'s
	template<typename ReturnType, typename... Args, class T>
	auto make_function( T &&t ) -> std::function<decltype( ReturnType( t( std::declval<Args>( )... ) ) )( Args... )> {
		return {std::forward<T>( t )};
	}

	// handles explicit overloads
	template<typename ReturnType, typename... Args>
	auto make_function( ReturnType ( *p )( Args... ) ) -> std::function<ReturnType( Args... )> {
		return {p};
	}

	// handles explicit overloads
	template<typename ReturnType, typename... Args, typename ClassType>
	auto make_function( ReturnType ( ClassType::*p )( Args... ) ) -> std::function<ReturnType( Args... )> {
		return {p};
	}

	template<typename T>
	T copy( T const &value ) {
		return value;
	}

	template<typename T>
	std::vector<T> copy_vector( std::vector<T> const &container, std::size_t num_items ) {
		assert( num_items <= container.size( ) );
		std::vector<T> result( num_items );
		auto first = std::begin( container );
		std::copy( first, first + static_cast<typename std::vector<T>::difference_type>( num_items ),
		           std::begin( result ) );
		return result;
	}

	template<typename T>
	void copy_vect_and_set( std::vector<T> &source, std::vector<T> &destination, std::size_t num_items,
	                        T const &replacement_value ) {
		using item_size_t = typename std::vector<T>::difference_type;
		assert( num_items < std::numeric_limits<item_size_t>::max( ) );
		auto first = std::begin( source );
		auto last = std::end( source );
		auto max_dist = std::distance( first, last );
		auto items = static_cast<item_size_t>( num_items );
		if( items < max_dist ) {
			last = first + items;
		}

		for( auto it = first; it != last; ++it ) {
			destination.push_back( *it );
			*it = replacement_value;
		}
	}

	template<class T, class U>
	T round_to_nearest( const T &value, const U &rnd_by ) {
		static_assert( std::is_arithmetic<T>::value, "First template parameter must be an arithmetic type" );
		static_assert( std::is_floating_point<U>::value, "Second template parameter must be a floating point type" );
		const auto rnd = std::round( static_cast<U>( value ) / rnd_by );
		const auto ret = rnd * rnd_by;
		return static_cast<T>( ret );
	}

	template<class T, class U>
	T floor_by( const T &value, const U &rnd_by ) {
		static_assert( std::is_arithmetic<T>::value, "First template parameter must be an arithmetic type" );
		static_assert( std::is_floating_point<U>::value, "Second template parameter must be a floating point type" );
		const auto rnd = std::floor( static_cast<U>( value ) / rnd_by );
		const auto ret = rnd * rnd_by;
		assert( ret <=
		        value ); // , __func__": Error, return value should always be less than or equal to value supplied" );
		return static_cast<T>( ret );
	}

	template<class T, class U>
	T ceil_by( const T &value, const U &rnd_by ) {
		static_assert( std::is_arithmetic<T>::value, "First template parameter must be an arithmetic type" );
		static_assert( std::is_floating_point<U>::value, "Second template parameter must be a floating point type" );
		const auto rnd = std::ceil( static_cast<U>( value ) / rnd_by );
		const auto ret = rnd * rnd_by;
		assert(
		    ret >=
		    value ); // , __func__": Error, return value should always be greater than or equal to value supplied" );
		return static_cast<T>( ret );
	}

	template<typename T>
	void copy_vect_and_set( std::shared_ptr<std::vector<T>> &source, std::shared_ptr<std::vector<T>> &destination,
	                        std::size_t num_items, T const &replacement_value ) {
		using item_size_t = typename std::vector<T>::difference_type;
		assert( num_items < std::numeric_limits<item_size_t>::max( ) );
		auto first = std::begin( *source );
		auto last = std::end( *source );
		auto max_dist = std::distance( first, last );
		auto items = static_cast<item_size_t>( num_items );
		if( items < max_dist ) {
			last = first + items;
		}

		for( auto it = first; it != last; ++it ) {
			destination->push_back( *it );
			*it = replacement_value;
		}
	}

	inline bool is_space( char chr ) {
		return 32 == chr;
	}

	template<typename Iterator, typename Pred>
	auto find_all_where( Iterator first, Iterator last, Pred predicate ) -> std::vector<Iterator> {
		std::vector<Iterator> results;
		for( auto it = first; it != last; ++it ) {
			if( predicate( *it ) ) {
				results.push_back( it );
			}
		}
		return results;
	}

	template<typename T, typename Pred>
	auto find_all_where( T const &values, Pred predicate ) -> std::vector<decltype( std::begin( values ) )> {
		return find_all_where( std::begin( values ), std::end( values ), predicate );
	}

	template<typename Iterator>
	Iterator advance( Iterator it, Iterator last, typename Iterator::difference_type how_far ) {
		auto result = it;
		while( result != last && std::distance( it, result ) < how_far ) {
			++it;
		}
		return it;
	}

	template<typename Iterator>
	Iterator find_buff( Iterator first, Iterator last, daw::string_view key ) {
		auto it = advance( first, last, static_cast<typename Iterator::difference_type>( key.size( ) ) );
		if( it == last ) {
			return last;
		}

		for( ; it != last; ++it, ++first ) {
			if( equal( first, it, key ) ) {
				return first;
			}
		}
		return last;
	}

	template<typename CharType>
	CharType AsciiUpper( CharType chr ) {
		return chr & ~static_cast<CharType>( 32 );
	}

	template<typename CharType>
	CharType AsciiLower( CharType chr ) {
		return chr | static_cast<CharType>( 32 );
	}

	template<typename CharType>
	std::basic_string<CharType> AsciiUpper( std::basic_string<CharType> str ) {
		for( auto &chr : str ) {
			chr = AsciiUpper( chr );
		}
		return str;
	}

	template<typename CharType>
	std::basic_string<CharType> AsciiLower( std::basic_string<CharType> str ) {
		for( auto &chr : str ) {
			chr = AsciiLower( chr );
		}
		return str;
	}

	template<typename Iterator>
	bool equal_nc( Iterator first, Iterator last, daw::string_view upper_value ) {
		if( static_cast<std::size_t>( std::distance( first, last ) ) != upper_value.size( ) ) {
			return false;
		}
		for( std::size_t off = 0; off < upper_value.size( ); ++off ) {
			auto const &left = upper_value[off];
			auto const &right = daw::AsciiUpper( *( first + static_cast<std::ptrdiff_t>( off ) ) );
			if( left != right ) {
				return false;
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	/// Summary:	Use this to move a value into a lambda capture by copy
	///				capture without incurring a copy
	template<typename T>
	class MoveCapture final {
		mutable T m_value;

	  public:
		MoveCapture( ) = delete;
		~MoveCapture( ) = default;
		MoveCapture( T &&val ) : m_value( std::move( val ) ) {}
		MoveCapture( MoveCapture && ) = default;
		MoveCapture &operator=( MoveCapture && ) = default;

		MoveCapture( MoveCapture const &other ) : m_value( std::move( other.m_value ) ) {}

		MoveCapture &operator=( MoveCapture const &rhs ) {
			if( this != &rhs ) {
				m_value = std::move( rhs.m_value );
			}
			return *this;
		}

		T &value( ) {
			return m_value;
		}

		T const &value( ) const {
			return m_value;
		}

		T &operator*( ) {
			return m_value.operator*( );
		}

		T const &operator*( ) const {
			return m_value.operator*( );
		}

		T const *operator->( ) const {
			return m_value.operator->( );
		}

		T move_out( ) {
			auto result = std::move( m_value );
			return result;
		}
	}; // class MoveCapture

	template<typename T>
	MoveCapture<T> as_move_capture( T &&val ) {
		return MoveCapture<T>( std::move( val ) );
	}

	namespace details {
		template<typename T>
		struct RunIfValid {
			::std::weak_ptr<T> m_link;
			RunIfValid( std::weak_ptr<T> w_ptr ) : m_link( w_ptr ) {}

			template<typename Function>
			bool operator( )( Function func ) {
				if( auto s_ptr = m_link.lock( ) ) {
					func( s_ptr );
					return true;
				} else {
					return false;
				}
			}
		};
	} // namespace details

	template<typename T>
	auto RunIfValid(::std::weak_ptr<T> w_ptr ) {
		return details::RunIfValid<T>( w_ptr );
	}

	template<typename T>
	auto copy_ptr_value( T *original ) {
		using result_t = std::decay_t<T>;
		return new result_t( *original );
	}

	// Acts like a reference, but has a strong no-null guarantee
	// Non-owning
	template<typename T>
	class not_null {
		T *m_ptr;

	  public:
		not_null( ) = delete;
		~not_null( ) = default;
		not_null( not_null const & ) noexcept = default;
		not_null( not_null && ) = default;
		not_null &operator=( not_null const & ) noexcept = default;
		not_null &operator=( not_null && ) noexcept = default;

		not_null( T *ptr ) : m_ptr( ptr ) {
			if( nullptr == ptr ) {
				throw std::invalid_argument( "ptr" );
			}
		}

		friend void swap( not_null &lhs, not_null &rhs ) noexcept {
			using std::swap;
			swap( lhs.m_ptr, rhs.m_ptr );
		}

		explicit operator bool( ) const noexcept {
			return true;
		}

		T *operator->( ) noexcept {
			return m_ptr;
		}

		T const *operator->( ) const noexcept {
			return m_ptr;
		}

		T *get( ) noexcept {
			return m_ptr;
		}

		T const *get( ) const noexcept {
			return m_ptr;
		}

		friend bool operator==( not_null const &lhs, not_null const &rhs ) noexcept {
			return std::equal_to<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

		friend bool operator!=( not_null const &lhs, not_null const &rhs ) noexcept {
			return !std::equal_to<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

		friend bool operator<( not_null const &lhs, not_null const &rhs ) noexcept {
			return std::less<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

		friend bool operator>( not_null const &lhs, not_null const &rhs ) noexcept {
			return std::greater<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

		friend bool operator<=( not_null const &lhs, not_null const &rhs ) noexcept {
			return std::less_equal<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

		friend bool operator>=( not_null const &lhs, not_null const &rhs ) noexcept {
			return std::greater_equal<void *>( )( lhs.m_ptr, rhs.m_ptr );
		}

	}; // not_null

	template<typename Arg, typename... Args>
	auto make_initializer_list( Arg &&arg, Args &&... args ) {
		return std::initializer_list<Arg>{std::forward<Arg>( arg ), std::forward<Args>( args )...};
	}

	template<typename Container, typename... Args>
	decltype( auto ) append( Container &container, Args &&... args ) {
		return container.insert( container.end( ), make_initializer_list( std::forward<Args>( args )... ) );
	}

	template<typename Container, typename Item>
	bool contains( Container const &container, Item const &item ) {
		using std::begin;
		using std::end;
		return std::find( begin( container ), end( container ), item ) == end( container );
	}

	template<typename Container, typename Item>
	decltype( auto ) index_of( Container const &container, Item const &item ) {
		using std::begin;
		using std::end;
		auto const pos = std::find( begin( container ), end( container ), item );
		return std::distance( begin( container ), pos );
	}

	constexpr auto or_all( ) noexcept {
		return 0;
	}

	template<typename Value>
	constexpr auto or_all( Value value ) noexcept {
		return value;
	}

	template<typename Value, typename... T>
	constexpr auto or_all( Value value, T... values ) noexcept {
		return value | or_all( values... );
	}

	template<typename Value>
	constexpr size_t bitcount( Value value ) noexcept {
		size_t result = 0;
		while( value ) {
			result += static_cast<size_t>( value & ~static_cast<Value>( 0b1 ) );
			value >>= 1;
		}
		return result;
	}

	template<typename Value, typename... T>
	constexpr size_t bitcount( Value value, T... values ) noexcept {
		return bitcount( value ) + bitcount( values... );
	};

	template<typename IntegerDest, typename IntegerSource>
	constexpr bool can_fit( IntegerSource const value ) noexcept {
		static_assert( std::numeric_limits<IntegerDest>::is_integer, "Must supply an integral type" );
		static_assert( std::numeric_limits<IntegerSource>::is_integer, "Must supply an integral type" );
		if( value >= 0 ) {
			return value <= std::numeric_limits<IntegerDest>::max( );
		} else if( std::numeric_limits<IntegerDest>::is_signed ) {
			return value >= std::numeric_limits<IntegerDest>::min( );
		} else {
			return false;
		}
	}
	constexpr void breakpoint( ) {
	;
	}

	template<typename T, typename std::enable_if_t<!std::is_floating_point<T>::value, long> = 0>
	constexpr bool nearly_equal( T const &a, T const &b ) noexcept {
		return a == b;
	}

	template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long> = 0>
	constexpr bool nearly_equal( T const &a, T const &b ) noexcept {
		// Code from http://floating-point-gui.de/errors/comparison/
		auto absA = std::abs( a );
		auto absB = std::abs( b );
		auto diff = std::abs( a - b );

		if( a == b ) { // shortcut, handles infinities
			return true;
		}
		if( a == 0 || b == 0 || diff < std::numeric_limits<T>::min_exponent ) {
			// a or b is zero or both are extremely close to it
			// 			// relative error is less meaningful here
			return diff < ( std::numeric_limits<T>::epsilon( ) * std::numeric_limits<T>::min_exponent );
		}
		// use relative error
		return diff / std::min( ( absA + absB ), std::numeric_limits<T>::max( ) ) <
		       std::numeric_limits<T>::epsilon( );
	}

	/*
	constexpr bool nearly_equal( double const &a, double const &b ) noexcept {
		using Float = double;
		// Code from http://floating-point-gui.de/errors/comparison/
		auto absA = std::abs( a );
		auto absB = std::abs( b );
		auto diff = std::abs( a - b );

		if( a == b ) { // shortcut, handles infinities
			return true;
		}
		if( a == 0 || b == 0 || diff < std::numeric_limits<Float>::min_exponent ) {
			// a or b is zero or both are extremely close to it
			// 			// relative error is less meaningful here
			return diff < ( std::numeric_limits<Float>::epsilon( ) * std::numeric_limits<Float>::min_exponent );
		}
		// use relative error
		return diff / std::min( ( absA + absB ), std::numeric_limits<Float>::max( ) ) <
		       std::numeric_limits<Float>::epsilon( );
	}

	constexpr bool nearly_equal( long double const &a, long double const &b ) noexcept {
		using Float = long double;
		// Code from http://floating-point-gui.de/errors/comparison/
		auto absA = std::abs( a );
		auto absB = std::abs( b );
		auto diff = std::abs( a - b );

		if( a == b ) { // shortcut, handles infinities
			return true;
		}
		if( a == 0 || b == 0 || diff < std::numeric_limits<Float>::min_exponent ) {
			// a or b is zero or both are extremely close to it
			// 			// relative error is less meaningful here
			return diff < ( std::numeric_limits<Float>::epsilon( ) * std::numeric_limits<Float>::min_exponent );
		}
		// use relative error
		return diff / std::min( ( absA + absB ), std::numeric_limits<Float>::max( ) ) <
		       std::numeric_limits<Float>::epsilon( );
	}
	*/
	template<typename Iterator>
	constexpr std::reverse_iterator<Iterator> make_reverse_iterator( Iterator i ) {
		return std::reverse_iterator<Iterator>( std::move( i ) );
	}

	template<typename T, typename Iterator>
	void fill_random( T min_value, T max_value, Iterator first, Iterator last ) {
		std::random_device rnd_device;
		// Specify the engine and distribution.
		std::mt19937 mersenne_engine{rnd_device( )};
		std::uniform_int_distribution<T> dist{min_value, max_value};

		std::generate( first, last, [&]( ) { return dist( mersenne_engine ); } );
	}

	template<typename T, typename RndType = T>
	std::vector<T> generate_random_data( size_t count, RndType min_value = std::numeric_limits<T>::min( ), RndType max_value = std::numeric_limits<T>::max( ) ) {
		std::vector<T> result;
		result.resize( count );
		fill_random( min_value, max_value, result.begin( ), result.end( ) );
		return result;
	}	
} // namespace daw

template<typename... Ts>
constexpr void Unused( Ts &&... ) {}

