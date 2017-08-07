#include "string.hpp"

#include <stdio.h>

#include "forth.hpp"

extern "C" {
FORTH_API uint32_t __forth_hash_string__(const char* str)
{
	uint32_t seed = 0;

	size_t s = Forth::String(str).size();
	for( size_t i = 0; i < s; ++i )
	{
		seed ^= str[i] + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	return seed;
}

FORTH_API uint32_t __forth_reverse_hash_string__(const char* str)
{
	uint32_t seed = 0;

	size_t s = Forth::String(str).size();
	for( size_t i = 0; i < s; ++i )
	{
		seed ^= str[s - i - 1] + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	return seed;
}

}

void*    operator new(size_t, void* p) NOEXCEPT  { return p; }
void*    operator new(size_t s) NOEXCEPT     { return malloc(s); }
void     operator delete(void* p) NOEXCEPT   { free(p);  }
void*    operator new[](size_t s) NOEXCEPT   { return malloc(s); }
void     operator delete[](void* p) NOEXCEPT { free(p);  }
void     operator delete[](void* p, size_t) NOEXCEPT { free(p);  }

namespace Forth {
RCObject::~RCObject() {}

}


