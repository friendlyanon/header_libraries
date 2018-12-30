// The MIT License (MIT)
//
// Copyright (c) 2018 Darrell Wright
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

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "daw_traits.h"

namespace daw {
	template<typename T>
	struct value_ptr : enable_default_constructor<T>,
	                   enable_copy_constructor<T>,
	                   enable_copy_assignment<T> {
		using value_type = T;
		using reference = value_type &;
		using const_reference = value_type const &;
		using pointer = value_type *;
		using const_pointer = value_type const *;

	private:
		pointer m_value = nullptr;

		template<typename U = value_type, typename... Args>
		static pointer make_ptr( Args &&... args ) noexcept(
		  is_nothrow_constructible_v<value_type, Args...> ) {

			return new U( std::forward<Args>( args )... );
		}

	public:
		constexpr explicit value_ptr( std::nullopt_t ) noexcept {}

		template<
		  typename... Args,
		  std::enable_if_t<
		    all_true_v<is_constructible_v<value_type, Args...>,
		               !traits::is_first_type_v<value_ptr, std::decay_t<Args>...>>,
		    std::nullptr_t> = nullptr>
		explicit value_ptr( Args &&... args ) noexcept(
		  is_nothrow_constructible_v<value_type, Args...> )
		  : enable_default_constructor<T>( impl::non_constructor{} )
		  , enable_copy_constructor<T>( impl::non_constructor{} )
		  , enable_copy_assignment<T>( impl::non_constructor{} )
		  , m_value( make_ptr( std::forward<Args>( args )... ) ) {}

		template<
		  typename U, typename... Args,
		  std::enable_if_t<
		    all_true_v<is_constructible_v<value_type, Args...>,
		               !traits::is_first_type_v<value_ptr, std::decay_t<Args>...>>,
		    std::nullptr_t> = nullptr>
		static value_ptr emplace( Args &&... args ) noexcept(
		  is_nothrow_constructible_v<value_type, Args...> ) {

			auto result = value_ptr( std::nullopt );
			result.m_value = make_ptr<U>( std::forward<Args>( args )... );
			return result;
		}

		value_ptr( value_ptr const &other ) noexcept(
		  noexcept( make_ptr( *other.m_value ) ) )
		  : enable_default_constructor<T>( other )
		  , enable_copy_constructor<T>( other )
		  , enable_copy_assignment<T>( other )
		  , m_value( make_ptr( *other.m_value ) ) {}

		value_ptr( value_ptr &&other ) noexcept
		  : enable_default_constructor<T>( std::move( other ) )
		  , enable_copy_constructor<T>( std::move( other ) )
		  , enable_copy_assignment<T>( std::move( other ) )
		  , m_value( std::exchange( other.m_value, nullptr ) ) {}

		value_ptr &operator=( value_ptr const &rhs ) noexcept(
		  is_nothrow_copy_constructible_v<value_type> ) {
			*m_value = *( rhs.m_value );
			return *this;
		}

		value_ptr &operator=( value_ptr &&rhs ) noexcept {
			m_value = std::exchange( rhs.m_value, nullptr );
			return *this;
		}

		value_ptr &operator=( value_type const &rhs ) noexcept(
		  is_nothrow_copy_assignable_v<value_type> ) {

			*m_value = rhs;
			return *this;
		}

		value_ptr &operator=( value_type &&rhs ) noexcept(
		  is_nothrow_move_assignable_v<value_type> ) {

			*m_value = std::move( rhs );
			return *this;
		}

		void reset( ) noexcept( is_nothrow_destructible_v<value_type> ) {
			delete std::exchange( m_value, nullptr );
		}

		template<typename... Args,
		         std::enable_if_t<traits::is_callable_v<value_type, Args...>,
		                          std::nullptr_t> = nullptr>
		decltype( auto ) operator( )( Args &&... args ) noexcept(
		  noexcept( m_value->operator( )( std::forward<Args>( args )... ) ) ) {

			return m_value->operator( )( std::forward<Args>( args )... );
		}

		~value_ptr( ) noexcept( is_nothrow_destructible_v<value_type> ) {
			reset( );
		}

		pointer release( ) noexcept {
			return std::exchange( m_value, nullptr );
		}

		void swap( value_ptr &rhs ) noexcept {
			auto tmp = m_value;
			m_value = std::exchange( rhs.m_value, tmp );
		}

		pointer get( ) noexcept {
			return m_value;
		}

		const_pointer get( ) const noexcept {
			return m_value;
		}

		reference operator*( ) noexcept {
			return *m_value;
		}

		const_reference operator*( ) const noexcept {
			return *m_value;
		}

		pointer operator->( ) noexcept {
			return m_value;
		}

		const_pointer operator->( ) const noexcept {
			return m_value;
		}

		explicit operator reference( ) noexcept {
			return *m_value;
		}

		explicit operator const_reference( ) const noexcept {
			return *m_value;
		}
	};

