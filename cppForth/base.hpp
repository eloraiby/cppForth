#ifndef __FORTH_BASE__
#define __FORTH_BASE__

#define _HAS_EXCEPTIONS 0   // MSVC (disable exception)

#ifndef FORTH_API
#	ifdef BUILDING_STATIC
#		define FORTH_API
#		define FORTH_LOCAL_API
#	else
#		if defined _WIN32 || defined __CYGWIN__
#			ifdef BUILD_FORTH_SHARED
#				ifdef __GNUC__
#					define FORTH_API __attribute__ ((dllexport))
#				else
#					define FORTH_API __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#				endif
#			else
#				ifdef __GNUC__
#					define FORTH_API __attribute__ ((dllimport))
#				else
#					define FORTH_API __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#				endif
#			endif	// BUILD_RAJA_RSTL_SHARED
#			define FORTH_LOCAL_API
#		else
#			if __GNUC__ >= 4
#				define FORTH_API __attribute__ ((visibility ("default")))
#				define FORTH_LOCAL_API  __attribute__ ((visibility ("hidden")))
#			else
#				define FORTH_API
#				define FORTH_LOCAL_API
#			endif
#		endif
#	endif
#endif	// RAJA_RSTL_API

#ifdef _MSC_VER
#   define CRT_API __CRTDECL
#else
#   define CRT_API FORTH_API
#endif

#include <cfloat>
#include <cstdint>
#include <cstring>
#include <malloc.h>
#include <cassert>

#ifdef _MSC_VER
#	if !_HAS_EXCEPTIONS
#		define NOEXCEPT
#	else
#		define NOEXCEPT noexcept
#	endif
#else
#	define NOEXCEPT noexcept
#endif

#ifdef __GNUC__
#   define _NEW
namespace std {
struct nothrow_t
{
  explicit nothrow_t() = default;
};

extern const nothrow_t nothrow;
}   // namespace std

inline void*	operator new(size_t s, const std::nothrow_t&)   NOEXCEPT { return malloc(s);    }

#endif

// TODO: use this ?
// #include <new>

inline void*	operator new(size_t s)		            NOEXCEPT { return malloc(s);    }
inline void	    operator delete(void* p)	            NOEXCEPT { return free(p);      }
inline void*	operator new[](size_t s)	            NOEXCEPT { return malloc(s);    }
inline void	    operator delete[](void* p)	            NOEXCEPT { return free(p);      }
inline void*	operator new(size_t /*s*/, void* p)	    NOEXCEPT { return p;            }
inline void*	operator new[](size_t /*s*/, void* p)	NOEXCEPT { return p;            }
inline void	    operator delete(void* , void* p)	    NOEXCEPT { }
inline void	    operator delete[](void* , void* p)	    NOEXCEPT { }



namespace Forth {

template<typename T>
struct Hash {
    static uint32_t    hash(const T& t);
};

template <>
struct Hash<uint32_t> {
    static uint32_t hash(const uint32_t t) {
        uint32_t seed = 0;

        seed ^= (t & 0xFF) + 0x9e3779b9;
        seed ^= ((t & 0xFF00) >> 8) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= ((t & 0xFF0000) >> 16) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= ((t & 0xFF000000) >> 24) + 0x9e3779b9 + (seed<<6) + (seed>>2);

        return seed;
    }
};

class NonCopyable
{
protected:
	NonCopyable() {}
	~NonCopyable() {}
private:  // emphasize the following members are private
	NonCopyable( const NonCopyable& );
	const NonCopyable& operator=( const NonCopyable& );
};

}

#endif  // __FORTH_BASE__
