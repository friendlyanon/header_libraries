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

#include <boost/type_traits.hpp>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cpp_17.h"

namespace daw {
	namespace traits {
		template<typename T>
		using root_type_t = std::decay_t<std::remove_reference_t<T>>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Return the largest sizeof in list of types.  Used to pad
		///				unions and small value optimization
		///
		template<typename... Args>
		struct max_sizeof;

		template<typename First>
		struct max_sizeof<First> {
			using type = First;
			static const size_t value = sizeof( type );
		};

		// the biggest of everything in Args and First
		template<typename First, typename... Args>
		struct max_sizeof<First, Args...> {
			using next = typename max_sizeof<Args...>::type;
			using type = typename std::conditional<sizeof( First ) >= sizeof( next ), First, next>::type;
			static const size_t value = sizeof( type );
		};

		template<typename... Types>
		using max_sizeof_t = typename max_sizeof<Types...>::type;

		template<typename... Types>
		constexpr auto const max_sizeof_v = max_sizeof<Types...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Returns true if all values passed are true
		///
		template<typename BoolType, typename std::enable_if<!is_integral_v<BoolType>, long>::type = 0>
		bool are_true( BoolType b1 ) {
			return b1 == true;
		}

		template<typename IntLike, typename std::enable_if<is_integral_v<IntLike>, long>::type = 0>
		bool are_true( IntLike b1 ) {
			return b1 != 0;
		}

		template<typename BoolType1, typename BoolType2>
		bool are_true( BoolType1 b1, BoolType2 b2 ) {
			return are_true( b1 ) && are_true( b2 );
		}

		template<typename BoolType1, typename BoolType2, typename... Booleans>
		bool are_true( BoolType1 b1, BoolType2 b2, Booleans... others ) {
			return are_true( b1, b2 ) && are_true( others... );
		}

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Are all template parameters the same type
		///
		namespace impl {
			template<typename T, typename... Rest>
			struct are_same_types : std::false_type {};

			template<typename T, typename First>
			struct are_same_types<T, First> : std::is_same<T, First> {};

			template<typename T, typename First, typename... Rest>
			struct are_same_types<T, First, Rest...>
			  : std::integral_constant<bool, is_same_v<T, First> || are_same_types<T, Rest...>::value> {};
		} // namespace impl

		template<typename T, typename... Rest>
		constexpr bool are_same_types_v = impl::are_same_types<T, Rest...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	A sequence of bool values
		///
		template<bool...>
		struct bool_sequence {};

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Integral constant with result of and'ing all bool's supplied
		///
		template<bool... Bools>
		using bool_and = std::is_same<bool_sequence<Bools...>, bool_sequence<( Bools || true )...>>;

		template<bool... Bools>
		constexpr bool bool_and_v = bool_and<Bools...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Integral constant with result of or'ing all bool's supplied
		///
		template<bool... Bools>
		using bool_or = std::integral_constant<bool, ( !bool_and<!Bools...>::value )>;

		template<bool... Bools>
		constexpr bool bool_or_v = bool_or<Bools...>::value;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Similar to enable_if but enabled when any of the
		///				parameters is true
		///
		template<typename R, bool... Bs>
		using enable_if_any = std::enable_if<bool_or_v<Bs...>, R>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Similar to enable_if but enabled when all of the
		///				parameters is true
		///
		template<typename R, bool... Bs>
		using enable_if_all = std::enable_if<bool_and_v<Bs...>, R>;

		//////////////////////////////////////////////////////////////////////////
		/// Summary:	Is type T on of the other types
		///
		namespace impl {
			template<typename T, typename... Types>
			struct is_one_of : public std::false_type {};

			template<typename T, typename Type>
			struct is_one_of<T, Type> : public std::is_same<T, Type> {};

			template<typename T, typename Type, typename... Types>
			struct is_one_of<T, Type, Types...>
			  : public std::integral_constant<bool, is_same_v<T, Type> || is_one_of<T, Types...>::value> {};

		} // namespace impl

		template<typename T, typename... Types>
		constexpr bool is_one_of_v = impl::is_one_of<T, Types...>::value;

		namespace details {
			template<typename>
			struct type_sink {
				using type = void;
			}; // consumes a type, and makes it `void`
			template<typename T>
			using type_sink_t = typename type_sink<T>::type;
		} // namespace details

