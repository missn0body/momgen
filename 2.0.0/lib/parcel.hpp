#ifndef momgen__parcel__hpp
#define momgen__parcel__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	A simple, more compact way to store information for momgen
	Made by anson in <YEAR>, see LICENSE for related details
*/

#include <bitset>
#include <limits>

enum { 	IS_CPP    = (1 << 0), IS_MULTI      = (1 << 1), IS_MODERN = (1 << 2),
       	DEBUG_SYM = (1 << 3), WARN_AS_ERROR = (1 << 4), OPTIMIZE  = (1 << 5),
       	OPTI_FAST = (1 << 6), OPTI_SIZE	    = (1 << 7), ALL_WARN  = (1 << 8),
 	HAS_LIB	  = (1 << 9), WANT_LINT	    = (1 << 10) };

constexpr std::size_t bitsize = std::numeric_limits<unsigned short>::max();
struct parcel
{
	std::string project, compiler_flags;
	std::bitset<bitsize> flag;

	parcel(const std::string &name) : project(name) { this->flag = 0x00; }
	parcel(const std::string &name, const std::size_t what) : project(name), flag(what) {}

	constexpr bool operator[](std::size_t index) const { return this->flag.test(index); }
	void   set(std::size_t what) { this->flag.set(what); }
	void reset(std::size_t what) { this->flag.reset(what); }

	void MakeFlags(void);
};

#endif /* momgen__parcel__hpp */
