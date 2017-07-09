#ifndef VECTOR_HPP
#define VECTOR_HPP
///
/// RTK vector implementation
///

namespace raja
{
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <malloc.h>

struct VectorBase {
    size_t			count_;
    size_t			reserved_;
    void*           data_;

    VectorBase(size_t count, size_t reserved, void* data) : count_(count), reserved_(reserved), data_(data) {}
};

template<typename T>
struct rvector : protected VectorBase
{
private:
	enum
	{
        MIN_VEC_RES_SIZE_	= 4
	};

public:
    rvector() : VectorBase(0, MIN_VEC_RES_SIZE_, malloc(sizeof(T) * MIN_VEC_RES_SIZE_)) {
	}

    rvector(size_t n, const T* elems) : VectorBase(n, n ? n : MIN_VEC_RES_SIZE_, malloc(sizeof(T) * (n ? n : MIN_VEC_RES_SIZE_))) {
		if( n ) {
            T*  data    = static_cast<T*>(data_);
            for( size_t i = 0; i < count_; ++i ) {
                new(&(data[i])) T(elems[i]);
            }
        }
	}

	~rvector() {
        T*  data    = static_cast<T*>(data_);
        for( size_t i = 0; i < count_; ++i ) {
            (data[i]).~T();
		}
        free(data_);
        count_		= 0;
        reserved_	= 0;
	}

	const T*
    get() const	{	return data_; }

	T*
    get()		{ return data_; }

    const T&
    back() const { T*  data    = static_cast<T*>(data_); return data[count_ - 1]; }

	void
	push_back(const T& t)	{
        T*  data    = static_cast<T*>(data_);

        if( count_ == reserved_ ) {	// we have reached the limit
            reserved_	<<= 1;

            T*	new_data	= static_cast<T*>(malloc(sizeof(T) * reserved_));
            for( size_t i = 0; i < count_; ++i )
                new(&(new_data[i])) T(data[i]);

			// remove old data
            for( size_t i = 0; i < count_; ++i )
                (data[i]).~T();

            free(data_);

            data_	= new_data;
            data    = new_data;
		}

        new(&(data[count_])) T(t);
        ++count_;
	}

	void
	pop_back() {
        if( count_ ) {
            T*  data    = static_cast<T*>(data_);
            --count_;
            (data[count_]).~T();
		}
	}

    size_t		size() const			{ return count_;	}
    const T&	operator[] (size_t i) const	{ const T*  data    = static_cast<const T*>(data_); return data[i];	}

    T&		operator[] (size_t i)		{ T*  data    = static_cast<T*>(data_); return data[i];	}

	rvector&
	operator = (const rvector<T>& v) {
		this->~rvector();
        reserved_	= v.reserved_;
        count_		= v.count_;

        data_		= malloc(sizeof(T) * reserved_);
        T*  data    = static_cast<T*>(data_);
        const T*  vData    = static_cast<const T*>(v.data_);
        for( size_t i = 0; i < count_; ++i )
            new(&(data[i])) T(vData[i]);
		return *this;
	}

    rvector(const rvector<T>& v) : VectorBase(v.count_, v.reserved_, nullptr) {
        data_		= malloc(sizeof(T) * reserved_);
        T*  data    = static_cast<T*>(data_);
        const T*  vData    = static_cast<const T*>(v.data_);

        for( size_t i = 0; i < count_; ++i )
            new(&(data[i])) T(vData[i]);
	}

	void
	clear()	{
        T*  data    = static_cast<T*>(data_);
        for( size_t i = 0; i < count_; ++i ) {
            (data[i]).~T();
		}
        count_	= 0;
	}

	void
	resize(size_t new_size)	{
        if( new_size > reserved_ ) {
			// expand
            reserved_	= new_size;

            T*	new_data	= static_cast<T*>(malloc(sizeof(T) * reserved_));
			assert(new_data != 0);
            T*  data    = static_cast<T*>(data_);
            for( size_t i = 0; i < count_; ++i ) {
                new(&(new_data[i])) T(data[i]);
			}

			// remove old data
            for( size_t i = 0; i < count_; ++i ) {
                (data[i]).~T();
			}
            free(data_);

            data_	= new_data;
            data    = static_cast<T*>(data_);
			// initialize the new new allocated elements
            for( size_t i = count_; i < reserved_; ++i )
			{
                new(&(data[i])) T();
			}

            count_	= reserved_;
        } else if( new_size < count_ ) {
			// shrink and remove data
            T* data    = static_cast<T*>(data_);
            for( size_t i = new_size; i < count_; ++i ) {
                (data[i]).~T();
			}

            count_	= new_size;
        } else if( new_size > count_ ) {	// and new_size <= __v_reserved

			// initialize new elements in reserved
            T* data    = static_cast<T*>(data_);
            for( size_t i = count_; i < new_size; ++i )
			{
                new(&(data[i])) T();
			}
            count_	= new_size;
		}
	}

	void*
	operator new(size_t s) {
        return malloc(s);
	}

	void
	operator delete(void* p) {
        return free(p);
	}

	void*
	operator new(size_t /* s */, void* p) {
		return p;
	}

	void
	operator delete(void* /* p */, void*) {
		return;
	}

};	// struct vector
}	// raja
#endif // VECTOR_HPP