		/*
		#define MEMBER_FUNC_CHECKER( name, fn, ret, args ) \
		  template<typename C, typename = void> \
		  struct name : std::false_type {}; \
		  template<typename C> \
		  struct name<C, typename std::enable_if_t<is_convertible_v<decltype( std::declval<C>( ).fn args ), ret>>> \ :
		std::true_type {};

		#define MTYPE_CHECKER_ANY( checker, name ) \
		  template<class C, typename = void> \
		  struct checker : std::false_type {}; \
		  template<class C> \
		  struct checker<C, typename std::enable_if<!std::is_same<decltype( C::name ) *, void>::value>::type> \ :
		std::true_type {}
		*/

#define METHOD_CHECKER_ANY( name, fn, args )                                                                           \
	namespace impl {                                                                                                     \
		template<typename T, typename = void>                                                                              \
		struct name : std::false_type {};                                                                                  \
		template<typename T>                                                                                               \
		struct name<T, typename std::enable_if_t<!is_same_v<decltype( std::declval<T>( ).fn args ) *, void>>>              \
		  : std::true_type {};                                                                                             \
	}                                                                                                                    \
	template<typename T>                                                                                                 \
	constexpr bool name##_v = impl::name<T>::value;

#define GENERATE_HAS_MEMBER_FUNCTION_TRAIT( MemberName )                                                               \
	namespace impl {                                                                                                     \
		template<typename T, typename = void>                                                                              \
		class has_##MemberName##_member_impl : public std::false_type {};                                                  \
		template<typename T>                                                                                               \
		class has_##MemberName##_member_impl<T, typename std::enable_if<std::is_class<T>::value>::type> {                  \
			struct Fallback {                                                                                                \
				int MemberName;                                                                                                \
			};                                                                                                               \
			struct Derived : T, Fallback {};                                                                                 \
                                                                                                                       \
			template<typename U, U>                                                                                          \
			struct Check;                                                                                                    \
                                                                                                                       \
			using ArrayOfOne = char[1];                                                                                      \
			using ArrayOfTwo = char[2];                                                                                      \
                                                                                                                       \
			template<typename U>                                                                                             \
			static ArrayOfOne &func( Check<int Fallback::*, &U::MemberName> * );                                             \
			template<typename U>                                                                                             \
			static ArrayOfTwo &func( ... );                                                                                  \
                                                                                                                       \
		public:                                                                                                            \
			using type = has_##MemberName##_member_impl;                                                                     \
			enum { value = sizeof( func<Derived>( 0 ) ) == 2 };                                                              \
		}; /*struct has_##MemberName##_member_impl*/                                                                       \
	}    /* namespace impl */                                                                                            \
	template<typename T>                                                                                                 \
	constexpr bool has_##MemberName##_member_v = impl::has_##MemberName##_member_impl<T>::value;

#define GENERATE_HAS_MEMBER_TYPE_TRAIT( TypeName )                                                                     \
	namespace impl {                                                                                                     \
		template<typename T, typename = void>                                                                              \
		struct has_##TypeName##_member : std::false_type {};                                                               \
		template<typename T>                                                                                               \
		struct has_##TypeName##_member<T, details::type_sink_t<typename T::TypeName>> : std::true_type {};                 \
	}                                                                                                                    \
	template<typename T>                                                                                                 \
	constexpr bool has_##TypeName##_member_v = impl::has_##TypeName##_member<T>::value;

		//		GENERATE_HAS_MEMBER_FUNCTION_TRAIT( begin );
		//
		METHOD_CHECKER_ANY( has_begin_member, begin, ( ) );
		METHOD_CHECKER_ANY( has_end_member, end, ( ) );
		METHOD_CHECKER_ANY( has_substr_member, substr, ( 0, 1 ) );

		// GENERATE_HAS_MEMBER_FUNCTION_TRAIT( end );
		// GENERATE_HAS_MEMBER_FUNCTION_TRAIT( substr );
		GENERATE_HAS_MEMBER_FUNCTION_TRAIT( push_back );

		GENERATE_HAS_MEMBER_TYPE_TRAIT( type );
		GENERATE_HAS_MEMBER_TYPE_TRAIT( value_type );
		GENERATE_HAS_MEMBER_TYPE_TRAIT( mapped_type );
		GENERATE_HAS_MEMBER_TYPE_TRAIT( iterator );

		template<typename T>
		constexpr bool is_string_v = all_true_v<is_convertible_v<T, std::string> || is_convertible_v<T, std::wstring>>;

		template<typename T>
		constexpr bool isnt_string_v = !is_string_v<T>;

		template<typename T>
		constexpr bool is_container_like_v = all_true_v<has_begin_member_v<T>, has_end_member_v<T>>;

		template<typename T>
		constexpr bool is_container_not_string_v = all_true_v<isnt_string_v<T>, is_container_like_v<T>>;

		template<typename T>
		constexpr bool is_map_like_v = all_true_v<is_container_like_v<T>, has_mapped_type_member_v<T>>;

		template<typename T>
		constexpr bool isnt_map_like_v = !is_map_like_v<T>;

