#ifndef momgen__print__hpp
#define momgen__print__hpp

#ifndef __cplusplus
#error This header file is to only be used with C++ code
#endif /* __cplusplus */

/*
	Wrapper for outputting to C++ streams
	Made by anson in 2024, see LICENSE for related details
*/

#include <iostream>

// This is a C++20 feature for the functions below, this line can be erased and
// the later "streamable" lines can be replaced with "typename" if need be.
template<typename T> concept streamable = requires(T t, std::ostream &o) { o << t; };

// These do not have an std::ostream object specified, so you will have to specify it
template<streamable... T> static inline void Print  (std::ostream &o, T&&... t) { (o << ... << std::forward<T&&>(t)); }
template<streamable... T> static inline void Println(std::ostream &o, T&&... t) { (o << ... << std::forward<T&&>(t)) << "\n"; }
// Meanwhile these direct to std::cout
template<streamable... T> static inline void Print  (T&&... t) { Print(std::cout, std::forward<T&&>(t)...); }
template<streamable... T> static inline void Println(T&&... t) { Print(std::cout, std::forward<T&&>(t)..., "\n"); }

#endif /* momgen__print__hpp */
