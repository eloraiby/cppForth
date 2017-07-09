#ifndef INTRUSIVE_PTR__HPP
#define INTRUSIVE_PTR__HPP

#include "base.hpp"
namespace Forth {
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/object_ptr.html for documentation.
//
//
//  object_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T>
class IntrusivePtr
{
private:

	typedef IntrusivePtr this_type;

public:

	typedef T element_type;

	IntrusivePtr(): px( 0 )	{}

	IntrusivePtr( T * p, bool add_ref = true ): px( p ) {
		if( px != 0 && add_ref ) px->grab();
	}

	template<class U>
	IntrusivePtr( IntrusivePtr<U> const & rhs )
		: px( rhs.get() ) {
		if( px != 0 ) px->grab();
	}

	IntrusivePtr(IntrusivePtr const & rhs): px( rhs.px ) {
		if( px != 0 ) px->grab();
	}

	~IntrusivePtr() {
		if( px != 0 ) px->release();
	}

	template<class U> IntrusivePtr & operator=(IntrusivePtr<U> const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	// Move support

	//	object_ptr(object_ptr && rhs): px( rhs.px )
	//	{
	//		rhs.px = 0;
	//	}

	//	object_ptr & operator=(object_ptr && rhs)
	//	{
	//		this_type( static_cast< object_ptr && >( rhs ) ).swap(*this);
	//		return *this;
	//	}

	IntrusivePtr & operator=(IntrusivePtr const & rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr & operator=(T * rhs) {
		this_type(rhs).swap(*this);
		return *this;
	}

	void reset() {
		this_type().swap( *this );
	}

	void reset( T * rhs ) {
		this_type( rhs ).swap( *this );
	}

	T * get() const {
		return px;
	}

	T & operator*() const {
		//		if( px == nullptr )
		//			throw "pointer is null";
		return *px;
	}

	T * operator->() const {
		//		if( px == nullptr )
		//			throw "pointer is null";
		return px;
	}

	typedef T * this_type::*unspecified_bool_type;

	operator unspecified_bool_type() const { // never throws
		return px == 0? 0: &this_type::px;
	}

	// operator! is redundant, but some compilers need it
	bool operator! () const {// never throws
		return px == 0;
	}

	void swap(IntrusivePtr & rhs) {
		T * tmp = px;
		px = rhs.px;
		rhs.px = tmp;
	}

	// will create a pointer ambiguous
	//	operator T*()
	//	{
	//		return px;
	//	}

private:

	T*		px;
};

template<class T, class U>
inline bool operator==(IntrusivePtr<T> const & a, IntrusivePtr<U> const & b) {
	return a.get() == b.get();
}

template<class T, class U>
inline bool operator!=(IntrusivePtr<T> const & a, IntrusivePtr<U> const & b) {
	return a.get() != b.get();
}

template<class T, class U>
inline bool operator==(IntrusivePtr<T> const & a, U * b) {
	return a.get() == b;
}

template<class T, class U>
inline bool operator!=(IntrusivePtr<T> const & a, U * b) {
	return a.get() != b;
}

template<class T, class U>
inline bool operator==(T * a, IntrusivePtr<U> const & b) {
	return a == b.get();
}

template<class T, class U>
inline bool operator!=(T * a, IntrusivePtr<U> const & b) {
	return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T>
inline bool operator!=(IntrusivePtr<T> const & a, IntrusivePtr<T> const & b) {
	return a.get() != b.get();
}

#endif

template<class T>
inline bool operator<(IntrusivePtr<T> const & a, IntrusivePtr<T> const & b) {
	return (static_cast<size_t>(a.get()) < static_cast<size_t>(b.get()));
}

template<class T>
void swap(IntrusivePtr<T> & lhs, IntrusivePtr<T> & rhs) {
	lhs.swap(rhs);
}

// mem_fn support

template<class T>
T * get_pointer(IntrusivePtr<T> const & p) {
	return p.get();
}

template<class T, class U>
IntrusivePtr<T> static_pointer_cast(IntrusivePtr<U> const & p) {
	return static_cast<T *>(p.get());
}

template<class T, class U>
IntrusivePtr<T> const_pointer_cast(IntrusivePtr<U> const & p) {
	return const_cast<T *>(p.get());
}

template<class T, class U>
IntrusivePtr<T> dynamic_pointer_cast(IntrusivePtr<U> const & p) {
	return dynamic_cast<T *>(p.get());
}

} // namespace Forth
#endif  // INTRUSIVE_PTR__HPP