		template<typename T>
		constexpr bool is_vector_like_not_string_v = all_true_v<is_container_not_string_v<T>, isnt_map_like_v<T>>;

		template<typename T>
		using static_not = std::conditional<T::value, std::false_type, std::true_type>;

		template<typename T>
		using static_not_t = typename static_not<T>::type;

		template<typename T>
		constexpr bool static_not_v = static_not<T>::value;

#define GENERATE_IS_STD_CONTAINER1( ContainerName )                                                                    \
	template<typename T>                                                                                                 \
	constexpr bool is_##ContainerName##_v = is_same_v<T, std::ContainerName<typename T::value_type>>;

		GENERATE_IS_STD_CONTAINER1( vector );
		GENERATE_IS_STD_CONTAINER1( list );
		GENERATE_IS_STD_CONTAINER1( set );
		GENERATE_IS_STD_CONTAINER1( unordered_set );
		GENERATE_IS_STD_CONTAINER1( deque );

#undef GENERATE_IS_STD_CONTAINER1

#define GENERATE_IS_STD_CONTAINER2( ContainerName )                                                                    \
	template<typename T>                                                                                                 \
	constexpr bool is_##ContainerName##_v =                                                                              \
	  is_same_v<T, std::ContainerName<typename T::key_type, typename T::mapped_type>>;

		GENERATE_IS_STD_CONTAINER2( map );
		GENERATE_IS_STD_CONTAINER2( unordered_map );

#undef GENERATE_IS_STD_CONTAINER2

		template<typename T>
		constexpr bool is_single_item_container_v =
		  any_true_v<is_vector_v<T>, is_list_v<T>, is_set_v<T>, is_deque_v<T>, is_unordered_set_v<T>>;

		template<typename T>
		constexpr bool is_container_v = any_true_v<is_vector_v<T>, is_list_v<T>, is_set_v<T>, is_deque_v<T>,
		                                           is_unordered_set_v<T>, is_map_v<T>, is_unordered_map_v<T>>;

		template<typename T>
		constexpr bool is_map_type_v = any_true_v<is_map_v<T>, is_unordered_map_v<T>>;

		template<typename T>
		constexpr bool is_numberic_v = any_true_v<is_floating_point_v<T>, is_integral_v<T>>;

		template<typename T>
		constexpr bool is_container_or_array_v = any_true_v<is_container_v<T>, is_array_v<T>>;

		template<typename T>
		constexpr bool is_streamable_v = boost::has_left_shift<std::ostream &, T const &, std::ostream &>::value;

		template<template<class> class Base, typename Derived>
		constexpr bool is_mixed_from_v = is_base_of_v<Base<Derived>, Derived>;

		template<std::size_t I, typename T, typename... Ts>
		struct nth_element_impl {
			using type = typename nth_element_impl<I - 1, Ts...>::type;
		};

		template<typename T, typename... Ts>
		struct nth_element_impl<0, T, Ts...> {
			using type = T;
		};

		template<std::size_t I, typename... Ts>
		using nth_element = typename nth_element_impl<I, Ts...>::type;

		namespace impl {
			template<typename T>
			auto has_to_string( T const &, long ) -> std::false_type;

			template<typename T>
			auto has_to_string( T const &lhs, int ) -> std::is_convertible<decltype( lhs.to_string( ) ), std::string>;
		} // namespace impl

		template<typename T>
		constexpr bool has_to_string_v = decltype( impl::has_to_string( std::declval<T>( ), 1 ) )::value;

		namespace operators {
			namespace impl {
				template<typename L, typename R>
				auto has_op_eq_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_eq_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs == rhs ), bool>;

				template<typename L, typename R>
				auto has_op_ne_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_ne_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs != rhs ), bool>;

