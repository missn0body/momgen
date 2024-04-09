#ifndef momgen__strops__hpp
#define momgen__strops__hpp

#ifndef __cplusplus
#error This header file is only to be used with C++ code
#endif /* __cplusplus */

/*
	Additional string operations for simpifying certain functions
	These are made for ASCII strings, not unicode safe!
	Made by anson in 2024, see LICENSE for related details
*/

#include <algorithm>
#include <string>
#include <type_traits>

// Append a bunch of strings into one using the comma and return it
// I really didn't want to do a bunch of '+=' or '.append()' statements
// so I really like this
template<typename... T>
std::string AsString(T&&... t)
{
	std::string ret;
	static_assert((std::is_convertible_v<T&&, std::string> && ...));
	return ((ret += std::forward<T>(t)), ...);
}

// Repeat a string N amount of times
// Again, saves me from having to both write and read repeated code
static std::string Mult(const std::string &what, std::size_t count)
{
	std::string ret;
	while(count--) ret += what;
	return ret;
}

// Make an entire string uppercase
inline std::string &ToUpper(std::string &what)
{
	std::transform(what.begin(), what.end(), what.begin(), [](unsigned char c){ return std::toupper(c); });
	return what;
}

// Make an entire string lowercase
inline std::string &ToLower(std::string &what)
{
	std::transform(what.begin(), what.end(), what.begin(), [](unsigned char c){ return std::tolower(c); });
	return what;
}

// Removes any trailing whitespaces after a string
inline std::string TruncateSpaces(std::string &input)
{
	std::string ret;
	auto a = [](unsigned char a, unsigned char b) { return std::isspace(a) && std::isspace(b); };

	std::unique_copy(input.begin(), input.end(), std::back_insert_iterator<std::string>(ret), a);
	return ret;
}

// Applies the top two functions to effectively normalize a string.
// e.g. "ExaMple     " to "example"
inline std::string Normal(std::string &input) { return TruncateSpaces(ToLower(input)); }

#endif /* momgen__strops__hpp */
