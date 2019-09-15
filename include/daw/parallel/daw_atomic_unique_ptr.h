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

#pragma once

#include <atomic>

namespace daw {
	template<typename T>
	class atomic_unique_ptr {
		::std::atomic<T *> m_ptr = nullptr;

	public:
		atomic_unique_ptr( ) noexcept = default;

		atomic_unique_ptr( atomic_unique_ptr &&other ) noexcept
		  : m_ptr( other.m_ptr.exchange( nullptr, ::std::memory_order_acquire ) ) {}

		atomic_unique_ptr &operator=( atomic_unique_ptr &&rhs ) noexcept {
			swap( rhs );
			return *this;
		}

		~atomic_unique_ptr( ) noexcept {
			delete m_ptr.exchange( nullptr, ::std::memory_order_acquire );
		}

		atomic_unique_ptr( atomic_unique_ptr const & ) = delete;
		atomic_unique_ptr &operator=( atomic_unique_ptr const & ) = delete;

		template<typename U>
		atomic_unique_ptr( U *ptr ) noexcept
		  : m_ptr( ptr ) {}

		template<typename U>
		atomic_unique_ptr &operator=( U *ptr ) noexcept {
			delete m_ptr.exchange( nullptr, ::std::memory_order_acquire );
			m_ptr.store( ptr, ::std::memory_order_release );
			return *this;
		}

		template<typename U>
		atomic_unique_ptr( ::std::atomic<U *> ptr ) noexcept
		  : m_ptr( ptr ) {}

		template<typename U>
		atomic_unique_ptr &operator=( ::std::atomic<U *> ptr ) noexcept {
			delete m_ptr.exchange( nullptr, ::std::memory_order_acquire );
			m_ptr.store( ptr, ::std::memory_order_release );
			return *this;
		}

		atomic_unique_ptr( ::std::nullptr_t ) noexcept
		  : m_ptr( nullptr ) {}

		atomic_unique_ptr &operator=( ::std::nullptr_t ) noexcept {
			delete m_ptr.exchange( nullptr, ::std::memory_order_acquire );
			return *this;
		}

		T *get( ) const noexcept {
			return static_cast<T *>( m_ptr );
		}

		T *operator->( ) const noexcept {
			return get( );
		}

		decltype( auto ) operator*( ) const noexcept {
			return *get( );
		}

		explicit operator bool( ) const noexcept {
			return static_cast<bool>( get( ) );
		}

		T *release( ) noexcept {
			return m_ptr.exchange( nullptr, ::std::memory_order_acquire );
		}

		void reset( ) noexcept {
			delete m_ptr.exchange( nullptr, ::std::memory_order_acquire );
		}

		template<typename U>
		void swap( atomic_unique_ptr<U> & other ) noexcept {
			// TODO: verify this is correct
			auto tmp = other.m_ptr.load( ::std::memory_order_acquire );
			other.m_ptr.store( m_ptr.load( ::std::memory_order_acquire );
			m_ptr.store( tmp, ::std::memory_order_release );
		}
	};

	template<typename T, typename... Args>
	atomic_unique_ptr<T> make_atomic_unique_ptr( Args &&... args ) {
		if constexpr( ::std::is_aggregate_v<T> ) {
			return atomic_unique_ptr<T>( new T{::std::forward<Args>( args )...} );
		} else {
			return atomic_unique_ptr<T>( new T( ::std::forward<Args>( args )... ) );
		}
	}
} // namespace daw