				template<typename L, typename R>
				auto has_op_lt_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_lt_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs < rhs ), bool>;

				template<typename L, typename R>
				auto has_op_le_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_le_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs <= rhs ), bool>;

				template<typename L, typename R>
				auto has_op_gt_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_gt_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs > rhs ), bool>;

				template<typename L, typename R>
				auto has_op_ge_impl( L const &, R const &, long ) -> std::false_type;

				template<typename L, typename R>
				auto has_op_ge_impl( L const &lhs, R const &rhs, int ) -> std::is_convertible<decltype( lhs >= rhs ), bool>;
			} // namespace impl

			template<typename L, typename R = L>
			constexpr bool has_op_eq_v = decltype( impl::has_op_eq_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			constexpr bool has_op_ne_v = decltype( impl::has_op_ne_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			constexpr bool has_op_lt_v = decltype( impl::has_op_lt_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			constexpr bool has_op_le_v = decltype( impl::has_op_le_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			constexpr bool has_op_gt_v = decltype( impl::has_op_gt_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

			template<typename L, typename R = L>
			constexpr bool has_op_ge_v = decltype( impl::has_op_ge_impl( std::declval<L>( ), std::declval<R>( ), 1 ) )::value;

		} // namespace operators

		template<typename T, typename U>
		constexpr inline bool not_self( ) {
			using decayed_t = typename std::decay_t<T>;
			return !is_same_v<decayed_t, U> && !is_base_of_v<U, decayed_t>;
		}
	} // namespace traits

	template<typename T>
	using make_fp = std::add_pointer_t<T>;

	namespace impl {
		template<typename Function>
		struct void_function {
			Function function;
			void_function( ) = default;
			void_function( Function &&func ) : function{std::forward<Function>( func )} {}
			~void_function( ) = default;
			void_function( void_function const & ) = default;
			void_function( void_function && ) = default;
			void_function &operator=( void_function const & ) = default;
			void_function &operator=( void_function && ) = default;

			explicit constexpr operator bool( ) noexcept {
				return static_cast<bool>( function );
			}

			template<typename... Args>
			void operator( )( Args &&... args ) {
				function( std::forward<Args>( args )... );
			}
		};
	} // namespace impl
	template<typename Function>
	auto make_void_function( Function func ) noexcept {
		return impl::void_function<Function>{std::move( func )};
	}
	namespace detectors {
		template<typename Function, typename... Args>
		using callable_with = decltype( std::declval<Function &>( )( std::declval<Args &>( )... ) );

		template<typename BinaryPredicate, typename T, typename U = T>
		using binary_predicate = callable_with<BinaryPredicate, T, U>;

		template<typename UnaryPredicate, typename T>
		using unary_predicate = callable_with<UnaryPredicate, T>;

		// Verifies that a == b is valid along with b == a and that the result is the same.
		// TODO: add referce check is U == V and U < V valid
		template<typename T, typename U>
		using equality_comparable = decltype( std::declval<T>( ) == std::declval<U>( ) );

		template<typename T, typename U>
		using less_than_comparable = decltype( std::declval<T>( ) < std::declval<U>( ) );

		namespace details {
			template<typename T, typename U>
			void swap( T &lhs, U &rhs ) {
				using std::swap;
				swap( lhs, rhs );
			}
		} // namespace details

		template<typename T>
		using swappable = decltype( details::swap( std::declval<T>( ), std::declval<T>( ) ) );

		template<typename Iterator, typename T>
		using assignable = decltype( *std::declval<Iterator>( ) = std::declval<T>( ) );

	} // namespace detectors

	template<typename Function, typename... Args>
	using is_callable = is_detected<detectors::callable_with, Function, Args...>;

	template<typename Function, typename... Args>
	using is_callable_t = detected_t<detectors::callable_with, Function, Args...>;

	template<typename Function, typename... Args>
	constexpr bool is_callable_v = is_detected_v<detectors::callable_with, Function, Args...>;

	template<typename Predicate, typename... Args>
	constexpr bool is_predicate_v = is_detected_convertible_v<bool, detectors::callable_with, Predicate, Args...>;

	template<typename BinaryPredicate, typename T, typename U = T>
	constexpr bool is_binary_predicate_v = is_predicate_v<BinaryPredicate, T, U>;

	template<typename UnaryPredicate, typename T>
	constexpr bool is_unary_predicate_v = is_predicate_v<UnaryPredicate, T>;

	template<typename T, typename U = T>
	constexpr bool is_equality_comparable_v = is_detected_convertible_v<bool, detectors::equality_comparable, T, U>;

	template<typename T, typename U = T>
	constexpr bool is_less_than_comparable_v = is_detected_convertible_v<bool, detectors::less_than_comparable, T, U>;

	template<typename T>
	constexpr bool is_swappable_v = is_detected_v<detectors::swappable, T>;

	template<typename Iterator, typename T = typename std::iterator_traits<Iterator>::value_type>
	constexpr bool is_assignable_iterator_v = is_detected_v<detectors::assignable, Iterator, T>;

	template<typename L, typename R>
	constexpr bool is_comparable_v = is_equality_comparable_v<L, R> &&is_equality_comparable_v<R, L>;
	//:
	//////////////////////////////////////////////////////////////////////////
	/// Summary: is like a regular type see http://www.stepanovpapers.com/DeSt98.pdf
	template<typename T>
	constexpr bool is_regular_v =
	  all_true_v<is_default_constructible_v<T> && is_copy_constructible_v<T> && is_move_constructible_v<T> &&
	             is_copy_assignable_v<T> && is_move_assignable_v<T> && is_equality_comparable_v<T>>;

} // namespace daw
