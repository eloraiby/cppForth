#ifndef STRING_HPP
#define STRING_HPP

#ifndef __FORTH_BASE__
#   include "base.hpp"
#endif

#include "vector.hpp"

extern "C" {
FORTH_API uint32_t __forth_hash_string__(const char* str);
FORTH_API uint32_t __forth_reverse_hash_string__(const char* str);
}

namespace Forth
{
///
/// RTK string implementation
///
struct String
{
	inline String()	{	__data.push_back('\0');	}

	inline String(const String& other) : __data(other.__data)	{}

	inline String(const char* other) {
		if( other ) {
			size_t len	= strlen(other);

			__data.resize(len + 1);
			strcpy(&(__data[0]), other);
		} else {
			__data.push_back('\0');
		}
	}

	inline String(char s) {
		__data.push_back(s);
		__data.push_back('\0');
	}

	inline ~String() {}

	inline void
	clear()	{
		__data.resize(1);
		__data[0]	= '\0';
	}

	inline String&
	operator = (const String& s) {
		if( &s != this )	// an idiot is trying to copy himself ?
			__data	= s.__data;
		return *this;
	}

	inline String&
	operator += (const String& s) {
		if( &s == this )
		{
			String	scpy(s);
			*this	+= scpy;
		}
		else
		{
			size_t	len	= __data.size();
			__data.resize(__data.size() + s.__data.size() - 1);
			strcpy(&(__data[len - 1]), &(s.__data[0]));
		}

		return *this;
	}

	inline String&
	operator = (const char* s)
	{
		size_t len	= strlen(s);

		__data.resize(len + 1);
		strcpy(&(__data[0]), s);
		return *this;
	}

	inline String&
	operator += (const char* s)
	{
		size_t	slen	= strlen(s);
		size_t	len	= __data.size();
		__data.resize(__data.size() + slen);
		strcpy(&(__data[len - 1]), s);

		return *this;
	}

	inline String&
	operator = (char s)
	{
		__data.resize(2);
		__data[0]	= s;
		__data[1]	= '\0';
		return *this;
	}

	inline String&
	operator += (char s)
	{
		__data[__data.size() - 1]	= s;
		__data.push_back('\0');
		return *this;
	}

	inline bool
	operator == (const String& s) const
	{
		return (strcmp(&(s.__data[0]), &(__data[0])) == 0);
	}

	inline bool
	operator != (const String& s) const
	{
		return (strcmp(&(s.__data[0]), &(__data[0])) != 0);
	}

	inline bool
	operator < (const String& s) const
	{
		return (strcmp(&(__data[0]), &(s.__data[0])) < 0 );
	}

	inline bool
	operator > (const String& s) const
	{
		return (strcmp(&(__data[0]), &(s.__data[0])) > 0 );
	}

	inline String
	operator + (const String& s) const
	{
		String	temp(*this);
		return (temp += s);
	}

	inline String
	operator + (const char* s) const
	{
		String	temp(*this);
		return (temp += s);
	}

	inline size_t		length() const	{	return __data.size() - 1;	}
	inline size_t		size() const	{	return __data.size() - 1;	}

	inline char         operator[] (size_t i) const	{		return __data[i];	}
	inline char&		operator[] (size_t i)		{		return __data[i];	}

	inline const char*	c_str() const			    {	return &(__data[0]);		}


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

private:
	Vector<char>		__data;		///< the actual string data
};	// struct string

inline String operator + (const char* cstr, const String& str) {	return (String(cstr) + str);	}




///
/// make a string upper case
/// @param str the string
/// @return the upper cased string
///
inline String to_upper(const String& str)
{
	String res;
	for( size_t i = 0; i < str.size(); ++i )
		if( str[i] >= 'a' && str[i] <= 'z' )
			res	+= (str[i] - 'a') + 'A';
		else
			res	+= str[i];
	return res;

}

///
/// make a string lower case
/// @param str the string
/// @return the lower cased string
///
inline String to_lower(const String& str)
{
	String res;
	for( size_t i = 0; i < str.size(); ++i )
		if( str[i] >= 'A' && str[i] <= 'Z' )
			res	+= (str[i] - 'A') + 'a';
		else
			res	+= str[i];
	return res;
}

///
/// hash a string
/// @param str the string to hash
/// @return hash number as 32 bits integer
///
inline uint32_t			hash_string(const String& str)	{ return __forth_hash_string__(str.c_str());	}

template<>
struct Hash<String> {
    static uint32_t hash(const String& str) { return hash_string(str); }
};

}	// namespace Forth
#endif // STRING_HPP