	template<typename T>
	void swap( value_ptr<T> & lhs, value_ptr<T> & rhs ) noexcept {
		lhs.swap( rhs );
	}

	namespace value_ptr_impl {
		template<typename T, typename U>
		using has_compare_equality_test =
		  decltype( std::declval<T>( ) == std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_equality =
		  std::enable_if_t<is_detected_v<has_compare_equality_test, T, U>, V>;

		template<typename T, typename U>
		using has_compare_inequality_test =
		  decltype( std::declval<T>( ) != std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_inequality =
		  std::enable_if_t<is_detected_v<has_compare_inequality_test, T, U>, V>;

		template<typename T, typename U>
		using has_compare_greater_test =
		  decltype( std::declval<T>( ) > std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_greater =
		  std::enable_if_t<is_detected_v<has_compare_greater_test, T, U>, V>;

		template<typename T, typename U>
		using has_compare_greater_equal_test =
		  decltype( std::declval<T>( ) >= std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_greater_equal =
		  std::enable_if_t<is_detected_v<has_compare_greater_equal_test, T, U>, V>;

		template<typename T, typename U>
		using has_compare_less_test =
		  decltype( std::declval<T>( ) < std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_less =
		  std::enable_if_t<is_detected_v<has_compare_less_test, T, U>, V>;

		template<typename T, typename U>
		using has_compare_less_equal_test =
		  decltype( std::declval<T>( ) <= std::declval<U>( ) );

		template<typename T, typename U, typename V = void>
		using has_compare_less_equal =
		  std::enable_if_t<is_detected_v<has_compare_less_equal_test, T, U>, V>;
	} // namespace value_ptr_impl

	template<typename T, typename U,
	         value_ptr_impl::has_compare_equality<T, U, std::nullptr_t> = nullptr>
	bool
	operator==( value_ptr<T> const &lhs,
	            value_ptr<U> const &rhs ) noexcept( noexcept( *lhs == *rhs ) ) {
		return *lhs == *rhs;
	}

	template<
	  typename T, typename U,
	  value_ptr_impl::has_compare_inequality<T, U, std::nullptr_t> = nullptr>
	bool
	operator!=( value_ptr<T> const &lhs,
	            value_ptr<U> const &rhs ) noexcept( noexcept( *lhs != *rhs ) ) {
		return *lhs != *rhs;
	}

	template<typename T, typename U,
	         value_ptr_impl::has_compare_greater<T, U, std::nullptr_t> = nullptr>
	bool
	operator>( value_ptr<T> const &lhs,
	           value_ptr<U> const &rhs ) noexcept( noexcept( *lhs > *rhs ) ) {
		return *lhs > *rhs;
	}

	template<
	  typename T, typename U,
	  value_ptr_impl::has_compare_greater_equal<T, U, std::nullptr_t> = nullptr>
	bool
	operator>=( value_ptr<T> const &lhs,
	            value_ptr<U> const &rhs ) noexcept( noexcept( *lhs >= *rhs ) ) {
		return *lhs >= *rhs;
	}

	template<typename T, typename U,
	         value_ptr_impl::has_compare_less<T, U, std::nullptr_t> = nullptr>
	bool
	operator<( value_ptr<T> const &lhs,
	           value_ptr<U> const &rhs ) noexcept( noexcept( *lhs < *rhs ) ) {
		return *lhs < *rhs;
	}

	template<
	  typename T, typename U,
	  value_ptr_impl::has_compare_less_equal<T, U, std::nullptr_t> = nullptr>
	bool
	operator<=( value_ptr<T> const &lhs,
	            value_ptr<U> const &rhs ) noexcept( noexcept( *lhs <= *rhs ) ) {
		return *lhs <= *rhs;
	}
	namespace impl {
		struct empty_t {};
	} // namespace impl
} // namespace daw

namespace std {
	template<typename T>
	struct hash<daw::value_ptr<T>>
	  : std::enable_if_t<daw::traits::is_callable_v<std::hash<T>, T>,
	                     daw::impl::empty_t> {

		template<typename Arg>
		size_t operator( )( Arg &&arg ) const {
			return std::hash<T>{}( *std::forward<Arg>( arg ) );
		}
	};
} // namespace std
