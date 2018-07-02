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

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "../daw_exception.h"
#include "../daw_value_ptr.h"

namespace daw {
	template<typename Mutex, typename ConditionVariable>
	class basic_condition_variable {
		// These are in value_ptr's so that the semaphore can be moved
		mutable value_ptr<Mutex> m_mutex;
		value_ptr<ConditionVariable> m_condition;

	public:
		basic_condition_variable( )
		  : m_mutex( )
		  , m_condition( ) {}

		template<typename Int>
		explicit basic_condition_variable( Int count )
		  : m_mutex( )
		  , m_condition( ) {}

		template<typename Int>
		basic_condition_variable( Int count, bool condition_variableed )
		  : m_mutex( )
		  , m_condition( ) {}

		void notify_all( ) {
			m_condition->notify_all( );
		}

		void notify_one( ) {
			m_condition->notify_one( );
		}

		template<typename Predicate>
		void wait( Predicate &&pred ) {
			auto lock = std::unique_lock<Mutex>( *m_mutex );
			m_condition->wait( lock, pred );
		}

		template<typename Rep, typename Period, typename Predicate>
		auto wait_for( std::chrono::duration<Rep, Period> const &rel_time,
		               Predicate &&pred ) {
			auto lock = std::unique_lock<Mutex>( *m_mutex );
			return m_condition->wait_for( lock, rel_time, pred );
		}

		template<typename Clock, typename Duration, typename Predicate>
		auto
		wait_until( std::chrono::time_point<Clock, Duration> const &timeout_time,
		            Predicate &&pred ) {
			auto lock = std::unique_lock<Mutex>( *m_mutex );
			return m_condition->wait_until( lock, timeout_time, pred );
		}
	}; // basic_condition_variable

	using condition_variable =
	  basic_condition_variable<std::mutex, std::condition_variable>;
} // namespace daw